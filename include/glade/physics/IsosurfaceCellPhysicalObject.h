#pragma once

#include <glade/physics/PhysicalObject.h>

class IsosurfaceCellPhysicalObject : public PhysicalObject {
public:
  IsosurfaceCellPhysicalObject(GladeObject *entity_) : PhysicalObject(entity_) {}
  void *getGjkSupportFunction() override;
};
