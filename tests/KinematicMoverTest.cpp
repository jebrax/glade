#include <glade/GladeObject.h>
#include <glade/collisions/CollisionWorld.h>
#include <glade/collisions/KinematicMover.h>
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
  RectangleCollisionShape bodyShape{0.5f, 0.5f};
  RectangleCollisionShape wallShape;
  GladeObject bodyObject{nullptr};
  GladeObject wallObject{nullptr};
  Collider &body;
  Collider &wall;
  CollisionWorld world;

  Fixture(float half_width = 0.5f, float half_height = 5.0f)
      : wallShape(half_width, half_height),
        body(bodyObject.setCollider(bodyShape, Collider::Type::KINEMATIC)),
        wall(wallObject.setCollider(wallShape)) {
    wallObject.getTransform()->setPosition(3.0f, 0.0f, 0.0f);
    world.add(body);
    world.add(wall);
  }

  Glade::Vector3f position() const { return *body.getOwner().getTransform()->position; }

  void placeWall(float x, float y) {
    wallObject.getTransform()->setPosition(x, y, 0.0f);
    world.update(wall);
  }

  void requireSeparated() const {
    Transform transform;
    body.getWorldTransform(transform);
    CollisionFilter filter;
    filter.categoryBits = body.getCategoryBits();
    filter.maskBits = body.getMaskBits();
    filter.ignoredCollider = &body;

    for (const auto &overlap : world.queryOverlaps(bodyShape, transform, filter)) {
      for (const auto &point : overlap.manifold.points) {
        ::require(point.separation >= 0.0f, "Final position must not penetrate terrain");
      }
    }
  }
};

void test_free_movement_and_transforms() {
  Fixture fixture;
  fixture.placeWall(100.0f, 100.0f);
  fixture.bodyObject.getTransform()->setPosition(-10.0f, 20.0f, 5.0f);
  fixture.body.getTransform()->setPosition(0.25f, -0.5f, 9.0f);
  fixture.bodyObject.getTransform()->setRotation(1.0f, 2.0f, 3.0f);
  fixture.bodyObject.getTransform()->setScale(2.0f, 3.0f, 4.0f);
  KinematicMover mover(fixture.world);
  const auto result = mover.move(fixture.body, {{1.0f, 2.0f, 100.0f}});
  ::require(result.status == MoveStatus::COMPLETED && result.contacts.empty(), "Free movement");
  ::require(result.requestedDisplacement.z == 100.0f &&
                result.actualDisplacement.x == 1.0f && result.actualDisplacement.y == 2.0f &&
                result.actualDisplacement.z == 0.0f, "Requested and actual displacement");
  ::require(fixture.position().x == -9.0f && fixture.position().y == 22.0f &&
                fixture.position().z == 5.0f, "Only owner's X/Y position changes");
  ::require(fixture.body.getTransform()->position->x == 0.25f &&
                fixture.body.getTransform()->position->z == 9.0f &&
                fixture.bodyObject.getTransform()->rotation->z == 3.0f &&
                fixture.bodyObject.getTransform()->scale->x == 2.0f,
            "Local offset, rotation and scale are preserved");
  const CollisionBounds current{{-9.0f, 21.0f}, {-8.5f, 22.0f}};
  ::require(fixture.world.queryBounds(current).size() == 1, "World bounds refreshed after move");
  const auto zero = mover.move(fixture.body, {{0.0f, 0.0f, 7.0f}});
  ::require(zero.status == MoveStatus::COMPLETED && zero.actualDisplacement.x == 0.0f &&
                zero.actualDisplacement.y == 0.0f, "Zero and Z-only movement");
  fixture.requireSeparated();
  std::cout << "PASS: free movement, owner/local transforms, world bounds, zero movement\n";
}

