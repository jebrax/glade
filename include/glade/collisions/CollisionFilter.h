#pragma once

#include <cstdint>

class Collider;

struct CollisionFilter {
  std::uint32_t categoryBits = ~std::uint32_t{0};
  std::uint32_t maskBits = ~std::uint32_t{0};
  const Collider *ignoredCollider = nullptr;
  bool includeSensors = false;

  bool accepts(const Collider &collider) const;
};
