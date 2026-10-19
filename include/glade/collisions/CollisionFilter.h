#pragma once

#include <cstdint>
#include <functional>

class Collider;

struct CollisionFilter {
  std::uint32_t categoryBits = ~std::uint32_t{0};
  std::uint32_t maskBits = ~std::uint32_t{0};
  const Collider *ignoredCollider = nullptr;
  bool includeSensors = false;
  // Optional additional candidate condition. Must not mutate the collision world.
  std::function<bool(const Collider &)> predicate;

  bool accepts(const Collider &collider) const;
};
