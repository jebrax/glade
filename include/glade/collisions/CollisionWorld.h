#pragma once

#include <vector>

#include <glade/collisions/CollisionBounds.h>
#include <glade/collisions/CollisionFilter.h>
#include <glade/collisions/CollisionResults.h>
#include <glade/math/Transform.h>

class Collider;
class CollisionShape;

// Non-owning registry. No movement, resolution or gameplay callbacks.
class CollisionWorld {
public:
  // Adding an already registered collider is a no-op.
  void add(Collider &collider);

  void remove(Collider &collider);

  void clear();

  // Call after owner/local position changes, before querying the world.
  // Updating an unregistered collider throws std::invalid_argument.
  void update(Collider &collider);

  std::vector<const Collider *> queryBounds(
      const CollisionBounds &bounds, const CollisionFilter &filter = {}) const;

  // Includes touching contacts, not just penetration.
  std::vector<OverlapResult> queryOverlaps(
      const CollisionShape &shape, const Transform &world_transform,
      const CollisionFilter &filter = {}) const;

  // Translation only. Initial penetration is reported at fraction zero.
  // Existing touching contacts block only motion into the surface.
  ShapeCastResult castShape(
      const CollisionShape &shape, const Transform &world_transform,
      const Glade::Vector3f &displacement, const CollisionFilter &filter = {}) const;

private:
  struct Entry {
    Collider *collider;
    CollisionBounds bounds;
  };

  // Linear broadphase initially; cached bounds can later live in an index.
  std::vector<Entry> entries;
};
