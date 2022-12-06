#pragma once

#include <glade/GladeObject.h>
#include <set>

namespace Glade
{
  class NetworkManager
  {
    static constexpr char DEFAULT_SERVER_HOST[] = "localhost";
    static constexpr unsigned int DEFAULT_SERVER_PORT = 58314;

    public:
      NetworkManager(bool server);
      ~NetworkManager();
      bool isServer() { return server; }

      void AddObject(GladeObject *object) {
        replicatedObjects.insert(object);
      }

      int connectToServer(const char *host = DEFAULT_SERVER_HOST, unsigned int port = DEFAULT_SERVER_PORT);
      bool sendToServer(const char *data, int length, bool reliable = false);
      void process();

    private:
      void processNetworkEvents(void *host);
      void processIncomingData(unsigned char *data, unsigned int size);
      int initServer(unsigned int port = DEFAULT_SERVER_PORT);
      int initClient();

      bool server;
      std::set<GladeObject*> replicatedObjects;
  };
}

