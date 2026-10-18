#pragma once

#include <glade/debug/log.h>
#include <unordered_map>
#include <vector>

namespace Glade {

enum EventType {
  GLADE_COLLISION_EVENT,
};

class EventListener {
public:
  virtual void onEvent(EventType type, void *event_payload) = 0;
};

class EventBus {
public:
  void registerListener(EventType type, EventListener *listener) {
    if (!listeners.contains(type)) listeners[type] = std::vector<EventListener *>();

    listeners[type].push_back(listener);
  }

  void removeListener(EventType type, EventListener *listener) {
    if (!listeners.contains(type)) {
      ::log("Warning! Tried to remove event listener, but there are no listeners of such type");
      return;
    }

    auto &listeners_of_type = listeners[type];
    auto i = listeners_of_type.begin();
    bool erased = false;

    while (i != listeners_of_type.end()) {
      if (*i == listener) {
        if (erased) ::log("Warning! Found duplicate listener while removing");

        listeners_of_type.erase(i);
        erased = true;
      } else {
        ++i;
      }
    }

    if (!erased) ::log("Warning! Tried to remove event listener but it was not found");
  }

  void postEvent(EventType type, void *event_payload = nullptr) {
    if (!listeners.contains(type)) return;

    auto &listeners_of_type = listeners[type];

    for (EventListener *listener : listeners_of_type) {
      listener->onEvent(type, event_payload);
    }
  }

private:
  std::unordered_map<EventType, std::vector<EventListener *>> listeners;
};

}
