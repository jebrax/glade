#include <glade/net/NetworkManager.h>
#include <glade/io/Stream.h>
#include <glade/Context.h>
#include <enet/enet.h>

static ENetHost *serverHost = nullptr, *clientHost = nullptr;
static ENetPeer *serverPeer = nullptr;
unsigned int Glade::NetworkManager::nextUserId = 1;

static std::set<ENetPeer*> clients;

void Glade::NetworkManager::processNetworkEvents(void *host)
{
  ENetEvent event;
  enet_host_service((ENetHost*) host, &event, 0);

  switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT: {
      if (mServer) {
        log("A new peer connected from %x:%u.",
            event.peer->address.host,
            event.peer->address.port);

        event.peer->data = (void *) "Player";
        clients.insert(event.peer);

        unsigned int newUserId = nextUserId++;
        Glade::MemoryOutputStream stream;
        stream.Write((unsigned int) MessageType::LOGIN);
        stream.Write(newUserId);
        log("New user logged in with UID %d", newUserId);
        mContext->GetCurrentState()->onUserLoggedIn(newUserId);

        send(event.peer, stream.GetBuffer(), stream.GetWrittenSize(), true);
      }

      break;
    }
    case ENET_EVENT_TYPE_RECEIVE: {
      /*
      log("A packet of length %lu was received from %s on channel %u.",
             event.packet->dataLength,
             (char *) event.peer->data,
             event.channelID);
      */

      processIncomingData(event.packet->data, event.packet->dataLength);

      enet_packet_destroy(event.packet);
      break;
    }
    case ENET_EVENT_TYPE_DISCONNECT: {
      if (mServer) {
        log("%s disconnected.", (char *) event.peer->data);
        event.peer->data = nullptr;
        clients.erase(event.peer);
      } else {
        log("Event disconnect, but we are the client");
      }

      break;
    }
    case ENET_EVENT_TYPE_NONE:
      break;
  }
}

Glade::NetworkManager::NetworkManager(Context* context, bool server):
  mContext(context),
  mServer(server),
  mCurrentUserId(0)
{
  if (server)
    initServer();

  initClient();
}

Glade::NetworkManager::~NetworkManager()
{
  enet_host_destroy(clientHost);
  enet_host_destroy(serverHost);
}

int Glade::NetworkManager::connectToServer(const char *host, unsigned int port)
{
  if (!mInitialized)
    return 1;

  ENetAddress address;
  address.port = port;

  if (enet_address_set_host(&address, host)) {
    printf("Couldn't resolve server hostname");
    return 3;
  }

  // last parameter is some arbitrary data (client id?)
  serverPeer = enet_host_connect(clientHost, &address, 1, 0);

  if (serverPeer == nullptr)
    return 2;

  return 0;
}

bool Glade::NetworkManager::send(void *peer, const unsigned char *data, int length, bool reliable)
{
  ENetPacket* packet = enet_packet_create(data, length, 0);
  enet_peer_send((ENetPeer *) peer, 0, packet);
  return true;
}

bool Glade::NetworkManager::sendToServer(const unsigned char *data, int length, bool reliable)
{
  if (!mInitialized)
    return false;

  ENetPacket* packet = enet_packet_create(data, length, 0);
  enet_peer_send(serverPeer, 0, packet);
  return true;
}

bool Glade::NetworkManager::sendInputActionToServer(unsigned int inputAction, bool isPressed)
{
  Glade::MemoryOutputStream stream;
  stream.Write((unsigned int) MessageType::CLIENT_INPUT);
  stream.Write(mCurrentUserId);
  stream.Write(inputAction);
  stream.Write(isPressed);

  sendToServer(stream.GetBuffer(), stream.GetWrittenSize());
}

void Glade::NetworkManager::process()
{
  if (!mInitialized)
    return;

  if (mServer) {
    Glade::MemoryOutputStream stream;

    for (GladeObject* object: mReplicatedObjects) {
      stream.Write((unsigned int) MessageType::OBJECT_REPLICA);
      object->serialize(stream);
    }

    for (ENetPeer *clientPeer: clients) {
      ENetPacket* packet = enet_packet_create(stream.GetBuffer(), stream.GetWrittenSize(), 0);
      enet_peer_send(clientPeer, 0, packet);
    }

    processNetworkEvents(serverHost);
    return;
  }

  processNetworkEvents(clientHost);
}

void Glade::NetworkManager::processIncomingData(unsigned char *data, unsigned int size)
{
  Glade::MemoryInputStream stream(data, size);
  unsigned int messageType;

  while (stream.GetUnreadSize() > 0) {
    stream.Read(messageType);

    switch ((MessageType) messageType) {
      case OBJECT_REPLICA: {
        if (mServer) {
          log("Warning: recieved object replica message, but we are the server");
          return;
        }

        unsigned int receivedObjectId, receivedObjectTypeId;
        stream.Read(receivedObjectId);
        stream.Read(receivedObjectTypeId);

        auto entry = GladeObject::indexById.find(receivedObjectId);
        //log("Received object: %d", receivedObjectId);
        GladeObject *entity;
        bool isNewEntity = (entry == GladeObject::indexById.end());

        if (isNewEntity) {
          log("Creating new entity");
          entity = mContext->GetCurrentState()->CreateEntityByTypeId(receivedObjectTypeId);
        } else {
          entity = entry->second;
        }

        if (entity) {
          entity->unserialize(stream);

          if (isNewEntity)
            mContext->GetCurrentState()->onEntityReplicated(entity);
        } else {
          log("Warning: tried to replicate an object of unexpected type id. Broken packet?");
        }

        break;
      }
      case LOGIN: {
        if (mServer) {
          log("Warning: recieved login message, but we are the server");
          return;
        }

        unsigned int userId;
        stream.Read(userId);
        mCurrentUserId = userId;
        log("Logged in with UID %d", userId);
        mContext->GetCurrentState()->onUserLoggedIn(userId);
        break;
      }
      case CLIENT_INPUT: {
        if (!mServer) {
          log("Warning: recieved client input message, but we are the client");
          return;
        }

        unsigned int userId, gladeKeyCode;
        bool isPressed;
        stream.Read(userId);
        stream.Read(gladeKeyCode);
        stream.Read(isPressed);
        mContext->GetCurrentState()->onClientInputReceived(userId, gladeKeyCode, isPressed);
        break;
      }
      default:
        log("Warning: unexpected network message type received");
        break;
    }
  }
}

int Glade::NetworkManager::initServer(unsigned int port)
{
  ENetAddress serverAddress;
  serverAddress.host = ENET_HOST_ANY;
  serverAddress.port = port;
  serverHost = enet_host_create(&serverAddress, 32, 2, 0, 0);

  assert(serverHost);
  if (serverHost == nullptr)
    return 1;

  mInitialized = true;
  return 0;
}

int Glade::NetworkManager::initClient()
{
  clientHost = enet_host_create(nullptr, 1, 2, 0, 0);

  assert(clientHost);
  if (clientHost == nullptr)
    return 1;

  mInitialized = true;
  return 0;
}

