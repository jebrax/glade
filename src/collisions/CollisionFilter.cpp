#include <glade/collisions/CollisionFilter.h>
#include <glade/collisions/Collider.h>
#include <glade/GladeObject.h>

bool CollisionFilter::accepts(const Collider &collider) const {
  return &collider != ignoredCollider && collider.isEnabled() &&
         collider.getOwner().isEnabled() && (includeSensors || !collider.isSensor()) &&
         (maskBits & collider.getCategoryBits()) != 0 &&
         (categoryBits & collider.getMaskBits()) != 0 && (!predicate || predicate(collider));
}
