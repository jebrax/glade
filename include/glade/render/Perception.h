#pragma once

#include <glade/render/Drawable.h>

class Perception : public Drawable {
private:
  Glade::Vector3f baseLightDirection;
  Glade::Vector3f lightDirection;

public:
  Perception();
  void adjust();
  virtual ~Perception() {}
};
