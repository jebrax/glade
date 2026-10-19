#include <glade/GladeObject.h>
#include <glade/collisions/CollisionWorld.h>
#include <glade/collisions/RectangleCollisionShape.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}

bool near(float actual, float expected) {
  return std::abs(actual - expected) < 0.00001f;
}

bool normal_is(const CollisionManifold &manifold, float x, float y) {
  return manifold.normal.x == x && manifold.normal.y == y && manifold.normal.z == 0.0f;
}

template <class Function> void rejects(Function function) {
  bool rejected = false;

  try {
    function();
  } catch (const std::invalid_argument &) {
    rejected = true;
  }

  ::require(rejected, "Expected std::invalid_argument");
}

struct Fixture {
  RectangleCollisionShape movingShape{0.5f, 0.5f};
  RectangleCollisionShape obstacleShape;
  GladeObject movingObject{nullptr};
  GladeObject obstacleObject{nullptr};
  Collider &moving;
  Collider &obstacle;
  CollisionWorld world;
  CollisionFilter filter;

  Fixture(float half_width = 0.5f, float half_height = 0.5f)
      : obstacleShape(half_width, half_height),
        moving(movingObject.setCollider(movingShape, Collider::Type::KINEMATIC)),
        obstacle(obstacleObject.setCollider(obstacleShape)) {
    obstacleObject.getTransform()->setPosition(3.0f, 0.0f, 0.0f);
    world.add(moving);
    world.add(obstacle);
    filter.ignoredCollider = &moving;
  }

  void placeObstacle(float x, float y, float z = 0.0f) {
    obstacleObject.getTransform()->setPosition(x, y, z);
    world.update(obstacle);
  }

  std::vector<OverlapResult> overlaps() const {
    Transform transform;
    moving.getWorldTransform(transform);

    return world.queryOverlaps(movingShape, transform, filter);
  }

  ShapeCastResult cast(float x, float y, float z = 0.0f) const {
    Transform transform;
    moving.getWorldTransform(transform);

    return world.castShape(movingShape, transform, {x, y, z}, filter);
  }
};

void test_objects_and_manifolds() {
  Fixture fixture;
  ::require(fixture.movingObject.getCollider() == &fixture.moving, "Object owns its collider");
  const GladeObject &const_object = fixture.movingObject;
  ::require(const_object.getCollider() == &fixture.moving, "Const collider access");
  ::require(fixture.moving.getType() == Collider::Type::KINEMATIC, "Kinematic type");
  ::require(fixture.obstacle.getType() == Collider::Type::STATIC, "Static type");
  ::require(fixture.overlaps().empty(), "Separated objects must not overlap");

  fixture.placeObstacle(0.75f, 0.0f);
  const auto overlaps = fixture.overlaps();
  ::require(overlaps.size() == 1, "Detect the other object's collider, not self");
  ::require(overlaps[0].collider == &fixture.obstacle, "Contact identifies the obstacle");
  const auto &manifold = overlaps[0].manifold;
  ::require(::normal_is(manifold, -1.0f, 0.0f), "Normal points from obstacle to query");
  ::require(manifold.points.size() == 2, "Face contact has two endpoints");
  ::require(::near(manifold.points[0].separation, -0.25f), "Signed penetration");
  ::require(::near(manifold.points[0].position.x, 0.25f), "Contact lies on obstacle face");
  ::require(::near(manifold.points[0].position.y, -0.5f) &&
                ::near(manifold.points[1].position.y, 0.5f), "Contact segment endpoints");
  ::require(fixture.movingObject.getTransform()->position->x == 0.0f &&
                fixture.obstacleObject.getTransform()->position->x == 0.75f,
            "Detection must not resolve or move objects");

  fixture.placeObstacle(-0.75f, 0.0f);
  ::require(::normal_is(fixture.overlaps()[0].manifold, 1.0f, 0.0f), "Rightward normal");
  fixture.placeObstacle(0.0f, 0.75f);
  ::require(::normal_is(fixture.overlaps()[0].manifold, 0.0f, -1.0f), "Downward normal");
  fixture.placeObstacle(0.0f, -0.75f);
  ::require(::normal_is(fixture.overlaps()[0].manifold, 0.0f, 1.0f), "Upward normal");

  fixture.placeObstacle(0.0f, 0.0f);
  RectangleCollisionShape small_shape(0.25f, 0.25f);
  Transform transform;
  const auto contained = fixture.world.queryOverlaps(small_shape, transform, fixture.filter);
  ::require(contained.size() == 1 &&
                ::near(contained[0].manifold.points[0].separation, -0.75f),
            "Containment uses distance to separation, not intersection width");
  std::cout << "PASS: two GladeObjects, separation, overlap, normals, contacts, containment\n";
}

