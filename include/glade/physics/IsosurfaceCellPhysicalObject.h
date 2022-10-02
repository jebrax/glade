#pragma once

#include <glade/physics/PhysicalObject.h>

class IsosurfaceCellPhysicalObject: public PhysicalObject
{
  public:
    IsosurfaceCellPhysicalObject(GladeObject *entity): PhysicalObject(entity) {}
    void* getGjkSupportFunction() override;
};

