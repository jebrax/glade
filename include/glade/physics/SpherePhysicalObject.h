#pragma once

#include <glade/physics/PhysicalObject.h>

class SpherePhysicalObject: public PhysicalObject
{
  public:
    SpherePhysicalObject(GladeObject *entity, float radius, PhysicalObject::Type type = PhysicalObject::Type::STATIC): PhysicalObject(entity, type), radius(radius) {}
    void* getGjkSupportFunction() override;

    float radius;
};

