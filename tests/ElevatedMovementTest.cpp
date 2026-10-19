#include <glade/ElevatedObject.h>
#include <glade/collisions/KinematicMover.h>
#include <glade/collisions/CollisionWorld.h>
#include <glade/collisions/RectangleCollisionShape.h>
#include <glade/collisions/ConvexPolygonCollisionShape.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}

bool near(float actual, float expected) { return std::abs(actual - expected) < 0.00001f; }

class TestObject final : public ElevatedObject {
public:
  explicit TestObject(float height = 0.0f) : ElevatedObject(height) {}
  ~TestObject() override { clearCollider(); indexById.erase(getId()); }

private:
  void updateElevation() override {}
};

struct Fixture {
  RectangleCollisionShape actorShape{0.125f, 0.0625f};
  RectangleCollisionShape boxShape{0.25f, 0.125f};
  TestObject actor;
  TestObject box;
  CollisionWorld world;
  KinematicMover mover{world};

  explicit Fixture(float actor_height = 0.0f, float box_height = 0.3f)
      : actor(actor_height), box(box_height) {
    actor.setCollider(actorShape, Collider::Type::KINEMATIC);
    box.setCollider(boxShape);
    box.setSupportingSurface(true);
    actor.getTransform()->setPosition(-1.0f, 0.0f, -7.0f);
    world.add(*actor.getCollider());
    world.add(*box.getCollider());
  }

  ~Fixture() { world.clear(); }

  MoveResult move(float x = 0.0f, float y = 0.0f, float elevation = 0.0f) {
    return mover.move(*actor.getCollider(), {{x, y, 99.0f}, elevation});
  }

  void place(float x, float y, float elevation) {
    actor.getTransform()->setPosition(x, y, -7.0f);
    actor.setElevation(elevation);
    world.update(*actor.getCollider());
  }
};

void test_sides_clearance_and_landing() {
  Fixture f;
  ::require(f.move(1.0f).status == MoveStatus::BLOCKED &&
                f.actor.getTransform()->position->x < -0.375f && f.actor.isGrounded(),
            "Box did not block ground entry");
  f.place(-1.0f, 0.0f, 0.2f);
  ::require(f.move(1.0f).status == MoveStatus::BLOCKED, "Low airborne body passed through a side");
  f.place(-1.0f, 0.0f, 0.3f);
  ::require(f.move(1.0f).status == MoveStatus::COMPLETED && f.actor.isGrounded(),
            "Exact top clearance did not permit movement and establish support");
  f.place(-1.0f, 0.0f, 0.4f);
  ::require(f.move(1.0f).status == MoveStatus::COMPLETED && !f.actor.isGrounded(),
            "Above-top passage blocked or invented support");
  const auto landing = f.move(0.0f, 0.0f, -100.0f);
  ::require(landing.status == MoveStatus::BLOCKED && f.actor.getElevation() == 0.3f &&
                f.actor.isGrounded() && ::near(landing.actualElevationDisplacement, -0.1f),
            "Large descent skipped the supporting top");
  ::require(f.actor.getTransform()->position->z == -7.0f, "Elevation changed render Z");
  const auto hold = f.move(0.0f, 0.0f, -1.0f);
  ::require(hold.actualElevationDisplacement == 0.0f && f.actor.isGrounded(),
            "Repeated descent sank through support");
  f.move();
  ::require(f.actor.isGrounded(), "Zero request lost support");
  f.move(1.0f);
  ::require(!f.actor.isGrounded() && f.actor.getElevation() == 0.3f,
            "Walking off support did not clear grounded or introduced gravity");
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.0f && f.actor.isGrounded(), "Reference floor landing failed");
}

void test_edges_and_staged_movement() {
  Fixture f;
  f.place(0.374f, 0.0f, 0.5f);
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.3f && f.actor.isGrounded(),
            "Positive partial footprint overlap did not support the actor");
  f.place(0.375f, 0.0f, 0.3f);
  f.move();
  ::require(!f.actor.isGrounded(), "Touching an edge alone invented support");
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.0f, "Edge-only contact blocked descent");
  f.place(-1.0f, 0.0f, 0.0f);
  f.move(1.0f, 0.0f, 0.5f);
  ::require(::near(f.actor.getTransform()->position->x, 0.0f) && f.actor.getElevation() == 0.5f,
            "Rising did not precede ground traversal");
  f.move(0.0f, 0.0f, -0.5f);
  ::require(f.actor.getElevation() == 0.3f && f.actor.isGrounded(), "Staged descent did not land");
  f.move(1.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.0f && f.actor.isGrounded(),
            "Leaving support and descending used the old ground footprint");
}

void test_flags_filters_and_highest_surface() {
  Fixture f;
  f.place(0.0f, 0.0f, 0.3f);
  f.box.setSupportingSurface(false);
  f.move();
  ::require(!f.actor.isGrounded(), "Non-supporting top supplied grounded state");
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.3f && !f.actor.isGrounded(),
            "Solid non-supporting geometry was penetrated");
  f.box.setSupportingSurface(true);
  f.box.getCollider()->setSensor(true);
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.0f, "Sensor supplied support");
  f.box.getCollider()->setSensor(false);
  f.box.getCollider()->setMaskBits(0);
  f.place(0.0f, 0.0f, 0.8f);
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.0f, "Masked collider supplied support");
  f.box.getCollider()->setMaskBits(~0u);
  f.box.getCollider()->setEnabled(false);
  f.place(0.0f, 0.0f, 0.8f);
  f.move(0.0f, 0.0f, -1.0f);
  ::require(f.actor.getElevation() == 0.0f, "Disabled collider supplied support");
  f.box.getCollider()->setEnabled(true);
  TestObject higher(0.6f);
  higher.setSupportingSurface(true);
  higher.setCollider(f.boxShape);
  f.world.add(*higher.getCollider());
  f.place(0.0f, 0.0f, 1.0f);
  f.move(0.0f, 0.0f, -10.0f);
  ::require(f.actor.getElevation() == 0.6f && f.actor.isGrounded(), "Did not land on highest crossed top");
  f.world.remove(*higher.getCollider());
}

