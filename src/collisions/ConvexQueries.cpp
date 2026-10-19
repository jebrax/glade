#include "ConvexQueries.h"

#include <glade/collisions/ConvexPolygonCollisionShape.h>
#include <glade/collisions/RectangleCollisionShape.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

struct Point {
  double x;
  double y;
};

using Polygon = std::vector<Point>;

double dot(const Point &a, const Point &b) {
  return a.x * b.x + a.y * b.y;
}

Polygon polygon_vertices(const CollisionShape &shape, const Transform &transform,
                         const Point &origin) {
  Polygon vertices;
  const auto &position = *transform.position;
  const Point offset{double(position.x) - origin.x, double(position.y) - origin.y};

  if (shape.getType() == CollisionShape::Type::RECTANGLE) {
    const auto &half = static_cast<const RectangleCollisionShape &>(shape).getHalfExtents();
    vertices = {{-half.x, -half.y}, {half.x, -half.y},
                {half.x, half.y}, {-half.x, half.y}};
  } else {
    for (const auto &vertex : static_cast<const ConvexPolygonCollisionShape &>(shape).getVertices()) {
      vertices.push_back({vertex.x, vertex.y});
    }
  }

  for (auto &vertex : vertices) {
    vertex.x += offset.x;
    vertex.y += offset.y;
  }

  return vertices;
}

std::vector<Point> separating_axes(const Polygon &query, const Polygon &obstacle) {
  std::vector<Point> axes;

  for (const auto *polygon : {&query, &obstacle}) {
    for (std::size_t i = 0; i < polygon->size(); ++i) {
      const auto &a = (*polygon)[i];
      const auto &b = (*polygon)[(i + 1) % polygon->size()];
      // Avoid normalisation roundoff in interval comparisons. Normals are
      // normalised only for reported contacts and penetration distances.
      axes.push_back({b.y - a.y, a.x - b.x});
    }
  }

  return axes;
}

struct Interval {
  double min;
  double max;
};

Interval project(const Polygon &polygon, const Point &axis) {
  const double first = ::dot(polygon.front(), axis);
  Interval interval{first, first};

  for (const auto &vertex : polygon) {
    const double value = ::dot(vertex, axis);
    interval.min = std::min(interval.min, value);
    interval.max = std::max(interval.max, value);
  }

  return interval;
}

Interval support_span(const Polygon &polygon, const Point &normal,
                      const Point &tangent, bool maximum) {
  const auto projection = ::project(polygon, normal);
  const double support = maximum ? projection.max : projection.min;
  Interval span{std::numeric_limits<double>::infinity(),
                -std::numeric_limits<double>::infinity()};

  for (const auto &vertex : polygon) {
    const double distance = ::dot(vertex, normal);
    // Only accounts for double-precision projection roundoff, not a collision skin.
    const double error = 32.0 * std::numeric_limits<double>::epsilon() *
        (std::abs(vertex.x * normal.x) + std::abs(vertex.y * normal.y) + std::abs(support));

    if (std::abs(distance - support) <= error) {
      const double along = ::dot(vertex, tangent);
      span.min = std::min(span.min, along);
      span.max = std::max(span.max, along);
    }
  }

  return span;
}

CollisionManifold make_manifold(const Polygon &query, const Polygon &obstacle,
                                const Point &normal, double separation, const Point &origin) {
  CollisionManifold manifold;
  manifold.normal = {float(normal.x), float(normal.y), 0.0f};
  const Point tangent{-normal.y, normal.x};
  const auto query_span = ::support_span(query, normal, tangent, false);
  const auto obstacle_span = ::support_span(obstacle, normal, tangent, true);
  const double plane = ::project(obstacle, normal).max;
  // At minimum separation, the two supporting features meet. Clip their tangent
  // intervals to obtain one vertex or two face endpoints on the obstacle surface.
  const double low = std::clamp(query_span.min, obstacle_span.min, obstacle_span.max);
  const double high = std::max(low,
      std::clamp(query_span.max, obstacle_span.min, obstacle_span.max));
  const auto append = [&](double along) {
    manifold.points.push_back({{float(origin.x + normal.x * plane + tangent.x * along),
                                float(origin.y + normal.y * plane + tangent.y * along), 0.0f},
                               float(separation)});
  };
  append(low);

  if (high > low) append(high);

  return manifold;
}

