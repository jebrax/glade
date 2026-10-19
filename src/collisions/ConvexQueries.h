#pragma once

#include <optional>

#include <glade/collisions/CollisionShape.h>

#include "NarrowphaseCastResult.h"

// Rectangle/polygon and polygon/polygon queries. Translation only.
std::optional<CollisionManifold> overlap_convex_shapes(
    const CollisionShape &query, const Transform &query_transform,
    const CollisionShape &obstacle, const Transform &obstacle_transform);

NarrowphaseCastResult cast_convex_shapes(
    const CollisionShape &query, const Transform &query_transform,
    const Glade::Vector3f &displacement,
    const CollisionShape &obstacle, const Transform &obstacle_transform);
