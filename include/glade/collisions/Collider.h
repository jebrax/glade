#pragma once

#include <cstdint>

#include <glade/math/Transform.h>

class GladeObject;
class CollisionShape;

// Owner and shape must outlive the collider. Registered colliders must stay at
// a stable address and be removed from their world before destruction.
class Collider {
public:
  enum class Type {
    STATIC,
    KINEMATIC
  };

  Collider(GladeObject &owner_, const CollisionShape &shape_, Type type_ = Type::STATIC);

  Collider(const Collider &) = delete;

  Collider &operator=(const Collider &) = delete;

  GladeObject &getOwner() const { return owner; }
  const CollisionShape &getShape() const { return shape; }
  Type getType() const { return type; }
  Transform *getTransform() { return &transform; }
  const Transform *getTransform() const { return &transform; }

  void getWorldTransform(Transform &result_transform) const;

  void setEnabled(bool enabled_) { enabled = enabled_; }
  bool isEnabled() const { return enabled; }
  void setSensor(bool sensor_) { sensor = sensor_; }
  bool isSensor() const { return sensor; }
  void setCategoryBits(std::uint32_t category_bits) { categoryBits = category_bits; }
  std::uint32_t getCategoryBits() const { return categoryBits; }
  void setMaskBits(std::uint32_t mask_bits) { maskBits = mask_bits; }
  std::uint32_t getMaskBits() const { return maskBits; }

private:
  GladeObject &owner;
  const CollisionShape &shape;
  Transform transform;
  Type type;
  bool enabled = true;
  bool sensor = false;
  std::uint32_t categoryBits = 1;
  std::uint32_t maskBits = ~std::uint32_t{0};
};
