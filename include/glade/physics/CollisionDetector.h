#pragma once

#include <glade/generation/Grid.h>

class GladeObject;
class CollisionEventListener;
class Context;
struct CollisionInfo;

class CollisionDetector {
public:
  CollisionDetector(Context *context_)
      : context(context_), paused(false), grid(nullptr), kinematicObj(nullptr),
        prevPosition(nullptr) {}

  void detectAndResolveCollisions(long delta_time);
  void setSpatialIndex(Grid *grid_) { this->grid = grid_; }
  void add(GladeObject *object);
  void remove(GladeObject *object);
  void clear();
  void setPaused(bool paused_) { this->paused = paused_; }
  bool isPaused() { return paused; }

private:
  void testVsIsosurfaceCellNarrowPhase(GladeObject *kinematic_object,
                                       Grid::Cell *static_isosurface_cell,
                                       std::vector<CollisionInfo> &collisions);

  GladeObject *kinematicObj; // only one for now
  Glade::Vector3f *prevPosition;
  std::vector<GladeObject *> staticCollidableObjects;
  Grid *grid;
  Context *context;
  bool paused;
};
