#include <cassert>
#include <glade/net/NetworkManager.h>
#include <glade/io/Stream.h>
#include <glade/Context.h>

#include <enet/enet.h>

static ENetHost *server_host = nullptr, *client_host = nullptr;
static ENetPeer *server_peer = nullptr;
unsigned int Glade::NetworkManager::nextUserId = 1;

static std::set<ENetPeer *> clients;

void Glade::NetworkManager::processNetworkEvents(void *host) {
  ENetEvent event;
  ::enet_host_service((ENetHost *)host, &event, 0);

  switch (event.type) {
  case ENET_EVENT_TYPE_CONNECT: {
    if (mServer) {
      ::log("A new peer connected from %x:%u.", event.peer->address.host, event.peer->address.port);

      event.peer->data = (void *)"Player";
      ::clients.insert(event.peer);

      unsigned int new_user_id = nextUserId++;
      Glade::MemoryOutputStream stream;
      stream.write((unsigned int)MessageType::LOGIN);
      stream.write(new_user_id);
      ::log("New user logged in with UID %d", new_user_id);
      mContext->getCurrentState()->onUserLoggedIn(new_user_id);

      send(event.peer, stream.getBuffer(), stream.getWrittenSize(), true);

      // Make sure that replicas are received AFTER the login confirmation
      std::set<ENetPeer *> new_users;
      new_users.insert(event.peer);
      sendReplicas(new_users, true);

      // temporary solution
      //for (GladeObject* object: mReplicatedObjects) {
      //  log("Setting object dirty");
      //  object->SetReplicasOutdated(true);
      //}
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

    if (event.packet->data && event.packet->dataLength)
      processIncomingData(event.packet->data, event.packet->dataLength);
    else
      ::log("Warning: a packet with null buffer or zero size was received. This shouldn't happen");

    ::enet_packet_destroy(event.packet);
    break;
  }
  case ENET_EVENT_TYPE_DISCONNECT: {
    if (mServer) {
      ::log("%s disconnected.", (char *)event.peer->data);
      event.peer->data = nullptr;
      ::clients.erase(event.peer);
    } else {
      ::log("Event disconnect, but we are the client");
    }

    break;
  }
  case ENET_EVENT_TYPE_NONE:
    break;
  }
}

Glade::NetworkManager::NetworkManager(Context *context, bool server)
    : mContext(context), mServer(server), mCurrentUserId(0) {
  if (server) initServer();

  initClient();
}

Glade::NetworkManager::~NetworkManager() {
  ::enet_host_destroy(::client_host);
  ::enet_host_destroy(::server_host);
}

void Glade::NetworkManager::addObject(GladeObject *object) { mReplicatedObjects.insert(object); }

int Glade::NetworkManager::connectToServer(const char *host, unsigned int port) {
  if (!mInitialized) return 1;

  ENetAddress address;
  address.port = port;

  if (::enet_address_set_host(&address, host)) {
    ::printf("Couldn't resolve server hostname");
    return 3;
  }

  // last parameter is some arbitrary data (client id?)
  ::server_peer = ::enet_host_connect(::client_host, &address, 1, 0);

  if (::server_peer == nullptr) return 2;

  return 0;
}

bool Glade::NetworkManager::send(void *peer, const unsigned char *data, int length, bool reliable) {
  ENetPacket *packet = ::enet_packet_create(data, length, 0);
  ::enet_peer_send((ENetPeer *)peer, 0, packet);

  return true;
}

bool Glade::NetworkManager::sendToServer(const unsigned char *data, int length, bool reliable) {
  if (!mInitialized) return false;

  ENetPacket *packet = ::enet_packet_create(data, length, 0);
  ::enet_peer_send(::server_peer, 0, packet);

  return true;
}

bool Glade::NetworkManager::sendInputActionToServer(unsigned int input_action, bool is_pressed) {
  Glade::MemoryOutputStream stream;
  stream.write((unsigned int)MessageType::CLIENT_INPUT);
  stream.write(mCurrentUserId);
  stream.write(input_action);
  stream.write(is_pressed);

  sendToServer(stream.getBuffer(), stream.getWrittenSize());
}

void Glade::NetworkManager::process() {
  if (!mInitialized) return;

  if (mServer) sendReplicas(::clients);

  processNetworkEvents(mServer ? ::server_host : ::client_host);
}

void Glade::NetworkManager::sendReplicas(const std::set<ENetPeer *> &to_clients,
                                         bool force_all_objects) {
  assert(mServer);

  if (to_clients.size() == 0) return;

  if (force_all_objects) ::log("Forcfully sending all replicas to a client");

  Glade::MemoryOutputStream stream;

  for (GladeObject *object : mReplicatedObjects) {
    if (object->areReplicasOutdated() || force_all_objects) {
      stream.write((unsigned int)MessageType::OBJECT_REPLICA);
      object->serialize(stream);
      if (!force_all_objects) object->setReplicasOutdated(false);
      ::log("Sending object");
    }
  }

  if (stream.getWrittenSize() > 0) {
    for (ENetPeer *client_peer : to_clients) {
      ENetPacket *packet = ::enet_packet_create(stream.getBuffer(), stream.getWrittenSize(), 0);
      ::enet_peer_send(client_peer, 0, packet);
    }
  }
}

void Glade::NetworkManager::processIncomingData(unsigned char *data, unsigned int size) {
  Glade::MemoryInputStream stream(data, size);
  unsigned int message_type;

  while (stream.getUnreadSize() > 0) {
    stream.read(message_type);

    switch ((MessageType)message_type) {
    case OBJECT_REPLICA: {
      if (mServer) {
        ::log("Warning: recieved object replica message, but we are the server");
        return;
      }

      ::log("Received object");
      unsigned int received_object_id, received_object_type_id;
      stream.read(received_object_id);
      stream.read(received_object_type_id);

      if (received_object_type_id == 0) ::log("Received generic object");

      auto entry = GladeObject::indexById.find(received_object_id);
      //log("Received object: %d", receivedObjectId);
      GladeObject *entity;
      bool is_new_entity = (entry == GladeObject::indexById.end());

      if (is_new_entity) {
        ::log("Creating new entity");
        entity = mContext->getCurrentState()->createEntityByTypeId(received_object_type_id);
      } else {
        entity = entry->second;
      }

      if (entity) {
        entity->unserialize(stream);

        if (is_new_entity) mContext->getCurrentState()->onEntityReplicated(entity);
        else mContext->getCurrentState()->onEntityUnserialized(entity);
      } else {
        ::log("Warning: tried to replicate an object of unexpected type id. Broken packet?");
      }

      break;
    }
    case LOGIN: {
      if (mServer) {
        ::log("Warning: recieved login message, but we are the server");
        return;
      }

      unsigned int user_id;
      stream.read(user_id);
      mCurrentUserId = user_id;
      ::log("Logged in with UID %d", user_id);
      mContext->getCurrentState()->onUserLoggedIn(user_id);
      break;
    }
    case CLIENT_INPUT: {
      if (!mServer) {
        ::log("Warning: recieved client input message, but we are the client");
        return;
      }

      unsigned int user_id, glade_key_code;
      bool is_pressed;
      stream.read(user_id);
      stream.read(glade_key_code);
      stream.read(is_pressed);
      mContext->getCurrentState()->onClientInputReceived(user_id, glade_key_code, is_pressed);
      break;
    }
    default:
      ::log("Warning: unexpected network message type received");
      break;
    }
  }
}

int Glade::NetworkManager::initServer(unsigned int port) {
  ENetAddress server_address;
  server_address.host = ENET_HOST_ANY;
  server_address.port = port;
  ::server_host = ::enet_host_create(&server_address, 32, 2, 0, 0);

  assert(server_host);

  if (::server_host == nullptr) return 1;

  mInitialized = true;

  return 0;
}

int Glade::NetworkManager::initClient() {
  ::client_host = ::enet_host_create(nullptr, 1, 2, 0, 0);

  assert(client_host);

  if (::client_host == nullptr) return 1;

  mInitialized = true;

  return 0;
}
