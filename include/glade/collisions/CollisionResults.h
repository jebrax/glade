#pragma once

#include <vector>

#include <glade/math/Vector.h>

class Collider;

struct CollisionContactPoint {
  // Physical world X/Y; Z is zero for X/Y queries.
  Glade::Vector3f position;
  // Negative for penetration, zero for touching.
  float separation = 0.0f;
};

struct CollisionManifold {
  // Unit normal from the encountered collider towards the query shape.
  Glade::Vector3f normal;
  std::vector<CollisionContactPoint> points;
};

struct OverlapResult {
  const Collider *collider = nullptr;
  CollisionManifold manifold;
};

struct ShapeCastResult {
  bool hit = false;
  float fraction = 1.0f;
  bool startsOverlapping = false;
  // Earliest contacts only. A corner can produce multiple normals, even for
  // the same collider. Results do not outlive the colliders they reference.
  std::vector<OverlapResult> contacts;
};