void test_transforms_and_updates() {
  Fixture fixture;
  fixture.movingObject.getTransform()->setPosition(-10.0f, 20.0f, 50.0f);
  fixture.movingObject.getTransform()->setRotation(0.0f, 0.0f, 1.0f);
  fixture.movingObject.getTransform()->setScale(10.0f, 20.0f, 30.0f);
  fixture.moving.getTransform()->setPosition(2.0f, 3.0f, 4.0f);
  fixture.moving.getTransform()->setRotation(0.0f, 0.0f, 2.0f);
  fixture.moving.getTransform()->setScale(2.0f, 3.0f, 4.0f);
  fixture.world.update(fixture.moving);
  fixture.obstacle.getTransform()->setPosition(0.25f, 0.0f, 0.0f);
  fixture.placeObstacle(-7.5f, 23.0f, -100.0f);

  Transform world_transform;
  fixture.moving.getWorldTransform(world_transform);
  ::require(world_transform.position->x == -8.0f && world_transform.position->y == 23.0f &&
                world_transform.position->z == 54.0f, "Compose owner and local positions");
  ::require(world_transform.rotation->z == 3.0f && world_transform.scale->x == 20.0f,
            "Reuse Glade's transform composition convention");
  const auto bounds = fixture.movingShape.computeBounds(world_transform);
  ::require(bounds.min.x == -8.5f && bounds.max.x == -7.5f &&
                bounds.min.y == 22.5f && bounds.max.y == 23.5f,
            "Rectangle ignores Z, rotation and scale");
  ::require(fixture.overlaps().size() == 1, "Local offsets participate in collision");
  ::require(fixture.movingObject.getTransform()->position->x == -10.0f &&
                fixture.moving.getTransform()->position->x == 2.0f,
            "Composition does not mutate or alias input transforms");

  fixture.placeObstacle(100.0f, -100.0f);
  ::require(fixture.overlaps().empty(), "Updated bounds remove the old collision");
  const CollisionBounds destination{{99.0f, -101.0f}, {101.0f, -99.0f}};
  ::require(fixture.world.queryBounds(destination, fixture.filter).size() == 1,
            "Updated bounds find geometry at the new world position");
  std::cout << "PASS: parent/local transforms, ignored Z/rotation/scale, bounds updates\n";
}

void test_filters() {
  Fixture fixture;
  fixture.placeObstacle(0.75f, 0.0f);
  fixture.obstacle.setSensor(true);
  ::require(fixture.overlaps().empty() && !fixture.cast(1.0f, 0.0f).hit,
            "Sensors are excluded by default");
  fixture.filter.includeSensors = true;
  ::require(fixture.overlaps().size() == 1 && fixture.cast(1.0f, 0.0f).hit,
            "Sensors can be explicitly queried");
  fixture.obstacle.setEnabled(false);
  ::require(fixture.overlaps().empty(), "Disabled collider excluded");
  fixture.obstacle.setEnabled(true);
  fixture.obstacleObject.setEnabled(false);
  ::require(fixture.overlaps().empty(), "Disabled owner excluded");
  fixture.obstacleObject.setEnabled(true);

  fixture.filter.categoryBits = 2;
  fixture.filter.maskBits = 4;
  fixture.obstacle.setCategoryBits(4);
  fixture.obstacle.setMaskBits(2);
  ::require(fixture.overlaps().size() == 1, "Matching categories and masks");
  fixture.obstacle.setMaskBits(1);
  ::require(fixture.overlaps().empty(), "Target mask can reject query category");
  fixture.obstacle.setMaskBits(2);
  fixture.filter.maskBits = 1;
  ::require(fixture.overlaps().empty(), "Query mask can reject target category");
  fixture.filter.maskBits = 4;
  fixture.filter.ignoredCollider = &fixture.obstacle;
  ::require(fixture.overlaps().empty(), "Explicit collider exclusion");
  std::cout << "PASS: sensors, enabled state, owner state, two-way masks, self exclusion\n";
}

