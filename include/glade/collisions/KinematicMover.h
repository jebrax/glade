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
};

struct MoveResult {
  MoveStatus status = MoveStatus::COMPLETED;
  Glade::Vector3f requestedDisplacement;
  Glade::Vector3f actualDisplacement;
  // Snapshots of contacts encountered along the path, not final support state.
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
};

// Synchronous X/Y movement. World and registered colliders must remain valid.
// Requires an enabled, non-sensor KINEMATIC collider registered in this world.
class KinematicMover {
public:
  explicit KinematicMover(CollisionWorld &world_, const KinematicMoverSettings &settings_ = {});

  // Never automatically recovers initial penetration. Z displacement is ignored.
  MoveResult move(Collider &collider, const MoveRequest &request);

  // Bounded best-effort recovery; an unresolved result can include partial correction.
  RecoveryResult recoverOverlap(Collider &collider);

private:
  CollisionWorld &world;
  KinematicMoverSettings settings;
};
