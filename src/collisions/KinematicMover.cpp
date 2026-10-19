#include <glade/collisions/KinematicMover.h>
#include <glade/collisions/Collider.h>
#include <glade/collisions/CollisionWorld.h>
#include <glade/GladeObject.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

CollisionFilter mover_filter(const Collider &collider) {
  CollisionFilter filter;
  filter.categoryBits = collider.getCategoryBits();
  filter.maskBits = collider.getMaskBits();
  filter.ignoredCollider = &collider;

  return filter;
}

void validate_collider(const Collider &collider) {
  if (collider.getType() != Collider::Type::KINEMATIC || !collider.isEnabled() ||
      !collider.getOwner().isEnabled() || collider.isSensor()) {
    throw std::invalid_argument("Kinematic mover requires an enabled, non-sensor kinematic collider");
  }
}

float penetration(const OverlapResult &overlap) {
  float depth = 0.0f;

  for (const auto &point : overlap.manifold.points) {
    depth = std::max(depth, -point.separation);
  }

  return depth;
}

std::vector<OverlapResult> penetrating_contacts(const CollisionWorld &world,
                                               const Collider &collider,
                                               const Transform &transform,
                                               const CollisionFilter &filter) {
  auto overlaps = world.queryOverlaps(collider.getShape(), transform, filter);
  std::erase_if(overlaps, [](const OverlapResult &overlap) {
    return ::penetration(overlap) == 0.0f;
  });

  return overlaps;
}

std::vector<OverlapResult> penetrating_contacts(const CollisionWorld &world,
                                               const Collider &collider,
                                               const CollisionFilter &filter) {
  Transform transform;
  collider.getWorldTransform(transform);

  return ::penetrating_contacts(world, collider, transform, filter);
}

Glade::Vector3f displaced_position(const Collider &collider, const Glade::Vector3f &step) {
  const auto &position = *collider.getOwner().getTransform()->position;
  const Glade::Vector3f next(position.x + step.x, position.y + step.y, position.z);

  if (!std::isfinite(next.x) || !std::isfinite(next.y)) {
    throw std::invalid_argument("Kinematic movement would exceed finite X/Y coordinates");
  }

  return next;
}

bool advance_if_clear(CollisionWorld &world, Collider &collider, const CollisionFilter &filter,
                      const Glade::Vector3f &step) {
  const auto next = ::displaced_position(collider, step);
  auto &position = *collider.getOwner().getTransform()->position;

  if (next.x == position.x && next.y == position.y) return false;

  // Check the position the owner can actually represent, including its local
  // collider offset. Do not commit a rounded cast endpoint inside geometry.
  Transform candidate;
  collider.getWorldTransform(candidate);
  candidate.position->x = next.x + collider.getTransform()->position->x;
  candidate.position->y = next.y + collider.getTransform()->position->y;

  if (!::penetrating_contacts(world, collider, candidate, filter).empty()) return false;

  position.set(next);
  world.update(collider);

  return true;
}

Glade::Vector3f constrain_movement(const Glade::Vector3f &movement,
                                   const std::vector<Glade::Vector3f> &normals) {
  const double magnitude = std::hypot(double(movement.x), double(movement.y));
  const double roundoff = 64.0 * std::numeric_limits<double>::epsilon() * magnitude;
  const auto feasible = [&](double x, double y) {
    return std::all_of(normals.begin(), normals.end(), [&](const Glade::Vector3f &normal) {
      return x * normal.x + y * normal.y >= -roundoff;
    });
  };

  if (feasible(movement.x, movement.y)) return movement;

  // In 2D the closest feasible displacement is on a constraint boundary or
  // at the origin. Test each boundary against all normals: sequential normal
  // projections can reintroduce movement into an earlier non-orthogonal face.
  Glade::Vector3f best;
  double best_distance = double(movement.x) * movement.x + double(movement.y) * movement.y;

  for (const auto &normal : normals) {
    const double length_squared = double(normal.x) * normal.x + double(normal.y) * normal.y;
    const double into = (double(movement.x) * normal.x + double(movement.y) * normal.y) /
                        length_squared;
    const double x = movement.x - normal.x * into;
    const double y = movement.y - normal.y * into;

    if (!feasible(x, y)) continue;

    const double distance = (x - movement.x) * (x - movement.x) +
                            (y - movement.y) * (y - movement.y);

    if (distance < best_distance) {
      best_distance = distance;
      best.set(float(x), float(y), 0.0f);
    }
  }

  return best;
}

Glade::Vector3f position_delta(const Collider &collider, const Glade::Vector3f &start) {
  const auto &position = *collider.getOwner().getTransform()->position;

  return {position.x - start.x, position.y - start.y, 0.0f};
}

}

KinematicMover::KinematicMover(CollisionWorld &world_, const KinematicMoverSettings &settings_)
    : world(world_), settings(settings_) {
  if (!std::isfinite(settings.contactTolerance) || settings.contactTolerance <= 0.0f ||
      settings.maxMoveIterations == 0 || settings.maxRecoveryIterations == 0 ||
      !std::isfinite(settings.maxRecoveryDistance) || settings.maxRecoveryDistance < 0.0f) {
    throw std::invalid_argument("Invalid kinematic mover settings");
  }
}