void test_stopping_and_sliding() {
  Fixture fixture;
  KinematicMover mover(fixture.world);
  const auto result = mover.move(fixture.body, {{4.0f, 3.0f, 0.0f}});
  ::require(result.status == MoveStatus::BLOCKED && result.contacts.size() == 1,
            "Wall blocks movement and returns contact");
  ::require(::near(fixture.position().x, 1.9999f) && ::near(fixture.position().y, 3.0f),
            "Stop with normal clearance while preserving tangential displacement");
  ::require(result.contacts[0].collider == &fixture.wall &&
                result.contacts[0].manifold.normal.x == -1.0f, "Blocking collider and normal");
  ::require(::near(result.actualDisplacement.x, fixture.position().x) &&
                ::near(result.actualDisplacement.y, fixture.position().y), "Actual movement reported");
  fixture.requireSeparated();
  const auto repeated = mover.move(fixture.body, {{4.0f, 0.0f, 0.0f}});
  ::require(repeated.status == MoveStatus::BLOCKED, "Repeated movement into wall stays blocked");
  fixture.requireSeparated();
  const auto away = mover.move(fixture.body, {{-1.0f, 0.0f, 0.0f}});
  ::require(away.status == MoveStatus::COMPLETED && ::near(away.actualDisplacement.x, -1.0f),
            "Can move away from wall");

  fixture.bodyObject.getTransform()->setPosition(2.0f, 0.0f, 0.0f);
  const auto tangent = mover.move(fixture.body, {{0.0f, 1.0f, 0.0f}});
  ::require(tangent.status == MoveStatus::COMPLETED && fixture.position().x == 2.0f &&
                fixture.position().y == 1.0f, "Exact touching allows tangential movement");
  fixture.requireSeparated();

  Fixture offset_fixture;
  offset_fixture.body.getTransform()->setPosition(1.0f, 0.0f, 0.0f);
  KinematicMover offset_mover(offset_fixture.world);
  offset_mover.move(offset_fixture.body, {{4.0f, 0.0f, 0.0f}});
  ::require(::near(offset_fixture.position().x, 0.9999f), "Local offset affects stopping distance");
  offset_fixture.requireSeparated();
  std::cout << "PASS: wall stopping, diagonal sliding, repeated contact, departure, local offsets\n";
}

void test_thin_obstacles_and_directions() {
  Fixture fixture(0.01f, 5.0f);
  KinematicMover mover(fixture.world);
  const auto thin = mover.move(fixture.body, {{100.0f, 0.0f, 0.0f}});
  ::require(thin.status == MoveStatus::BLOCKED && ::near(fixture.position().x, 2.4899f),
            "High speed cannot skip a thin obstacle");
  fixture.requireSeparated();

  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  fixture.placeWall(-3.0f, 0.0f);
  mover.move(fixture.body, {{-100.0f, 0.0f, 0.0f}});
  ::require(::near(fixture.position().x, -2.4899f), "Negative X movement");
  fixture.requireSeparated();

  Fixture horizontal(5.0f, 0.01f);
  horizontal.placeWall(0.0f, 3.0f);
  KinematicMover horizontal_mover(horizontal.world);
  horizontal_mover.move(horizontal.body, {{0.0f, 100.0f, 0.0f}});
  ::require(::near(horizontal.position().y, 2.4899f), "Positive Y movement");
  horizontal.requireSeparated();
  horizontal.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  horizontal.placeWall(0.0f, -3.0f);
  horizontal_mover.move(horizontal.body, {{0.0f, -100.0f, 0.0f}});
  ::require(::near(horizontal.position().y, -2.4899f), "Negative Y movement");
  horizontal.requireSeparated();
  std::cout << "PASS: thin obstacles at speed, positive/negative movement on both axes\n";
}

