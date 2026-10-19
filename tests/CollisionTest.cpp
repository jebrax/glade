#include <glade/GladeObject.h>
#include <glade/collisions/CollisionWorld.h>
#include <glade/collisions/RectangleCollisionShape.h>
#include <glade/collisions/ConvexPolygonCollisionShape.h>

#include <algorithm>
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

void test_polygon_validation_and_bounds() {
  ConvexPolygonCollisionShape clockwise({{0, 0}, {0, 4}, {4, 0}});
  const auto &vertices = clockwise.getVertices();
  const auto &a = vertices[0];
  const auto &b = vertices[1];
  const auto &c = vertices[2];
  ::require((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x) > 0,
            "Clockwise input is normalised to counterclockwise");
  Transform transform;
  transform.setPosition(-10, 20, 100);
  transform.setRotation(0, 0, 2);
  transform.setScale(10, 20, 30);
  const auto bounds = clockwise.computeBounds(transform);
  ::require(bounds.min.x == -10 && bounds.min.y == 20 &&
                bounds.max.x == -6 && bounds.max.y == 24,
            "Polygon bounds translate local vertices and ignore Z/rotation/scale");
  ::rejects([] { ConvexPolygonCollisionShape shape({}); });
  ::rejects([] { ConvexPolygonCollisionShape shape({{0, 0}, {1, 0}}); });
  ::rejects([] { ConvexPolygonCollisionShape shape({{0, 0}, {1, 0}, {2, 0}}); });
  ::rejects([] { ConvexPolygonCollisionShape shape({{0, 0}, {1, 0}, {1, 0}, {0, 1}}); });
  ::rejects([] { ConvexPolygonCollisionShape shape({{0, 0}, {1, 0}, {2, 0}, {0, 1}}); });
  ::rejects([] { ConvexPolygonCollisionShape shape({{0, 0}, {2, 0}, {1, 1}, {2, 2}, {0, 2}}); });
  ::rejects([] { ConvexPolygonCollisionShape shape({{0, 0}, {2, 2}, {0, 2}, {2, 0}}); });
  ::rejects([] {
    ConvexPolygonCollisionShape shape({{0, 0}, {2, 3}, {4, 0}, {0, 2}, {4, 2}});
  });
  ::rejects([] {
    ConvexPolygonCollisionShape shape({{0, 0}, {1, 0},
        {0, std::numeric_limits<float>::quiet_NaN()}});
  });
  transform.setPosition(std::numeric_limits<float>::infinity(), 0, 0);
  ::rejects([&] { clockwise.computeBounds(transform); });
  std::cout << "PASS: convex boundary validation, winding, translated bounds and transform contract\n";
}

