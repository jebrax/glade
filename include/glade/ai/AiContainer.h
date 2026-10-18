#pragma once

#include <vector>
#include "Behavior.h"

class AiContainer {
public:
  bool paused;

private:
  std::vector<GladeObject *> smartObjects;

public:
  AiContainer() : paused(false) {}

  void process(State &world) {
    if (paused) {
      return;
    }

    std::vector<GladeObject *>::iterator i = smartObjects.begin();

    while (i != smartObjects.end()) {
      if ((*i)->getBehavior() != NULL && (*i)->isBehaviorEnabled()) {
        (*i)->getBehavior()->act(world, **i);
      }

      ++i;
    }
  }

  void add(GladeObject *object) { smartObjects.push_back(object); }

  void clear() {
    smartObjects.clear();
    paused = false;
  }
};
