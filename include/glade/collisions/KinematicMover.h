#pragma once

#include <vector>

#include <glade/collisions/CollisionResults.h>
#include <glade/math/Vector.h>

class Collider;
class CollisionWorld;

enum class MoveStatus {
  COMPLETED,
  BLOCKED,
  UNRESOLVED_OVERLAP,
  ITERATION_LIMIT
};

struct MoveRequest {
  Glade::Vector3f displacement;
  // Physical elevation delta, independent of transform/render Z.
  float elevationDisplacement = 0.0f;
};

struct MoveResult {
  MoveStatus status = MoveStatus::COMPLETED;
  Glade::Vector3f requestedDisplacement;
  Glade::Vector3f actualDisplacement;
  float requestedElevationDisplacement = 0.0f;
  float actualElevationDisplacement = 0.0f;
  // X/Y contact snapshots, not final support state or vertical contacts.
  std::vector<OverlapResult> contacts;
};

enum class RecoveryStatus {
  ALREADY_SEPARATED,
  RECOVERED,
  UNRESOLVED
};

struct RecoveryResult {
  RecoveryStatus status = RecoveryStatus::ALREADY_SEPARATED;
  Glade::Vector3f correction;
  // Penetrating contacts only; touching is not an unresolved overlap.
  std::vector<OverlapResult> remainingOverlaps;
};

struct KinematicMoverSettings {
  float contactTolerance = 0.0001f;
  unsigned int maxMoveIterations = 8;
  unsigned int maxRecoveryIterations = 8;
  // Bounds total distance travelled during one recovery call, not net offset.
  float maxRecoveryDistance = 1.0f;
  // Acute angle between the original X/Y request and a blocking face's tangent.
  // Inclusive [0, 90]; 90 preserves unrestricted collision sliding.
  float maxSlideAngleDegrees = 90.0f;
};

// Synchronous movement. World and registered colliders must remain valid.
// Elevated owners rise first, move in X/Y, then descend at the resolved X/Y.
// This is staged character movement, not a simultaneous 3D swept-volume solver.
// Requires an enabled, non-sensor KINEMATIC collider registered in this world.
class KinematicMover {
public:
  explicit KinematicMover(CollisionWorld &world_, const KinematicMoverSettings &settings_ = {});

  // Never automatically recovers initial penetration. Render Z is ignored.
  // Elevation requests require an ElevatedObject owner. Grounded is refreshed
  // even for zero displacement; the reference floor is currently elevation zero.
  MoveResult move(Collider &collider, const MoveRequest &request);

  // Bounded best-effort recovery; an unresolved result can include partial correction.
  RecoveryResult recoverOverlap(Collider &collider);

private:
  MoveResult moveGround(Collider &collider, const MoveRequest &request);

  void refreshGrounded(Collider &collider);

  CollisionWorld &world;
  KinematicMoverSettings settings;
  double maxSlideNormalRatioSquared;
};