MoveResult KinematicMover::move(Collider &collider, const MoveRequest &request) {
  ::validate_collider(collider);

  if (!std::isfinite(request.displacement.x) || !std::isfinite(request.displacement.y)) {
    throw std::invalid_argument("Kinematic X/Y displacement must be finite");
  }

  world.update(collider);
  const auto filter = ::mover_filter(collider);
  const Glade::Vector3f start = *collider.getOwner().getTransform()->position;
  MoveResult result;
  result.requestedDisplacement = request.displacement;
  result.contacts = ::penetrating_contacts(world, collider, filter);

  if (!result.contacts.empty()) {
    result.status = MoveStatus::UNRESOLVED_OVERLAP;
    return result;
  }

  Glade::Vector3f remaining(request.displacement.x, request.displacement.y, 0.0f);
  std::vector<Glade::Vector3f> normals;
  bool blocked = false;

  for (unsigned int iteration = 0; iteration < settings.maxMoveIterations; ++iteration) {
    if (remaining.x == 0.0f && remaining.y == 0.0f) break;

    Transform transform;
    collider.getWorldTransform(transform);
    const auto hit = world.castShape(collider.getShape(), transform, remaining, filter);

    if (hit.startsOverlapping) {
      result.contacts.insert(result.contacts.end(), hit.contacts.begin(), hit.contacts.end());
      result.status = MoveStatus::UNRESOLVED_OVERLAP;
      break;
    }

    if (!hit.hit) {
      result.status = ::advance_if_clear(world, collider, filter, remaining)
                          ? (blocked ? MoveStatus::BLOCKED : MoveStatus::COMPLETED)
                          : MoveStatus::ITERATION_LIMIT;
      remaining.setIdentity();
      break;
    }

    blocked = true;
    result.contacts.insert(result.contacts.end(), hit.contacts.begin(), hit.contacts.end());
    double backoff = 0.0;

    for (const auto &contact : hit.contacts) {
      const auto &normal = contact.manifold.normal;
      const double into = double(remaining.x) * normal.x + double(remaining.y) * normal.y;

      if (into < 0.0) {
        backoff = std::max(backoff, settings.contactTolerance / -into);
      }

      normals.push_back(normal);
    }

    const float fraction = float(std::max(0.0, double(hit.fraction) - backoff));
    const Glade::Vector3f step(remaining.x * fraction, remaining.y * fraction, 0.0f);

    if ((step.x != 0.0f || step.y != 0.0f) &&
        !::advance_if_clear(world, collider, filter, step)) {
      result.status = MoveStatus::ITERATION_LIMIT;
      break;
    }

    Glade::Vector3f constrained(remaining.x * (1.0f - fraction),
                              remaining.y * (1.0f - fraction), 0.0f);

    constrained = ::constrain_movement(constrained, normals);

    result.status = MoveStatus::BLOCKED;

    if (constrained.x == 0.0f && constrained.y == 0.0f) {
      remaining.setIdentity();
      break;
    }

    if (fraction == 0.0f && constrained.x == remaining.x && constrained.y == remaining.y) {
      result.status = MoveStatus::ITERATION_LIMIT;
      break;
    }

    remaining = constrained;

    if (iteration + 1 == settings.maxMoveIterations) {
      result.status = MoveStatus::ITERATION_LIMIT;
    }
  }

  result.actualDisplacement = ::position_delta(collider, start);

  return result;
}

RecoveryResult KinematicMover::recoverOverlap(Collider &collider) {
  ::validate_collider(collider);
  world.update(collider);
  const auto filter = ::mover_filter(collider);
  const Glade::Vector3f start = *collider.getOwner().getTransform()->position;
  RecoveryResult result;
  result.remainingOverlaps = ::penetrating_contacts(world, collider, filter);

  if (result.remainingOverlaps.empty()) return result;

  result.status = RecoveryStatus::UNRESOLVED;
  double distance_left = settings.maxRecoveryDistance;

  for (unsigned int iteration = 0;
       iteration < settings.maxRecoveryIterations && distance_left > 0.0; ++iteration) {
    const auto deepest = std::max_element(
        result.remainingOverlaps.begin(), result.remainingOverlaps.end(),
        [](const OverlapResult &first, const OverlapResult &second) {
          return ::penetration(first) < ::penetration(second);
        });
    const auto &normal = deepest->manifold.normal;
    const float distance = float(std::min(distance_left,
        double(::penetration(*deepest)) + settings.contactTolerance));
    const auto next = ::displaced_position(collider, {normal.x * distance, normal.y * distance, 0.0f});
    auto &position = *collider.getOwner().getTransform()->position;
    const double travelled = std::hypot(double(next.x) - position.x, double(next.y) - position.y);

    if (travelled == 0.0 || travelled > distance_left) break;

    // Recovery may still be penetrating after a partial correction. Re-query
    // every time, and report the actual remaining overlaps on failure.
    position.set(next);
    world.update(collider);
    distance_left -= travelled;
    result.remainingOverlaps = ::penetrating_contacts(world, collider, filter);

    if (result.remainingOverlaps.empty()) {
      result.status = RecoveryStatus::RECOVERED;
      break;
    }
  }

  result.correction = ::position_delta(collider, start);

  return result;
}
