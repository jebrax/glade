#include <glade/collisions/CollisionBounds.h>

#include <algorithm>

bool CollisionBounds::intersects(const CollisionBounds &other) const {
  return min.x <= other.max.x && max.x >= other.min.x &&
         min.y <= other.max.y && max.y >= other.min.y;
}

CollisionBounds CollisionBounds::translated(const Glade::Vector3f &displacement) const {
  return {{min.x + displacement.x, min.y + displacement.y},
          {max.x + displacement.x, max.y + displacement.y}};
}

CollisionBounds CollisionBounds::merged(const CollisionBounds &other) const {
  return {{std::min(min.x, other.min.x), std::min(min.y, other.min.y)},
          {std::max(max.x, other.max.x), std::max(max.y, other.max.y)}};
}