void test_swept_detection() {
  Fixture fixture(0.01f, 0.5f);
  const auto hit = fixture.cast(10.0f, 0.0f, 100.0f);
  ::require(hit.hit && !hit.startsOverlapping && ::near(hit.fraction, 0.249f),
            "Detect a thin obstacle crossed in one update");
  ::require(hit.contacts.size() == 1 && hit.contacts[0].collider == &fixture.obstacle,
            "Cast identifies the collider");
  ::require(::normal_is(hit.contacts[0].manifold, -1.0f, 0.0f), "Swept contact normal");
  ::require(::near(hit.contacts[0].manifold.points[0].position.x, 2.99f) &&
                hit.contacts[0].manifold.points[0].separation == 0.0f,
            "Cast contact lies on obstacle, with zero separation");
  ::require(!fixture.cast(10.0f, 10.0f).hit,
            "Broadphase swept bounds intersection is not itself a collision");
  ::require(!fixture.cast(0.0f, 0.0f, 100.0f).hit, "Z-only movement is ignored");
  ::require(fixture.movingObject.getTransform()->position->x == 0.0f,
            "A cast never applies displacement");

  fixture.placeObstacle(-3.0f, 0.0f);
  const auto reverse = fixture.cast(-10.0f, 0.0f);
  ::require(reverse.hit && ::near(reverse.fraction, 0.249f) &&
                ::normal_is(reverse.contacts[0].manifold, 1.0f, 0.0f),
            "Negative movement and negative world coordinates");
  fixture.placeObstacle(0.0f, 3.0f);
  ::require(fixture.cast(0.0f, 10.0f).hit, "Y-axis cast");
  std::cout << "PASS: swept thin obstacles, narrowphase rejection, both axes/directions\n";
}

void test_touching_and_corners() {
  Fixture fixture;
  fixture.placeObstacle(1.0f, 0.0f);
  ::require(fixture.overlaps().size() == 1 &&
                fixture.overlaps()[0].manifold.points[0].separation == 0.0f,
            "Overlap queries include touching");
  const auto inward = fixture.cast(1.0f, 0.0f);
  ::require(inward.hit && inward.fraction == 0.0f && !inward.startsOverlapping,
            "Touching contact blocks inward movement without claiming penetration");
  ::require(!fixture.cast(-1.0f, 0.0f).hit, "Can move away from touching face");
  ::require(!fixture.cast(0.0f, 1.0f).hit, "Can slide along touching face");
  ::require(!fixture.cast(0.0f, 0.0f).hit, "Stationary touching is not a blocking cast");

  fixture.placeObstacle(2.0f, 0.0f);
  ::require(fixture.cast(1.0f, 0.0f).hit && fixture.cast(1.0f, 0.0f).fraction == 1.0f,
            "Contact at the destination is included");
  ::require(!fixture.cast(2.0f, 2.0f).hit, "Isolated corner graze does not block");
  fixture.placeObstacle(2.0f, 2.0f);
  const auto corner = fixture.cast(2.0f, 2.0f);
  ::require(corner.hit && corner.fraction == 0.5f && corner.contacts.size() == 2,
            "Simultaneous faces on one collider preserve both normals");
  ::require(::normal_is(corner.contacts[0].manifold, -1.0f, 0.0f) &&
                ::normal_is(corner.contacts[1].manifold, 0.0f, -1.0f), "Corner normals");
  fixture.placeObstacle(1.0f, 1.0f);
  ::require(!fixture.cast(1.0f, -1.0f).hit, "Can move away from a touching corner");
  ::require(fixture.cast(1.0f, 1.0f).contacts.size() == 2, "Inward touching corner");
  fixture.placeObstacle(0.75f, 0.0f);
  const auto penetrating = fixture.cast(0.0f, 0.0f);
  ::require(penetrating.hit && penetrating.startsOverlapping && penetrating.fraction == 0.0f &&
                penetrating.contacts[0].manifold.points[0].separation < 0.0f,
            "Initial penetration is reported even with zero displacement");
  std::cout << "PASS: touching, sliding queries, endpoint hits, corner contacts, initial overlap\n";
}

