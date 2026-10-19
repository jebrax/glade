#pragma once

#include <glade/collisions/CollisionBounds.h>
#include <glade/math/Transform.h>

// Geometry is independent of the owning object's sprite and transform.
class CollisionShape {
public:
  enum class Type {
    RECTANGLE,
    CONVEX_POLYGON
  };

  virtual ~CollisionShape() = default;

  virtual Type getType() const = 0;

  virtual CollisionBounds computeBounds(const Transform &world_transform) const = 0;
};
