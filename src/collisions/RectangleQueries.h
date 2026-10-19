#pragma once

#include <optional>
#include <vector>

#include <glade/collisions/CollisionBounds.h>
#include <glade/collisions/CollisionResults.h>

#include "NarrowphaseCastResult.h"

std::optional<CollisionManifold> overlap_rectangles(
    const CollisionBounds &query, const CollisionBounds &obstacle);

NarrowphaseCastResult cast_rectangles(
    const CollisionBounds &query, const Glade::Vector3f &displacement,
    const CollisionBounds &obstacle);
