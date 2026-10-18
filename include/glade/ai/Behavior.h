#pragma once

#include "glade/State.h"
#include "glade/GladeObject.h"

class Behavior {
public:
  virtual void act(State &world, GladeObject &self) = 0;
};