void test_rectangle_polygon_intersections() {
  ConvexPolygonCollisionShape triangle({{0, 0}, {4, 0}, {0, 4}});
  RectangleCollisionShape rectangle(0.25f, 0.25f);
  GladeObject terrain(nullptr);
  auto &collider = terrain.setCollider(triangle);
  CollisionWorld world;
  world.add(collider);
  Transform transform;
  // Independent geometric oracle: the rectangle clipped to the triangle's
  // bounding square intersects x+y<=4 iff its lower-left clipped corner does.
  int cases = 0;

  for (int ix = -4; ix <= 20; ++ix) {
    for (int iy = -4; iy <= 20; ++iy) {
      const float x = ix * 0.25f;
      const float y = iy * 0.25f;
      transform.setPosition(x, y, 0);
      const bool expected = x + 0.25f >= 0 && y + 0.25f >= 0 &&
          x - 0.25f <= 4 && y - 0.25f <= 4 &&
          std::max(0.0f, x - 0.25f) + std::max(0.0f, y - 0.25f) <= 4;
      const auto overlaps = world.queryOverlaps(rectangle, transform);
      if ((!overlaps.empty()) != expected) {
        std::cerr << "Grid mismatch x=" << x << " y=" << y << " expected=" << expected << '\n';
        throw std::runtime_error("Rectangle/triangle grid matches geometric oracle");
      }

      if (expected) {
        const auto &manifold = overlaps[0].manifold;
        ::require(::near(manifold.normal.x * manifold.normal.x +
                            manifold.normal.y * manifold.normal.y, 1), "Polygon normal is unit length");
        ::require(!manifold.points.empty() && manifold.points.size() <= 2,
                  "Polygon contact has one or two points");

        for (const auto &point : manifold.points) {
          ::require(point.separation <= 0 && point.position.z == 0,
                    "Signed polygon contact in physical X/Y");
          const float px = point.position.x;
          const float py = point.position.y;
          ::require(px >= -0.00001f && py >= -0.00001f && px + py <= 4.00001f &&
                        (::near(px, 0) || ::near(py, 0) || ::near(px + py, 4)),
                    "Contacts lie on actual triangle boundary");
        }
      }

      ++cases;
    }
  }

  transform.setPosition(2.25f, 2.25f, 0);
  const auto touching = world.queryOverlaps(rectangle, transform);
  ::require(touching.size() == 1 && touching[0].manifold.points[0].separation == 0,
            "Angled face touching included");
  ::require(::near(touching[0].manifold.normal.x, std::sqrt(0.5f)) &&
                ::near(touching[0].manifold.normal.y, std::sqrt(0.5f)),
            "Slanted normal points out of terrain towards rectangle");
  transform.setPosition(2, 2, 0);
  const auto penetrating = world.queryOverlaps(rectangle, transform);
  ::require(::near(penetrating[0].manifold.points[0].separation, -std::sqrt(0.125f)),
            "Angled penetration is perpendicular separation distance");
  transform.setPosition(3.5f, 3.5f, 0);
  ::require(world.queryBounds(rectangle.computeBounds(transform)).size() == 1 &&
                world.queryOverlaps(rectangle, transform).empty(),
            "Overlapping bounds do not imply polygon intersection");
  collider.getTransform()->setPosition(1, 2, 50);
  terrain.getTransform()->setPosition(-10, 20, -100);
  world.update(collider);
  transform.setPosition(-7, 24, 0);
  ::require(world.queryOverlaps(rectangle, transform).size() == 1,
            "Polygon owner/local offsets and updated bounds participate");
  collider.setSensor(true);
  ::require(world.queryOverlaps(rectangle, transform).empty(), "Polygon sensor filtering");
  CollisionFilter filter;
  filter.includeSensors = true;
  ::require(world.queryOverlaps(rectangle, transform, filter).size() == 1,
            "Polygon sensors can be included");
  world.clear();
  std::cout << "PASS: " << cases << " rectangle/triangle intersections, contacts, bounds rejection, offsets, sensors\n";
}

void test_polygon_pairs_and_containment() {
  ConvexPolygonCollisionShape square({{-2, -2}, {2, -2}, {2, 2}, {-2, 2}});
  ConvexPolygonCollisionShape diamond({{0, -0.5f}, {0.5f, 0}, {0, 0.5f}, {-0.5f, 0}});
  RectangleCollisionShape rectangle(0.5f, 0.5f);
  GladeObject terrain(nullptr);
  auto &collider = terrain.setCollider(square);
  CollisionWorld world;
  world.add(collider);
  Transform transform;
  const auto contained = world.queryOverlaps(rectangle, transform);
  ::require(contained.size() == 1 &&
                ::near(contained[0].manifold.points[0].separation, -2.5f),
            "Rectangle containment uses exit distance, not interval intersection width");
  ::require(world.queryOverlaps(diamond, transform).size() == 1,
            "Polygon/polygon containment");
  world.remove(collider);
  terrain.clearCollider();
  auto &rectangle_collider = terrain.setCollider(rectangle);
  world.add(rectangle_collider);
  transform.setPosition(0.75f, 0, 0);
  const auto reverse = world.queryOverlaps(diamond, transform);
  ::require(reverse.size() == 1 && reverse[0].manifold.normal.x > 0,
            "Polygon query against rectangle has outward normal");
  transform.setPosition(3, 0, 0);
  const auto cast = world.castShape(diamond, transform, {-5, 0, 0});
  ::require(cast.hit && ::near(cast.fraction, 0.4f) &&
                cast.contacts[0].manifold.normal.x == 1,
            "Polygon casts against rectangle");
  world.clear();
  terrain.clearCollider();
  auto &polygon_collider = terrain.setCollider(diamond);
  world.add(polygon_collider);
  const auto polygon_cast = world.castShape(diamond, transform, {-5, 0, 0});
  ::require(polygon_cast.hit && ::near(polygon_cast.fraction, 0.4f),
            "Polygon/polygon swept vertex contact");
  world.clear();
  std::cout << "PASS: containment, polygon/rectangle and polygon/polygon overlaps and casts\n";
}

