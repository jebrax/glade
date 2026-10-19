#pragma once

#include <glade/collisions/CollisionShape.h>

// Centred on the transform's position. Z, rotation and scale are ignored.
class RectangleCollisionShape final : public CollisionShape {
public:
  RectangleCollisionShape(float half_width, float half_height);

  Type getType() const override { return Type::RECTANGLE; }

  CollisionBounds computeBounds(const Transform &world_transform) const override;

  const Glade::Vector2f &getHalfExtents() const { return halfExtents; }

private:
  Glade::Vector2f halfExtents;
};
