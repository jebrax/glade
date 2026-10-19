#pragma once

#include <vector>

#include <glade/collisions/CollisionShape.h>

// Immutable convex boundary in local X/Y. Translation only; ignores Z/rotation/scale.
class ConvexPolygonCollisionShape final : public CollisionShape {
public:
  explicit ConvexPolygonCollisionShape(std::vector<Glade::Vector2f> vertices_);

  Type getType() const override { return Type::CONVEX_POLYGON; }

  CollisionBounds computeBounds(const Transform &world_transform) const override;

  const std::vector<Glade::Vector2f> &getVertices() const { return vertices; }

private:
  std::vector<Glade::Vector2f> vertices;
};
