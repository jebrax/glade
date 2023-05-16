#pragma once

#include <glade/GladeObject.h>
#include <set>

class Context;
struct _ENetPeer;
typedef _ENetPeer ENetPeer;

namespace Glade
{
  class NetworkManager
  {
    enum MessageType {
      LOGIN = 0,
      OBJECT_REPLICA,
      CLIENT_INPUT,
      MESSAGE_TYPE_MAX
    };

    static constexpr char DEFAULT_SERVER_HOST[] = "localhost";
    static constexpr unsigned int DEFAULT_SERVER_PORT = 58314;

    static unsigned int nextUserId;

    public:
      NetworkManager(Context* context, bool server);
      ~NetworkManager();
      bool isServer() { return mServer; }

      // call only from main thread
      void addObject(GladeObject *object);
      int connectToServer(const char *host = DEFAULT_SERVER_HOST, unsigned int port = DEFAULT_SERVER_PORT);
      bool send(void *peer, const unsigned char *data, int length, bool reliable);
      bool sendInputActionToServer(unsigned int inputAction, bool isPressed);
      bool sendToServer(const unsigned char *data, int length, bool reliable = false);
      // call only from main thread
      void process();
      unsigned int getCurrentUserId() { return mCurrentUserId; }

    private:
      int initServer(unsigned int port = DEFAULT_SERVER_PORT);
      int initClient();
      void processNetworkEvents(void *host);
      void processIncomingData(unsigned char *data, unsigned int size);
      void sendReplicas(const std::set<ENetPeer*> &toClients, bool forceAllObjects = false);

      Context *mContext;
      bool mServer;
      bool mInitialized;
      unsigned int mCurrentUserId;
      std::set<GladeObject*> mReplicatedObjects;
  };
}

