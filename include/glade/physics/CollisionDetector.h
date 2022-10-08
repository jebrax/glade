#pragma once

#include <glade/generation/Grid.h>

class GladeObject;
class CollisionEventListener;
class Context;
struct CollisionInfo;

class CollisionDetector
{
  public:
    CollisionDetector(Context *context): context(context), paused(false), grid(nullptr), kinematicObj(nullptr), prevPosition(nullptr) {}

    void detectAndResolveCollisions(long deltaTime);
    void setSpatialIndex(Grid *grid) { this->grid = grid; }

    void add(GladeObject* object);
    void remove(GladeObject* object);
    void clear();

    void setPaused(bool paused) { this->paused = paused; }
    bool isPaused() { return paused; }

  private:
    void testVsIsosurfaceCellNarrowPhase(GladeObject *kinematicObject, Grid::Cell *staticIsosurfaceCell, std::vector<CollisionInfo> &collisions);

    GladeObject* kinematicObj; // only one for now
    Glade::Vector3f *prevPosition;
    std::vector<GladeObject*> staticCollidableObjects;
    Grid *grid;
    Context *context;
    bool paused;
};

