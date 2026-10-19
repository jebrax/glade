#pragma once

#include <vector>

#include <glade/collisions/CollisionResults.h>

// Internal shape-pair result; CollisionWorld attaches the encountered collider.
struct NarrowphaseCastResult {
  bool hit = false;
  float fraction = 1.0f;
  bool startsOverlapping = false;
  std::vector<CollisionManifold> manifolds;
};
