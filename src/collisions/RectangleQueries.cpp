#include "RectangleQueries.h"

#include <algorithm>
#include <limits>

namespace {

CollisionManifold make_manifold(const CollisionBounds &query, const CollisionBounds &obstacle,
                                const Glade::Vector3f &normal, float separation) {
  CollisionManifold manifold;
  manifold.normal = normal;

  if (normal.x != 0.0f) {
    const float x = normal.x < 0.0f ? obstacle.min.x : obstacle.max.x;
    const float low = std::max(query.min.y, obstacle.min.y);
    const float high = std::max(low, std::min(query.max.y, obstacle.max.y));
    manifold.points.push_back({{x, low, 0.0f}, separation});

    if (high > low) {
      manifold.points.push_back({{x, high, 0.0f}, separation});
    }
  } else {
    const float y = normal.y < 0.0f ? obstacle.min.y : obstacle.max.y;
    const float low = std::max(query.min.x, obstacle.min.x);
    const float high = std::max(low, std::min(query.max.x, obstacle.max.x));
    manifold.points.push_back({{low, y, 0.0f}, separation});

    if (high > low) {
      manifold.points.push_back({{high, y, 0.0f}, separation});
    }
  }

  return manifold;
}

struct AxisCast {
  double enter = -std::numeric_limits<double>::infinity();
  double exit = std::numeric_limits<double>::infinity();
  float normal = 0.0f;
};

bool cast_axis(float query_min, float query_max, float obstacle_min, float obstacle_max,
               float displacement, AxisCast &result) {
  if (displacement == 0.0f) {
    // A stationary touching axis allows sliding along the face. It cannot
    // produce penetration, even if the other axis moves through the bounds.
    return query_min < obstacle_max && query_max > obstacle_min;
  }

  if (displacement > 0.0f) {
    result.enter = (double(obstacle_min) - query_max) / displacement;
    result.exit = (double(obstacle_max) - query_min) / displacement;
    result.normal = -1.0f;
  } else {
    result.enter = (double(obstacle_max) - query_min) / displacement;
    result.exit = (double(obstacle_min) - query_max) / displacement;
    result.normal = 1.0f;
  }

  return true;
}

}

std::optional<CollisionManifold> overlap_rectangles(
    const CollisionBounds &query, const CollisionBounds &obstacle) {
  if (!query.intersects(obstacle)) return std::nullopt;

  // Face distances, rather than intersection width, also handle containment.
  const float left = query.max.x - obstacle.min.x;
  const float right = obstacle.max.x - query.min.x;
  const float bottom = query.max.y - obstacle.min.y;
  const float top = obstacle.max.y - query.min.y;
  const float depth_x = std::min(left, right);
  const float depth_y = std::min(bottom, top);

  if (depth_x <= depth_y) {
    const Glade::Vector3f normal(left <= right ? -1.0f : 1.0f, 0.0f, 0.0f);
    return ::make_manifold(query, obstacle, normal, -depth_x);
  }

  const Glade::Vector3f normal(0.0f, bottom <= top ? -1.0f : 1.0f, 0.0f);

  return ::make_manifold(query, obstacle, normal, -depth_y);
}

NarrowphaseCastResult cast_rectangles(
    const CollisionBounds &query, const Glade::Vector3f &displacement,
    const CollisionBounds &obstacle) {
  NarrowphaseCastResult result;
  const auto overlap = ::overlap_rectangles(query, obstacle);

  if (overlap && overlap->points.front().separation < 0.0f) {
    result.hit = true;
    result.fraction = 0.0f;
    result.startsOverlapping = true;
    result.manifolds.push_back(*overlap);

    return result;
  }

  AxisCast x;
  AxisCast y;

  if (!::cast_axis(query.min.x, query.max.x, obstacle.min.x, obstacle.max.x,
                   displacement.x, x) ||
      !::cast_axis(query.min.y, query.max.y, obstacle.min.y, obstacle.max.y,
                   displacement.y, y)) {
    return result;
  }

  const double enter = std::max(x.enter, y.enter);
  const double exit = std::min(x.exit, y.exit);

  // An isolated corner graze (enter == exit) cannot block movement. A contact
  // at the destination still counts when continuing the path would penetrate.
  if (enter < 0.0 || enter > 1.0 || exit <= enter) return result;

  result.hit = true;
  result.fraction = float(enter);
  const Glade::Vector3f travelled(displacement.x * result.fraction,
                                 displacement.y * result.fraction, 0.0f);
  const auto at_contact = query.translated(travelled);

  if (x.enter == enter) {
    result.manifolds.push_back(
        ::make_manifold(at_contact, obstacle, {x.normal, 0.0f, 0.0f}, 0.0f));
  }

  if (y.enter == enter) {
    result.manifolds.push_back(
        ::make_manifold(at_contact, obstacle, {0.0f, y.normal, 0.0f}, 0.0f));
  }

  return result;
}