void test_many_vertex_polygon_intersections() {
  const std::vector<Glade::Vector2f> vertices{{-2, -2}, {2, -2}, {3, 0}, {1, 3}, {-2, 2}};
  ConvexPolygonCollisionShape polygon(vertices);
  RectangleCollisionShape rectangle(0.25f, 0.5f);
  GladeObject terrain(nullptr);
  auto &collider = terrain.setCollider(polygon);
  CollisionWorld world;
  world.add(collider);
  Transform transform;
  const auto cross = [](const Glade::Vector2f &a, const Glade::Vector2f &b,
                        const Glade::Vector2f &c) {
    return double(b.x - a.x) * (c.y - a.y) - double(b.y - a.y) * (c.x - a.x);
  };
  const auto inside = [&](const Glade::Vector2f &point) {
    for (std::size_t i = 0; i < vertices.size(); ++i) {
      if (cross(vertices[i], vertices[(i + 1) % vertices.size()], point) < 0) return false;
    }

    return true;
  };
  const auto intersects = [&](const Glade::Vector2f &a, const Glade::Vector2f &b,
                              const Glade::Vector2f &c, const Glade::Vector2f &d) {
    return cross(a, b, c) * cross(a, b, d) <= 0 &&
           cross(c, d, a) * cross(c, d, b) <= 0 &&
           std::max(std::min(a.x, b.x), std::min(c.x, d.x)) <=
               std::min(std::max(a.x, b.x), std::max(c.x, d.x)) &&
           std::max(std::min(a.y, b.y), std::min(c.y, d.y)) <=
               std::min(std::max(a.y, b.y), std::max(c.y, d.y));
  };
  int cases = 0;

  for (int ix = -16; ix <= 16; ++ix) {
    for (int iy = -16; iy <= 16; ++iy) {
      const float x = ix * 0.25f;
      const float y = iy * 0.25f;
      const std::vector<Glade::Vector2f> corners{{x - 0.25f, y - 0.5f},
          {x + 0.25f, y - 0.5f}, {x + 0.25f, y + 0.5f}, {x - 0.25f, y + 0.5f}};
      bool expected = false;

      // Independent oracle using vertex containment and segment intersections.
      for (const auto &corner : corners) expected = expected || inside(corner);

      for (std::size_t i = 0; i < vertices.size(); ++i) {
        const auto &vertex = vertices[i];
        expected = expected || (vertex.x >= x - 0.25f && vertex.x <= x + 0.25f &&
                                vertex.y >= y - 0.5f && vertex.y <= y + 0.5f);

        for (std::size_t j = 0; j < corners.size(); ++j) {
          expected = expected || intersects(vertex, vertices[(i + 1) % vertices.size()],
                                             corners[j], corners[(j + 1) % corners.size()]);
        }
      }

      transform.setPosition(x, y, 0);
      const auto overlaps = world.queryOverlaps(rectangle, transform);
      ::require((!overlaps.empty()) == expected,
                "Rectangle/pentagon matches independent segment-intersection oracle");

      if (!overlaps.empty()) {
        const auto &manifold = overlaps[0].manifold;
        // Applying the reported exit distance plus clearance must separate.
        const float distance = -manifold.points[0].separation + 0.0001f;
        Transform separated;
        separated.setPosition(x + manifold.normal.x * distance,
                              y + manifold.normal.y * distance, 0);
        ::require(world.queryOverlaps(rectangle, separated).empty(),
                  "Polygon manifold normal/depth provides a valid separation");
      }

      ++cases;
    }
  }

  world.clear();
  std::cout << "PASS: " << cases << " rectangle/pentagon intersections and separation vectors\n";
}

void test_thin_polygon_casts_and_symmetry() {
  ConvexPolygonCollisionShape thin({{0, 0}, {0.125f, 0}, {4.125f, 4}, {4, 4}});
  RectangleCollisionShape rectangle(0.25f, 0.25f);
  GladeObject terrain(nullptr);
  auto &collider = terrain.setCollider(thin);
  CollisionWorld world;
  world.add(collider);
  Transform transform;
  transform.setPosition(-10, 2, 0);
  const auto forward = world.castShape(rectangle, transform, {20, 0, 0});
  ::require(forward.hit && ::near(forward.fraction, 0.575f),
            "Fast rectangle cast hits thin slanted quadrilateral");
  transform.setPosition(10, 2, 0);
  const auto reverse = world.castShape(rectangle, transform, {-20, 0, 0});
  ::require(reverse.hit && ::near(reverse.fraction, 0.36875f),
            "Reverse fast cast hits opposite slanted face");
  world.clear();
  terrain.clearCollider();
  auto &rectangle_collider = terrain.setCollider(rectangle);
  terrain.getTransform()->setPosition(10, 2, 0);
  world.add(rectangle_collider);
  transform.setPosition(0, 0, 0);
  const auto swapped = world.castShape(thin, transform, {20, 0, 0});
  ::require(swapped.hit && ::near(swapped.fraction, reverse.fraction) &&
                ::near(swapped.contacts[0].manifold.normal.x,
                       -reverse.contacts[0].manifold.normal.x) &&
                ::near(swapped.contacts[0].manifold.normal.y,
                       -reverse.contacts[0].manifold.normal.y),
            "Swapping query/target and reversing displacement preserves collision time");
  world.clear();
  terrain.clearCollider();
  std::vector<Glade::Vector2f> vertices;

  for (int i = 0; i < 32; ++i) {
    const double angle = i * 2.0 * std::acos(-1.0) / 32;
    vertices.push_back({float(2 * std::cos(angle)), float(2 * std::sin(angle))});
  }

  ConvexPolygonCollisionShape many_sided(vertices);
  auto &many_sided_collider = terrain.setCollider(many_sided);
  terrain.getTransform()->setPosition(0, 0, 0);
  world.add(many_sided_collider);
  transform.setPosition(5, 0, 0);
  const auto many_sided_hit = world.castShape(rectangle, transform, {-10, 0, 0});
  ::require(many_sided_hit.hit && ::near(many_sided_hit.fraction, 0.275f),
            "Polygon queries support more than a small fixed vertex count");
  world.clear();
  terrain.clearCollider();
  std::cout << "PASS: thin angled quadrilateral casts, pair symmetry, 32-vertex polygon\n";
}

