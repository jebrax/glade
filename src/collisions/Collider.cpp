#include <glade/collisions/Collider.h>
#include <glade/GladeObject.h>

Collider::Collider(GladeObject &owner_, const CollisionShape &shape_, Type type_)
    : owner(owner_), shape(shape_), type(type_) {}

void Collider::getWorldTransform(Transform &result_transform) const {
  Transform::combine(*owner.getTransform(), transform, false, false, false, result_transform);
}