std::optional<CollisionManifold> overlap_polygons(
    const Polygon &query, const Polygon &obstacle,
    const std::vector<Point> &axes, const Point &origin) {
  double depth = std::numeric_limits<double>::infinity();
  Point normal{};

  for (const auto &axis : axes) {
    const auto a = ::project(query, axis);
    const auto b = ::project(obstacle, axis);

    if (a.max < b.min || b.max < a.min) return std::nullopt;

    const double negative = a.max - b.min;
    const double positive = b.max - a.min;
    const double length = std::hypot(axis.x, axis.y);
    const double candidate = std::min(negative, positive) / length;

    if (candidate < depth) {
      depth = candidate;
      const double sign = negative <= positive ? -1.0 : 1.0;
      normal = {axis.x * sign / length, axis.y * sign / length};
    }
  }

  return ::make_manifold(query, obstacle, normal, -depth, origin);
}

}

std::optional<CollisionManifold> overlap_convex_shapes(
    const CollisionShape &query, const Transform &query_transform,
    const CollisionShape &obstacle, const Transform &obstacle_transform) {
  const Point origin{query_transform.position->x, query_transform.position->y};
  const auto a = ::polygon_vertices(query, query_transform, origin);
  const auto b = ::polygon_vertices(obstacle, obstacle_transform, origin);

  return ::overlap_polygons(a, b, ::separating_axes(a, b), origin);
}

NarrowphaseCastResult cast_convex_shapes(
    const CollisionShape &query, const Transform &query_transform,
    const Glade::Vector3f &displacement,
    const CollisionShape &obstacle, const Transform &obstacle_transform) {
  NarrowphaseCastResult result;
  const Point origin{query_transform.position->x, query_transform.position->y};
  auto a = ::polygon_vertices(query, query_transform, origin);
  const auto b = ::polygon_vertices(obstacle, obstacle_transform, origin);
  const auto axes = ::separating_axes(a, b);
  const auto overlap = ::overlap_polygons(a, b, axes, origin);

  if (overlap && overlap->points.front().separation < 0.0f) {
    result.hit = true;
    result.fraction = 0.0f;
    result.startsOverlapping = true;
    result.manifolds.push_back(*overlap);

    return result;
  }

  struct Entry {
    double fraction;
    Point normal;
  };
  std::vector<Entry> entries;
  double enter = -std::numeric_limits<double>::infinity();
  double exit = std::numeric_limits<double>::infinity();
  const Point movement{displacement.x, displacement.y};

  for (const auto &axis : axes) {
    const auto query_interval = ::project(a, axis);
    const auto obstacle_interval = ::project(b, axis);
    const double speed = ::dot(movement, axis);

    if (speed == 0.0) {
      if (query_interval.max <= obstacle_interval.min ||
          query_interval.min >= obstacle_interval.max) return result;

      continue;
    }

    double first = (obstacle_interval.min - query_interval.max) / speed;
    double last = (obstacle_interval.max - query_interval.min) / speed;
    const double sign = speed > 0.0 ? -1.0 : 1.0;

    if (first > last) std::swap(first, last);

    enter = std::max(enter, first);
    exit = std::min(exit, last);
    const double length = std::hypot(axis.x, axis.y);
    entries.push_back({first, {axis.x * sign / length, axis.y * sign / length}});
  }

  if (enter < 0.0 || enter > 1.0 || exit <= enter) return result;

  result.hit = true;
  result.fraction = float(enter);

  for (auto &vertex : a) {
    vertex.x += movement.x * enter;
    vertex.y += movement.y * enter;
  }

  std::vector<Point> normals;

  for (const auto &entry : entries) {
    if (entry.fraction != enter) continue;

    const bool duplicate = std::any_of(normals.begin(), normals.end(), [&](const Point &normal) {
      return std::abs(normal.x - entry.normal.x) < 1.0e-12 &&
             std::abs(normal.y - entry.normal.y) < 1.0e-12;
    });

    if (duplicate) continue;

    const Point tangent{-entry.normal.y, entry.normal.x};
    const auto query_span = ::support_span(a, entry.normal, tangent, false);
    const auto obstacle_span = ::support_span(b, entry.normal, tangent, true);

    // An axis from the opposite face can tie at a vertex/vertex impact even
    // though neither contacting feature has that face normal. Its constraint
    // is redundant with the actual incident faces, so do not report it.
    if (query_span.min == query_span.max && obstacle_span.min == obstacle_span.max) continue;

    normals.push_back(entry.normal);
    result.manifolds.push_back(::make_manifold(a, b, entry.normal, 0.0, origin));
  }

  return result;
}
