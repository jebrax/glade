#pragma once

#include <glade/GladeObject.h>
#include <set>

class Context;

namespace Glade
{
  class NetworkManager
  {
    enum MessaageType {
      LOGIN = 0,
      OBJECT_REPLICA,
      MESSAGE_TYPE_MAX
    };

    static constexpr char DEFAULT_SERVER_HOST[] = "localhost";
    static constexpr unsigned int DEFAULT_SERVER_PORT = 58314;

    static unsigned int nextUserId;

    public:
      NetworkManager(Context* context, bool server);
      ~NetworkManager();
      bool isServer() { return mServer; }

      void addObject(GladeObject *object) {
        mReplicatedObjects.insert(object);
      }

      int connectToServer(const char *host = DEFAULT_SERVER_HOST, unsigned int port = DEFAULT_SERVER_PORT);
      bool send(void *peer, const unsigned char *data, int length, bool reliable);
      bool sendToServer(const unsigned char *data, int length, bool reliable = false);
      void process();

    private:
      void processNetworkEvents(void *host);
      void processIncomingData(unsigned char *data, unsigned int size);
      int initServer(unsigned int port = DEFAULT_SERVER_PORT);
      int initClient();

      Context *mContext;
      bool mServer;
      bool initialized;
      std::set<GladeObject*> mReplicatedObjects;
  };
}