void test_height_intervals_legacy_and_recovery() {
  Fixture f(0.125f, 0.25f);
  f.box.setElevation(0.5f);
  f.place(-1.0f, 0.0f, 0.25f);
  ::require(f.move(1.0f).status == MoveStatus::COMPLETED, "Body below raised box could not pass");
  f.place(-1.0f, 0.0f, 0.4f);
  ::require(f.move(1.0f).status == MoveStatus::BLOCKED, "Overlapping body heights did not block");
  f.place(0.0f, 0.0f, 0.25f);
  const auto ceiling = f.move(0.0f, 0.0f, 1.0f);
  ::require(ceiling.status == MoveStatus::BLOCKED && f.actor.getElevation() == 0.375f &&
                !f.actor.isGrounded(), "Ascent penetrated the underside of a raised solid");
  f.place(0.0f, 0.0f, 0.9f);
  f.move(0.0f, 0.0f, -2.0f);
  ::require(f.actor.getElevation() == 0.75f && f.actor.isGrounded(), "Raised base elevation was ignored");
  ::require(f.mover.recoverOverlap(*f.actor.getCollider()).status == RecoveryStatus::ALREADY_SEPARATED,
            "Recovery treated top support as penetration");
  f.world.remove(*f.box.getCollider());
  TestObject wall;
  wall.setCollider(f.boxShape);
  f.world.add(*wall.getCollider());
  f.place(-1.0f, 0.0f, 20.0f);
  ::require(f.move(1.0f).status == MoveStatus::BLOCKED, "Legacy zero-height wall lost blocking");
  f.world.remove(*wall.getCollider());

  Fixture point_body(0.0f, 0.25f);
  point_body.box.setElevation(0.5f);
  point_body.place(0.0f, 0.0f, 0.25f);
  const auto point_ceiling = point_body.move(0.0f, 0.0f, 1.0f);
  ::require(point_ceiling.status == MoveStatus::BLOCKED &&
                point_body.actor.getElevation() < 0.5f &&
                ::near(point_body.actor.getElevation(), 0.5f),
            "Height-zero underside contact became initial penetration");
}

void test_polygon_support_and_nearest_filtering() {
  Fixture f;
  f.world.remove(*f.box.getCollider());
  ConvexPolygonCollisionShape triangle({{-0.5f, -0.5f}, {0.5f, -0.5f}, {0.0f, 0.5f}});
  TestObject slope_outline(0.3f);
  slope_outline.setSupportingSurface(true);
  slope_outline.setCollider(triangle);
  f.world.add(*slope_outline.getCollider());
  f.place(0.0f, 0.0f, 1.0f);
  f.move(0.0f, 0.0f, -2.0f);
  ::require(f.actor.getElevation() == 0.3f && f.actor.isGrounded(), "Convex footprint top did not support");
  f.place(0.4f, 0.35f, 1.0f);
  f.move(0.0f, 0.0f, -2.0f);
  ::require(f.actor.getElevation() == 0.0f, "Polygon bounds invented support outside the shape");
  f.world.remove(*slope_outline.getCollider());
  f.world.add(*f.box.getCollider());
  TestObject tall(1.0f);
  tall.getTransform()->setPosition(1.0f, 0.0f, 0.0f);
  tall.setCollider(f.boxShape);
  f.world.add(*tall.getCollider());
  f.place(-1.0f, 0.0f, 0.4f);
  const auto result = f.move(3.0f);
  ::require(result.status == MoveStatus::BLOCKED && !result.contacts.empty() &&
                result.contacts.front().collider == tall.getCollider(),
            "Ignoring a low box hid a farther tall obstacle");
  f.world.remove(*tall.getCollider());
}

void test_invalid_request() {
  Fixture f;
  bool rejected = false;

  try {
    f.move(1.0f, 0.0f, std::numeric_limits<float>::quiet_NaN());
  } catch (const std::invalid_argument &) {
    rejected = true;
  }

  ::require(rejected && f.actor.getTransform()->position->x == -1.0f &&
                f.actor.getElevation() == 0.0f, "Invalid request partially mutated the body");
  f.actor.setElevation(std::numeric_limits<float>::max());
  rejected = false;

  try {
    f.move(1.0f, 0.0f, std::numeric_limits<float>::max());
  } catch (const std::invalid_argument &) {
    rejected = true;
  }

  ::require(rejected && f.actor.getTransform()->position->x == -1.0f &&
                f.actor.getElevation() == std::numeric_limits<float>::max(),
            "Overflowing elevation request partially mutated the body");
}

}

int main() {
  try {
    ::test_sides_clearance_and_landing();
    ::test_edges_and_staged_movement();
    ::test_flags_filters_and_highest_surface();
    ::test_height_intervals_legacy_and_recovery();
    ::test_polygon_support_and_nearest_filtering();
    ::test_invalid_request();
    std::cout << "Elevated movement tests passed: blocking, clearance, landing, edges, support loss, filters, heights, polygons and legacy walls.\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
