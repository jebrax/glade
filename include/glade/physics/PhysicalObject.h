#pragma once

#include "../math/Vector.h"

class GladeObject;

class PhysicalObject {
public:
  enum Type { STATIC, KINEMATIC };

  PhysicalObject(GladeObject *entity_, Type type_ = Type::STATIC)
      : affectedByGravity(false), entity(entity_), type(type_) {}

  Type getType() { return type; }
  virtual void *getGjkSupportFunction() = 0;

  bool affectedByGravity;
  GladeObject *entity;

private:
  Type type;
};