void test_polygon_casts() {
  ConvexPolygonCollisionShape triangle({{0, 0}, {4, 0}, {0, 4}});
  RectangleCollisionShape rectangle(0.25f, 0.25f);
  GladeObject terrain(nullptr);
  auto &collider = terrain.setCollider(triangle);
  CollisionWorld world;
  world.add(collider);
  Transform transform;
  transform.setPosition(3, 3, 0);
  const auto hit = world.castShape(rectangle, transform, {-2, -2, 0});
  ::require(hit.hit && !hit.startsOverlapping && ::near(hit.fraction, 0.375f) &&
                hit.contacts.size() == 1, "Sweep hits angled face at analytical time");
  ::require(::near(hit.contacts[0].manifold.points[0].position.x, 2) &&
                ::near(hit.contacts[0].manifold.points[0].position.y, 2) &&
                hit.contacts[0].manifold.points[0].separation == 0,
            "Angled cast contact is on actual terrain surface");
  transform.setPosition(2.25f, 2.25f, 0);
  ::require(world.castShape(rectangle, transform, {-1, -1, 0}).fraction == 0 &&
                world.castShape(rectangle, transform, {-1, -1, 0}).hit,
            "Touching angled face blocks inward movement");
  ::require(!world.castShape(rectangle, transform, {1, 1, 0}).hit &&
                !world.castShape(rectangle, transform, {1, -1, 0}).hit &&
                !world.castShape(rectangle, transform, {0, 0, 0}).hit,
            "Touching permits outward, tangent and stationary requests");
  transform.setPosition(3, 3, 0);
  ::require(world.castShape(rectangle, transform, {-0.75f, -0.75f, 0}).fraction == 1 &&
                world.castShape(rectangle, transform, {-0.75f, -0.75f, 0}).hit,
            "Angled endpoint contact counts");
  transform.setPosition(-1, 0.25f, 0);
  ::require(!world.castShape(rectangle, transform, {1.5f, -1, 0}).hit,
            "Isolated polygon corner graze does not block");
  transform.setPosition(-1, -1, 0);
  const auto corner = world.castShape(rectangle, transform, {1, 1, 0});
  ::require(corner.hit && ::near(corner.fraction, 0.75f) && corner.contacts.size() == 2,
            "Polygon vertex impact retains simultaneous face normals");
  transform.setPosition(1, 1, 0);
  const auto initial = world.castShape(rectangle, transform, {0, 0, 0});
  ::require(initial.hit && initial.startsOverlapping && initial.fraction == 0,
            "Stationary initial polygon penetration reported");
  transform.setPosition(-100, 1, 0);
  ::require(::near(world.castShape(rectangle, transform, {200, 0, 0}).fraction, 0.49875f),
            "Fast cast cannot cross polygon unnoticed");
  collider.setMaskBits(2);
  CollisionFilter filter;
  filter.categoryBits = 1;
  ::require(!world.castShape(rectangle, transform, {200, 0, 0}, filter).hit,
            "Polygon casts respect target masks");
  world.clear();
  std::cout << "PASS: angled sweeps, touching, endpoint, graze, simultaneous normals, initial overlap, speed\n";
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
    ::test_polygon_validation_and_bounds();
    ::test_rectangle_polygon_intersections();
    ::test_polygon_pairs_and_containment();
    ::test_polygon_casts();
    ::test_many_vertex_polygon_intersections();
    ::test_thin_polygon_casts_and_symmetry();
    std::cout << "All collision detection tests passed.\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
