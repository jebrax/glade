#include <glade/collisions/ConvexPolygonCollisionShape.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace {

double side(const Glade::Vector2f &a, const Glade::Vector2f &b, const Glade::Vector2f &c) {
  return (double(b.x) - a.x) * (double(c.y) - a.y) -
         (double(b.y) - a.y) * (double(c.x) - a.x);
}

float outward_bound(double value, bool lower) {
  float result = float(value);

  if ((lower && result > value) || (!lower && result < value)) {
    result = std::nextafter(result, lower ? -std::numeric_limits<float>::infinity()
                                        : std::numeric_limits<float>::infinity());
  }

  if (!std::isfinite(result)) {
    throw std::invalid_argument("Polygon bounds exceed finite X/Y coordinates");
  }

  return result;
}

}

ConvexPolygonCollisionShape::ConvexPolygonCollisionShape(std::vector<Glade::Vector2f> vertices_)
    : vertices(std::move(vertices_)) {
  if (vertices.size() < 3) {
    throw std::invalid_argument("A convex polygon requires at least three vertices");
  }

  for (const auto &vertex : vertices) {
    if (!std::isfinite(vertex.x) || !std::isfinite(vertex.y)) {
      throw std::invalid_argument("Polygon vertices must be finite");
    }
  }

  const double winding = ::side(vertices[0], vertices[1], vertices[2]);

  if (winding < 0.0) std::reverse(vertices.begin(), vertices.end());

  // Every other vertex must be strictly inside every directed edge. Unlike
  // checking consecutive turns alone, this also rejects self-intersecting stars.
  for (std::size_t i = 0; i < vertices.size(); ++i) {
    const auto next = (i + 1) % vertices.size();

    for (std::size_t j = 0; j < vertices.size(); ++j) {
      if (j == i || j == next) continue;

      if (::side(vertices[i], vertices[next], vertices[j]) <= 0.0) {
        throw std::invalid_argument("Polygon boundary must be strictly convex without repeated vertices");
      }
    }
  }
}

CollisionBounds ConvexPolygonCollisionShape::computeBounds(const Transform &world_transform) const {
  const auto &position = *world_transform.position;

  if (!std::isfinite(position.x) || !std::isfinite(position.y)) {
    throw std::invalid_argument("Polygon collision X/Y position must be finite");
  }

  auto min = vertices.front();
  auto max = min;

  for (const auto &vertex : vertices) {
    min.x = std::min(min.x, vertex.x);
    min.y = std::min(min.y, vertex.y);
    max.x = std::max(max.x, vertex.x);
    max.y = std::max(max.y, vertex.y);
  }

  return {{::outward_bound(double(position.x) + min.x, true),
           ::outward_bound(double(position.y) + min.y, true)},
          {::outward_bound(double(position.x) + max.x, false),
           ::outward_bound(double(position.y) + max.y, false)}};
}