void test_corners_and_iteration_limit() {
  Fixture fixture;
  RectangleCollisionShape horizontal_shape(5.0f, 0.5f);
  GladeObject horizontal_object(nullptr);
  auto &horizontal = horizontal_object.setCollider(horizontal_shape);
  horizontal_object.getTransform()->setPosition(0.0f, 3.0f, 0.0f);
  fixture.world.add(horizontal);
  KinematicMover mover(fixture.world);
  const auto corner = mover.move(fixture.body, {{4.0f, 4.0f, 0.0f}});
  ::require(corner.status == MoveStatus::BLOCKED && corner.contacts.size() == 2,
            "Simultaneous corner retains both collider contacts");
  ::require(::near(fixture.position().x, 1.9999f) && ::near(fixture.position().y, 1.9999f),
            "Both corner normals constrain movement");
  fixture.requireSeparated();

  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  horizontal_object.getTransform()->setPosition(0.0f, 4.0f, 0.0f);
  fixture.world.update(horizontal);
  const auto sequential = mover.move(fixture.body, {{4.0f, 4.0f, 0.0f}});
  ::require(sequential.status == MoveStatus::BLOCKED && sequential.contacts.size() == 2 &&
                ::near(fixture.position().x, 1.9999f) && ::near(fixture.position().y, 2.9999f),
            "Recast sliding movement to detect a second obstacle");
  fixture.requireSeparated();

  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  KinematicMoverSettings settings;
  settings.maxMoveIterations = 1;
  KinematicMover limited(fixture.world, settings);
  const auto limit = limited.move(fixture.body, {{4.0f, 4.0f, 0.0f}});
  ::require(limit.status == MoveStatus::ITERATION_LIMIT && fixture.position().y < 2.1f,
            "Iteration exhaustion leaves remaining displacement unapplied");
  fixture.requireSeparated();
  fixture.world.remove(horizontal);
  std::cout << "PASS: simultaneous corners, sliding into another wall, movement iteration bound\n";
}

void test_overlap_recovery() {
  Fixture fixture;
  fixture.placeWall(0.75f, 0.0f);
  KinematicMover mover(fixture.world);
  const auto move = mover.move(fixture.body, {{-10.0f, 0.0f, 0.0f}});
  ::require(move.status == MoveStatus::UNRESOLVED_OVERLAP && move.contacts.size() == 1 &&
                fixture.position().x == 0.0f && move.actualDisplacement.x == 0.0f,
            "Movement reports initial overlap without automatic recovery");
  const auto recovery = mover.recoverOverlap(fixture.body);
  ::require(recovery.status == RecoveryStatus::RECOVERED && recovery.remainingOverlaps.empty() &&
                ::near(recovery.correction.x, -0.2501f) &&
                ::near(fixture.position().x, recovery.correction.x), "Explicit recovery separates");
  fixture.requireSeparated();
  ::require(mover.recoverOverlap(fixture.body).status == RecoveryStatus::ALREADY_SEPARATED,
            "Separated recovery does nothing");
  ::require(mover.move(fixture.body, {{-1.0f, 0.0f, 0.0f}}).status == MoveStatus::COMPLETED,
            "Ordinary movement works after recovery");

  fixture.bodyObject.getTransform()->setPosition(-0.25f, 0.0f, 0.0f);
  ::require(mover.recoverOverlap(fixture.body).status == RecoveryStatus::ALREADY_SEPARATED,
            "Touching is not penetration");
  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  KinematicMoverSettings settings;
  settings.maxRecoveryDistance = 0.125f;
  KinematicMover limited(fixture.world, settings);
  const auto partial = limited.recoverOverlap(fixture.body);
  ::require(partial.status == RecoveryStatus::UNRESOLVED && partial.remainingOverlaps.size() == 1 &&
                partial.correction.x == -0.125f && fixture.position().x == -0.125f,
            "Distance bound reports partial correction and remaining penetration");
  settings.maxRecoveryDistance = 0.0f;
  KinematicMover no_recovery(fixture.world, settings);
  ::require(no_recovery.recoverOverlap(fixture.body).status == RecoveryStatus::UNRESOLVED &&
                fixture.position().x == -0.125f, "Zero distance disables correction");
  std::cout << "PASS: initial overlap reporting, explicit recovery, touching, recovery distance bound\n";
}

