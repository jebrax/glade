#include <glade/net/NetworkManager.h>
#include <enet/enet.h>

static ENetHost *serverHost = nullptr, *clientHost = nullptr;
static ENetPeer *serverPeer = nullptr;

static std::set<ENetPeer*> clients;

void Glade::NetworkManager::processNetworkEvents(void *host)
{
  ENetEvent event;
  enet_host_service((ENetHost*) host, &event, 0);

  switch (event.type) {
    case ENET_EVENT_TYPE_CONNECT:
      printf ("A new client connected from %x:%u.\n",
              event.peer->address.host,
              event.peer->address.port);

      /* Store any relevant client information here. */
      event.peer->data = (void*) "Player";
      clients.insert(event.peer);
      break;

    case ENET_EVENT_TYPE_RECEIVE:
      printf("A packet of length %lu was received from %s on channel %u.\n",
              event.packet->dataLength,
              (char*) event.peer->data,
              event.channelID);

      if (!server)
        processIncomingData(event.packet->data, event.packet->dataLength);

      /* Clean up the packet now that we're done using it. */
      enet_packet_destroy(event.packet);
      break;

    case ENET_EVENT_TYPE_DISCONNECT:
      printf ("%s disconnected.\n", (char*) event.peer->data);
      /* Reset the peer's client information. */
      event.peer->data = NULL;
      clients.erase(event.peer);
      break;

    case ENET_EVENT_TYPE_NONE:
      break;
  }
}

Glade::NetworkManager::NetworkManager(bool server):
  server(server)
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

bool Glade::NetworkManager::sendToServer(const char *data, int length, bool reliable)
{
  ENetPacket* packet = enet_packet_create(data, length, 0);
  enet_peer_send(serverPeer, 0, packet);
  return true;
}

void Glade::NetworkManager::process()
{
  if (server) {
    // TODO: put many objects into one packet (as much as possible)
    for (GladeObject* object: replicatedObjects) {
      unsigned int size = object->serialize(nullptr, 0);
      unsigned char *buffer = (unsigned char*) malloc(size);

      object->serialize(buffer, size);

      for (ENetPeer *clientPeer: clients) {
        ENetPacket* packet = enet_packet_create(buffer, size, 0);
        enet_peer_send(clientPeer, 0, packet);
      }

      free(buffer);
    }

    processNetworkEvents(serverHost);
    return;
  }

  processNetworkEvents(clientHost);
}

void Glade::NetworkManager::processIncomingData(unsigned char *data, unsigned int size)
{
  GladeObject::unsearialize(data);
}

int Glade::NetworkManager::initServer(unsigned int port)
{
  ENetAddress serverAddress;
  serverAddress.host = ENET_HOST_ANY;
  serverAddress.port = port;
  serverHost = enet_host_create(&serverAddress, 32, 2, 0, 0);

  if (serverHost == nullptr)
    return 1;

  return 0;
}

int Glade::NetworkManager::initClient()
{
  clientHost = enet_host_create(nullptr, 1, 2, 0, 0);

  if (clientHost == nullptr)
    return 1;

  return 0;
}

