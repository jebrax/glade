#include <glade/collisions/CollisionWorld.h>
#include <glade/collisions/Collider.h>
#include <glade/collisions/CollisionShape.h>

#include "RectangleQueries.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {

void require_supported_shape(const CollisionShape &shape) {
  switch (shape.getType()) {
  case CollisionShape::Type::RECTANGLE:
    return;
  }

  throw std::invalid_argument("Unsupported collision shape type");
}

CollisionBounds collider_bounds(const Collider &collider) {
  ::require_supported_shape(collider.getShape());
  Transform world_transform;
  collider.getWorldTransform(world_transform);

  return collider.getShape().computeBounds(world_transform);
}

}

void CollisionWorld::add(Collider &collider) {
  const auto entry = std::find_if(entries.begin(), entries.end(), [&](const Entry &candidate) {
    return candidate.collider == &collider;
  });

  if (entry != entries.end()) return;

  entries.push_back({&collider, ::collider_bounds(collider)});
}

void CollisionWorld::remove(Collider &collider) {
  std::erase_if(entries, [&](const Entry &entry) { return entry.collider == &collider; });
}

void CollisionWorld::clear() {
  entries.clear();
}

void CollisionWorld::update(Collider &collider) {
  const auto entry = std::find_if(entries.begin(), entries.end(), [&](const Entry &candidate) {
    return candidate.collider == &collider;
  });

  if (entry == entries.end()) {
    throw std::invalid_argument("Cannot update an unregistered collider");
  }

  entry->bounds = ::collider_bounds(collider);
}

std::vector<const Collider *> CollisionWorld::queryBounds(
    const CollisionBounds &bounds, const CollisionFilter &filter) const {
  std::vector<const Collider *> result;

  for (const auto &entry : entries) {
    if (filter.accepts(*entry.collider) && bounds.intersects(entry.bounds)) {
      result.push_back(entry.collider);
    }
  }

  return result;
}

std::vector<OverlapResult> CollisionWorld::queryOverlaps(
    const CollisionShape &shape, const Transform &world_transform,
    const CollisionFilter &filter) const {
  ::require_supported_shape(shape);
  const auto bounds = shape.computeBounds(world_transform);
  std::vector<OverlapResult> result;

  for (const auto &entry : entries) {
    if (!filter.accepts(*entry.collider) || !bounds.intersects(entry.bounds)) continue;

    auto manifold = ::overlap_rectangles(bounds, entry.bounds);

    if (manifold) {
      result.push_back({entry.collider, std::move(*manifold)});
    }
  }

  return result;
}

ShapeCastResult CollisionWorld::castShape(
    const CollisionShape &shape, const Transform &world_transform,
    const Glade::Vector3f &displacement, const CollisionFilter &filter) const {
  ::require_supported_shape(shape);

  if (!std::isfinite(displacement.x) || !std::isfinite(displacement.y)) {
    throw std::invalid_argument("Collision cast X/Y displacement must be finite");
  }

  const auto bounds = shape.computeBounds(world_transform);
  const auto swept_bounds = bounds.merged(bounds.translated(displacement));
  ShapeCastResult result;

  for (const auto &entry : entries) {
    if (!filter.accepts(*entry.collider) || !swept_bounds.intersects(entry.bounds)) continue;

    auto hit = ::cast_rectangles(bounds, displacement, entry.bounds);

    if (!hit.hit || (result.hit && hit.fraction > result.fraction)) continue;

    if (!result.hit || hit.fraction < result.fraction) {
      result.contacts.clear();
      result.startsOverlapping = false;
    }

    result.hit = true;
    result.fraction = hit.fraction;
    result.startsOverlapping = result.startsOverlapping || hit.startsOverlapping;

    for (auto &manifold : hit.manifolds) {
      result.contacts.push_back({entry.collider, std::move(manifold)});
    }
  }

  return result;
}