void test_multiple_overlaps_and_trapping() {
  Fixture fixture;
  fixture.placeWall(0.75f, 0.0f);
  RectangleCollisionShape horizontal_shape(5.0f, 0.5f);
  GladeObject horizontal_object(nullptr);
  auto &horizontal = horizontal_object.setCollider(horizontal_shape);
  horizontal_object.getTransform()->setPosition(0.0f, 0.75f, 0.0f);
  fixture.world.add(horizontal);
  KinematicMoverSettings settings;
  settings.maxRecoveryIterations = 1;
  KinematicMover limited(fixture.world, settings);
  const auto first = limited.recoverOverlap(fixture.body);
  ::require(first.status == RecoveryStatus::UNRESOLVED && first.remainingOverlaps.size() == 1,
            "One correction cannot claim success while another overlap remains");
  KinematicMover mover(fixture.world);
  ::require(mover.recoverOverlap(fixture.body).status == RecoveryStatus::RECOVERED,
            "Recovery re-queries and handles the remaining obstacle");
  fixture.requireSeparated();
  fixture.world.remove(horizontal);

  RectangleCollisionShape other_shape(0.5f, 5.0f);
  GladeObject other_object(nullptr);
  auto &other = other_object.setCollider(other_shape);
  other_object.getTransform()->setPosition(-0.75f, 0.0f, 0.0f);
  fixture.world.add(other);
  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  const auto trapped = mover.recoverOverlap(fixture.body);
  ::require(trapped.status == RecoveryStatus::UNRESOLVED && !trapped.remainingOverlaps.empty() &&
                std::abs(trapped.correction.x) <= 1.0f, "Trapped recovery terminates and reports failure");
  fixture.world.remove(other);
  std::cout << "PASS: multiple overlaps, recovery iteration bound, trapped-body failure\n";
}

void test_filters_and_invalid_inputs() {
  Fixture fixture;
  KinematicMover mover(fixture.world);
  fixture.wall.setSensor(true);
  ::require(mover.move(fixture.body, {{4.0f, 0.0f, 0.0f}}).status == MoveStatus::COMPLETED,
            "Sensors do not block movement");
  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  fixture.wall.setSensor(false);
  fixture.body.setMaskBits(2);
  ::require(mover.move(fixture.body, {{4.0f, 0.0f, 0.0f}}).status == MoveStatus::COMPLETED,
            "Mover uses the body's collision mask");
  fixture.bodyObject.getTransform()->setPosition(0.0f, 0.0f, 0.0f);
  fixture.body.setMaskBits(~std::uint32_t{0});
  fixture.wall.setMaskBits(2);
  ::require(mover.move(fixture.body, {{4.0f, 0.0f, 0.0f}}).status == MoveStatus::COMPLETED,
            "Mover uses the body's collision category against the target mask");

  ::rejects([&] { mover.move(fixture.wall, {{1.0f, 0.0f, 0.0f}}); });
  fixture.body.setSensor(true);
  ::rejects([&] { mover.recoverOverlap(fixture.body); });
  fixture.body.setSensor(false);
  fixture.body.setEnabled(false);
  ::rejects([&] { mover.move(fixture.body, {{1.0f, 0.0f, 0.0f}}); });
  fixture.body.setEnabled(true);
  ::rejects([&] {
    mover.move(fixture.body, {{std::numeric_limits<float>::quiet_NaN(), 0.0f, 0.0f}});
  });
  fixture.world.remove(fixture.body);
  const auto before = fixture.position();
  ::rejects([&] { mover.move(fixture.body, {{1.0f, 0.0f, 0.0f}}); });
  ::require(fixture.position().x == before.x, "Unregistered body rejected before mutation");
  KinematicMoverSettings settings;
  settings.contactTolerance = 0.0f;
  ::rejects([&] { KinematicMover invalid(fixture.world, settings); });
  settings.contactTolerance = 0.0001f;
  settings.maxMoveIterations = 0;
  ::rejects([&] { KinematicMover invalid(fixture.world, settings); });
  settings.maxMoveIterations = 8;
  settings.maxRecoveryDistance = -1.0f;
  ::rejects([&] { KinematicMover invalid(fixture.world, settings); });
  std::cout << "PASS: sensors, category/mask filtering, collider preconditions, invalid settings/input\n";
}

}

int main() {
  try {
    ::test_free_movement_and_transforms();
    ::test_stopping_and_sliding();
    ::test_thin_obstacles_and_directions();
    ::test_corners_and_iteration_limit();
    ::test_overlap_recovery();
    ::test_multiple_overlaps_and_trapping();
    ::test_filters_and_invalid_inputs();
    std::cout << "All kinematic mover tests passed.\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
