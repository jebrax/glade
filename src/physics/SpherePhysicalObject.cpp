#include <glade/physics/SpherePhysicalObject.h>
#include <glade/GladeObject.h>

#include <ccd/ccd.h>
#include <ccd/quat.h>

static void sphereSupportFunction(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point)
{
  // Sphere radius must be in the SpherePhysicalObject class, and void *obj should be of this type
  SpherePhysicalObject *sphere = (SpherePhysicalObject*) obj;
  Glade::Vector3f direction(ccdVec3X(dir), ccdVec3Y(dir), ccdVec3Z(dir));

  direction.normalize();
  direction.scale(sphere->radius);

  Glade::Vector3f resultPoint;
  GladeObject* entity = sphere->entity;
  resultPoint.add(*entity->getTransform()->position.get());
  resultPoint.add(direction);

  ccdVec3Set(point, resultPoint.x, resultPoint.y, resultPoint.z);
}

void* SpherePhysicalObject::getGjkSupportFunction() {
  return (void*) sphereSupportFunction;
}
