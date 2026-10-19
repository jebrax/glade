#pragma once

#include <glade/math/Vector.h>

// Conservative bounds in the physical X/Y plane. Touching bounds intersect.
struct CollisionBounds {
  Glade::Vector2f min;
  Glade::Vector2f max;

  bool intersects(const CollisionBounds &other) const;

  CollisionBounds translated(const Glade::Vector3f &displacement) const;

  CollisionBounds merged(const CollisionBounds &other) const;
};