void test_earliest_and_simultaneous_hits() {
  Fixture fixture(0.5f, 5.0f);
  fixture.placeObstacle(4.0f, 0.0f);
  RectangleCollisionShape horizontal_shape(5.0f, 0.5f);
  GladeObject horizontal_object(nullptr);
  auto &horizontal = horizontal_object.setCollider(horizontal_shape);
  horizontal_object.getTransform()->setPosition(0.0f, 2.0f, 0.0f);
  fixture.world.add(horizontal);

  const auto earliest = fixture.cast(4.0f, 4.0f);
  ::require(earliest.hit && earliest.fraction == 0.25f && earliest.contacts.size() == 1 &&
                earliest.contacts[0].collider == &horizontal,
            "Return nearest contact, not first registered collider");
  fixture.placeObstacle(2.0f, 0.0f);
  const auto simultaneous = fixture.cast(4.0f, 4.0f);
  ::require(simultaneous.contacts.size() == 2 && simultaneous.fraction == 0.25f,
            "Preserve simultaneous contacts against separate colliders");
  fixture.world.remove(fixture.obstacle);
  fixture.world.add(fixture.obstacle);
  ::require(fixture.cast(4.0f, 4.0f).contacts.size() == 2,
            "Contact collection is independent of registration order");

  fixture.world.remove(horizontal);
  std::cout << "PASS: nearest hit and simultaneous contacts across multiple colliders\n";
}

void test_registration_and_validation() {
  Fixture fixture;
  const CollisionBounds all{{-10.0f, -10.0f}, {10.0f, 10.0f}};
  fixture.world.add(fixture.obstacle);
  ::require(fixture.world.queryBounds(all).size() == 2, "Duplicate registration is a no-op");
  fixture.world.remove(fixture.obstacle);
  fixture.world.remove(fixture.obstacle);
  ::require(fixture.world.queryBounds(all).size() == 1, "Removal is idempotent");
  ::rejects([&] { fixture.world.update(fixture.obstacle); });
  fixture.obstacleObject.clearCollider();
  ::require(fixture.obstacleObject.getCollider() == nullptr, "Collider is optional");
  fixture.world.clear();
  ::require(fixture.world.queryBounds(all).empty() && !fixture.cast(10.0f, 0.0f).hit,
            "Clear removes registrations without destroying objects");
  ::require(fixture.movingObject.getCollider() == &fixture.moving,
            "World does not own colliders");

  ::rejects([] { RectangleCollisionShape invalid(0.0f, 1.0f); });
  ::rejects([] { RectangleCollisionShape invalid(1.0f, -1.0f); });
  ::rejects([] {
    RectangleCollisionShape invalid(std::numeric_limits<float>::infinity(), 1.0f);
  });
  ::rejects([&] { fixture.cast(std::numeric_limits<float>::quiet_NaN(), 0.0f); });
  Transform invalid_transform;
  invalid_transform.setPosition(std::numeric_limits<float>::infinity(), 0.0f, 0.0f);
  ::rejects([&] { fixture.world.queryOverlaps(fixture.movingShape, invalid_transform); });
  std::cout << "PASS: registration lifetime, optional collider, invalid geometry rejection\n";
}

}

int main() {
  try {
    ::test_objects_and_manifolds();
    ::test_transforms_and_updates();
    ::test_filters();
    ::test_swept_detection();
    ::test_touching_and_corners();
    ::test_earliest_and_simultaneous_hits();
    ::test_registration_and_validation();
    std::cout << "All collision detection tests passed.\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
