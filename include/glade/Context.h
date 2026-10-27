#pragma once

#include <queue>
#include <cassert>
#include <chrono>

#include "render/GladeRenderer.h"
#include "State.h"
#include "GladeObject.h"
#include "ui/layout/Layout.h"
#include "controls/VirtualController.h"
#include "physics/CollisionDetector.h"
#include "collisions/CollisionWorld.h"
#include "ai/AiContainer.h"
#include "Timer.h"
#include "debug/log.h"
#include "net/NetworkManager.h"
#include "EventBus.h"

class Context {
public:
  static constexpr double FIXED_STEP_SECONDS = 1.0 / 60.0;
  static constexpr int MAX_FIXED_STEPS = 6;

  Timer physicsTimer;
  Glade::Renderer *renderer;

  CollisionDetector *collisionDetector;
  CollisionWorld collisionWorld;
  Glade::EventBus eventBus;
  AiContainer aiContainer;
  Glade::NetworkManager *networkManager;

  bool enableCollisionDetector, enableAiContainer;

private:
  State *currentState, *requestedState;
  bool stopRequested, clearRequested, suspend;
  VirtualController *controller;
  using UpdateClock = std::chrono::steady_clock;
  UpdateClock::time_point lastUpdateTime = UpdateClock::now();
  double fixedAccumulator = 0.0;
  std::queue<std::pair<GladeObject *, bool>> objectsToAdd;
  std::queue<GladeObject *> objectsToRemove;

public:
  Context(Glade::Renderer *renderer_, bool server);

  ~Context();

  State *getCurrentState() { return currentState; }

  void requestStateChange(State *state, bool suspend_ = false);

  void requestStop(void) { stopRequested = true; }
  void add(GladeObject *object, bool replicate = false) { objectsToAdd.emplace(object, replicate); }
  void remove(GladeObject *object) { objectsToRemove.push(object); }
  CollisionDetector *getCollisionDetector(void) { return collisionDetector; }
  CollisionWorld *getCollisionWorld() { return &collisionWorld; }
  AiContainer *getAiContainer(void) { return &aiContainer; }

  // Discard elapsed simulation time after a pause. Does not alter state-owned input.
  void resetSimulationTiming();

  /** Should be called only from a rendering thread. Uses a monotonic clock. */
  void processRequests();

  // Hosts with their own clock can supply elapsed seconds; do not call both overloads per frame.
  void processRequests(double elapsed_seconds);

private:
  void processFrame(double elapsed_seconds);

public:
  Glade::Renderer *getRenderer(void) { return renderer; }
  void clear(void) { clearRequested = true; }
  void setController(VirtualController &controller_) { this->controller = &controller_; }
  VirtualController *getController(void) { return controller; }

private:
  /**
   * Should be called only from rendering thread
   */
  void switchState(void);

  /**
   * Should not be called when containers are iterating
   */
  void removeNow(GladeObject *object);

  /**
   * Should not be called when containers are iterating
   */
  void addNow(std::pair<GladeObject *, bool> &entry);

  /**
   * Should be called only from rendering thread. Unconditionally force clear all resources
   */
  void clearNowFully(void);
};
