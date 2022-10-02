#include <vector>
#include <algorithm>

#include <glade/debug/log.h>
#include <glade/physics/CollisionDetector.h>
#include <glade/physics/PhysicalObject.h>
#include <glade/generation/Grid.h>
#include <glade/GladeObject.h>

#include <ccd/ccd.h>
#include <ccd/quat.h>

struct CollisionInfo {
  ccd_vec3_t dir;
  ccd_real_t depth;
};

void CollisionDetector::clear()
{
  staticCollidableObjects.clear();
  kinematicObj = nullptr;
  paused = false;
}

// only this one is supported for now
void CollisionDetector::testVsIsosurfaceCellNarrowPhase(GladeObject *kinematicObject, Grid::Cell *staticIsosurfaceCell, std::vector<CollisionInfo> &collisions)
{
  ccd_t ccd;
  CCD_INIT(&ccd);

  ccd.support1       = (ccd_support_fn) staticIsosurfaceCell->getChunkEntity()->getPhysicalObject()->getGjkSupportFunction();
  ccd.support2       = (ccd_support_fn) kinematicObject->getPhysicalObject()->getGjkSupportFunction();
  ccd.max_iterations = 100;
  ccd.epa_tolerance  = 0.0001;

  ccd_real_t depth;
  ccd_vec3_t dir, pos;

  int intersect;

  intersect = ccdGJKPenetration(staticIsosurfaceCell, kinematicObject->getPhysicalObject(), &ccd, &depth, &dir, &pos);
  if (intersect >= 0) {
    CollisionInfo collision;
    collision.depth = depth;
    collision.dir = dir;
    collisions.push_back(collision);
  }
}

void CollisionDetector::detectAndResolveCollisions(long deltaTime)
{
  if (kinematicObj == nullptr || grid == nullptr)
    return;

  // Broad phase (only kinematic shape (sphere or other simple) vs staticIsosurfaceCell is supported now)
  std::vector<Grid::Cell*> staticObjects;

  for (int i = 0; i < grid->chunkSizeCells; i++) {
    for (int j = 0; j < 40; j++) {
      for (int k = 0; k < grid->chunkSizeCells; k++) {
        Glade::Vector3i cellCoord(i, j, k);
        auto celli = grid->cells.find(cellCoord);

        if (celli == grid->cells.end()) {
          //log("No cell here");
          continue;
        }

        Grid::Cell &cell = celli->second;

        if (cell.numVertices <= 0) {
          //log("No vertices in this cell");
          continue;
        }

        staticObjects.push_back(&cell);
      }
    }
  }

  // Narrow phase
  std::vector<CollisionInfo> collisions;

  for (Grid::Cell* cell: staticObjects) {
    testVsIsosurfaceCellNarrowPhase(kinematicObj, cell, collisions);
  }

  log("Found collisions: %d", collisions.size());

  // Resolve
  if (prevPosition == nullptr) {
    if (collisions.size() > 0)
      log("Warning: collisions unresolved in the first frame");

    prevPosition = new Glade::Vector3f(*kinematicObj->getTransform()->position);
    return;
  }

  Glade::Vector3f separation;

  for (const CollisionInfo& collision: collisions) {
    Glade::Vector3f partialSeparation(ccdVec3X(&collision.dir), ccdVec3Y(&collision.dir), ccdVec3Z(&collision.dir));
    partialSeparation.scale(collision.depth);
    separation.add(partialSeparation);
  }

  //log("Penetration depth: %f, Separation dir: %f %f %f", depth, ccdVec3X(&dir), ccdVec3Y(&dir), ccdVec3Z(&dir));
  Glade::Vector3f separationDir(separation);
  separationDir.normalize();

  Glade::Vector3f toPrevPosition = *prevPosition;
  toPrevPosition.subtract(*kinematicObj->getTransform()->position);
  toPrevPosition.normalize();

  float dot = toPrevPosition.dot(separationDir);

  if (dot > 0.71) { // stick/slide threshold is about 45 degrees
    // stick
    log("Resolving (stick)");
    float pushDistance = separation.magnitude() / dot;
    Glade::Vector3f pushVector(toPrevPosition);
    pushVector.scale(pushDistance);
    kinematicObj->getTransform()->position->add(pushVector);
  } else {
    // slide
    log("Resolving (slide)");
    kinematicObj->getTransform()->position->add(separation);
  }

  prevPosition->set(*kinematicObj->getTransform()->position);
}

void CollisionDetector::add(GladeObject* object)
{
  if (object->getPhysicalObject() == nullptr) {
    return;
  }

  if (object->getPhysicalObject()->getType() == PhysicalObject::STATIC) {
    staticCollidableObjects.push_back(object);
    log("Adding static object");
  } else {
    kinematicObj = object;
    log("Adding kinematic object");
  }
}

void CollisionDetector::remove(GladeObject* object)
{
  if (object == kinematicObj) {
    kinematicObj = nullptr;
    return;
  }

  auto oi = std::find(staticCollidableObjects.begin(), staticCollidableObjects.end(), object);
  if (oi != staticCollidableObjects.end()) {
    staticCollidableObjects.erase(oi);
    return;
  }

  log("Warning: tried to remove physical object that isn't there");
}
