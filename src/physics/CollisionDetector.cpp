#include <vector>
#include <algorithm>

#include <glade/debug/log.h>
#include <glade/physics/CollisionDetector.h>
#include <glade/physics/PhysicalObject.h>
#include <glade/generation/Grid.h>
#include <glade/GladeObject.h>
#include <glade/Context.h>

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

  ccd_real_t depth = 0;
  ccd_vec3_t dir, pos;
  ccdVec3Set(&dir, 0, 0, 0);
  ccdVec3Set(&pos, 0, 0, 0);

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

  Glade::Vector3i centerCellCoord = grid->getCellForPoint(*kinematicObj->getTransform()->position);

  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      for (int k = -1; k <= 1; k++) {
        Glade::Vector3i cellCoord(centerCellCoord.x + i, centerCellCoord.y + j, centerCellCoord.z + k);
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

  // Resolve
  if (prevPosition == nullptr) {
    if (!collisions.empty())
      log("Warning: collisions unresolved in the first frame");

    prevPosition = new Glade::Vector3f(*kinematicObj->getTransform()->position);
    return;
  }

  if (collisions.empty()) {
    prevPosition->set(*kinematicObj->getTransform()->position);
    return;
  }

  float maxDepth = 0;
  unsigned maxi = 0;

  for (unsigned i = 0; i < collisions.size(); ++i) {
    if (collisions[i].depth > maxDepth) {
      maxDepth = collisions[i].depth;
      maxi = i;
    }
  }

  Glade::Vector3f separation(ccdVec3X(&collisions[maxi].dir), ccdVec3Y(&collisions[maxi].dir), ccdVec3Z(&collisions[maxi].dir));
  separation.scale(collisions[maxi].depth);

  //log("Penetration depth: %f, Separation dir: %f %f %f", depth, ccdVec3X(&dir), ccdVec3Y(&dir), ccdVec3Z(&dir));
  Glade::Vector3f separationDir(separation);
  separationDir.normalize();

  Glade::Vector3f toPrevPosition = *prevPosition;
  toPrevPosition.subtract(*kinematicObj->getTransform()->position);
  Glade::Vector3f stickyPushVector(toPrevPosition);
  toPrevPosition.normalize();

  float dot = toPrevPosition.dot(separationDir);

//  if (dot > 0.71) { // stick/slide threshold is about 45 degrees. But sticking should be only for the vertical part,
//  // so the player does not slide down the slopes by gravity
    // stick
//    stickyPushVector.scale(1.0);
//    kinematicObj->getTransform()->position->set(*prevPosition);
//  } else {
    // slide
    kinematicObj->getTransform()->position->add(separation);
//  }

  prevPosition->set(*kinematicObj->getTransform()->position);

  context->eventBus.postEvent(Glade::EventType::GLADE_COLLISION_EVENT, kinematicObj);
}

void CollisionDetector::add(GladeObject* object)
{
  if (object->getPhysicalObject() == nullptr) {
    return;
  }

  if (object->getPhysicalObject()->getType() == PhysicalObject::STATIC) {
    staticCollidableObjects.push_back(object);
  } else {
    kinematicObj = object;
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
