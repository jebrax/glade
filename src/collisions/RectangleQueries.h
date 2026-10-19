#pragma once

#include <optional>
#include <vector>

#include <glade/collisions/CollisionBounds.h>
#include <glade/collisions/CollisionResults.h>

// Internal narrowphase for the currently supported shape pair.
struct RectangleCastResult {
  bool hit = false;
  float fraction = 1.0f;
  bool startsOverlapping = false;
  std::vector<CollisionManifold> manifolds;
};

std::optional<CollisionManifold> overlap_rectangles(
    const CollisionBounds &query, const CollisionBounds &obstacle);

RectangleCastResult cast_rectangles(
    const CollisionBounds &query, const Glade::Vector3f &displacement,
    const CollisionBounds &obstacle);
