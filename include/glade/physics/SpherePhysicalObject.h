#pragma once

#include <glade/physics/PhysicalObject.h>

class SpherePhysicalObject : public PhysicalObject {
public:
  SpherePhysicalObject(GladeObject *entity_, float radius_,
                       PhysicalObject::Type type_ = PhysicalObject::Type::STATIC)
      : PhysicalObject(entity_, type_), radius(radius_) {}

  void *getGjkSupportFunction() override;

  float radius;
};
