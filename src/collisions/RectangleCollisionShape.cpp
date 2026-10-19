#include <glade/collisions/RectangleCollisionShape.h>

#include <cmath>
#include <stdexcept>

RectangleCollisionShape::RectangleCollisionShape(float half_width, float half_height)
    : halfExtents(half_width, half_height) {
  if (!std::isfinite(half_width) || !std::isfinite(half_height) ||
      half_width <= 0.0f || half_height <= 0.0f) {
    throw std::invalid_argument("Rectangle collision half extents must be finite and positive");
  }
}

CollisionBounds RectangleCollisionShape::computeBounds(const Transform &world_transform) const {
  const auto &position = *world_transform.position;

  if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
    throw std::invalid_argument("Rectangle collision X/Y position must be finite");
  }

  return {{position.x - halfExtents.x, position.y - halfExtents.y},
          {position.x + halfExtents.x, position.y + halfExtents.y}};
}
