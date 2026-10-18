#include <glade/physics/SpherePhysicalObject.h>
#include <glade/GladeObject.h>

#include <ccd/ccd.h>
#include <ccd/quat.h>

static void sphere_support_function(const void *obj, const ccd_vec3_t *dir, ccd_vec3_t *point) {
  // Sphere radius must be in the SpherePhysicalObject class, and void *obj should be of this type
  SpherePhysicalObject *sphere = (SpherePhysicalObject *)obj;
  Glade::Vector3f direction(::ccdVec3X(dir), ::ccdVec3Y(dir), ::ccdVec3Z(dir));

  direction.normalize();
  direction.scale(sphere->radius);

  Glade::Vector3f result_point;
  GladeObject *entity = sphere->entity;
  result_point.add(*entity->getTransform()->position.get());
  result_point.add(direction);

  ::ccdVec3Set(point, result_point.x, result_point.y, result_point.z);
}

void *SpherePhysicalObject::getGjkSupportFunction() { return (void *)::sphere_support_function; }
