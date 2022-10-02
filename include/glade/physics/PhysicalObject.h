#pragma once

#include "../math/Vector.h"

class GladeObject;

class PhysicalObject
{
  public:
    enum Type {
      STATIC,
      KINEMATIC
    };

    PhysicalObject(GladeObject *entity, Type type = Type::STATIC):
      affectedByGravity(false),
      entity(entity),
      type(type)
    {}

    Type getType() { return type; }

    virtual void* getGjkSupportFunction() = 0;

    bool affectedByGravity;
    GladeObject *entity;

  private:
    Type type;
};

