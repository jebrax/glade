#pragma once

#include <queue>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <stdexcept>

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
  Context(Glade::Renderer *renderer_, bool server)
      : renderer(renderer_), enableCollisionDetector(true), enableAiContainer(true),
        stopRequested(false), clearRequested(false), suspend(false), currentState(nullptr),
        requestedState(nullptr), collisionDetector(new CollisionDetector(this)), physicsTimer(true),
        controller(nullptr), networkManager(new Glade::NetworkManager(this, server)) {}

  ~Context() {
    delete collisionDetector;
    delete networkManager;
  }

  State *getCurrentState() { return currentState; }

  void requestStateChange(State *state, bool suspend_ = false) {
    this->suspend = suspend_;
    requestedState = state;
  }

  void requestStop(void) { stopRequested = true; }
  void add(GladeObject *object, bool replicate = false) { objectsToAdd.emplace(object, replicate); }
  void remove(GladeObject *object) { objectsToRemove.push(object); }
  CollisionDetector *getCollisionDetector(void) { return collisionDetector; }
  CollisionWorld *getCollisionWorld() { return &collisionWorld; }
  AiContainer *getAiContainer(void) { return &aiContainer; }

  // Discard elapsed simulation time after a pause. Does not alter state-owned input.
  void resetSimulationTiming() {
    lastUpdateTime = UpdateClock::now();
    fixedAccumulator = 0.0;
  }

  /** Should be called only from a rendering thread. Uses a monotonic clock. */
  void processRequests() {
    const auto now = UpdateClock::now();
    const double elapsed_seconds = std::chrono::duration<double>(now - lastUpdateTime).count();
    lastUpdateTime = now;
    processFrame(elapsed_seconds);
  }

  // Hosts with their own clock can supply elapsed seconds; do not call both overloads per frame.
  void processRequests(double elapsed_seconds) {
    if (!std::isfinite(elapsed_seconds))
      throw std::invalid_argument("Context elapsed seconds must be finite");

    lastUpdateTime = UpdateClock::now();
    processFrame(elapsed_seconds);
  }

private:
  void processFrame(double elapsed_seconds) {
    if (stopRequested) {
      ::log("Context stop requested");
      stopRequested = false;
      clearNowFully();

      if (currentState != nullptr) {
        currentState->shutdown(*this);
        delete currentState;
        currentState = nullptr;
      }

      resetSimulationTiming();

      return;
    }

    if (requestedState != nullptr) {
      ::log("State switch requested");
      switchState();
      resetSimulationTiming();

      return;
    }

    if (clearRequested) {
      ::log("Clear requested");
      clearNowFully();
    }

    bool glade_objects_lists_changed = false;

    if (!objectsToRemove.empty()) {
      ::log("Context: some objects will be deleted");
      glade_objects_lists_changed = true;

      while (!objectsToRemove.empty()) {
        removeNow(objectsToRemove.front());
        objectsToRemove.pop();
      }
    }

    if (!objectsToAdd.empty()) {
      ::log("Context: new objects will be loaded");

      while (!objectsToAdd.empty()) {
        addNow(objectsToAdd.front());
        objectsToAdd.pop();
      }

      glade_objects_lists_changed = true;
    }

    if (getCurrentState() != NULL) {
      // Registration and per-frame rules run once, before any catch-up steps.
      getCurrentState()->applyRules(*this);
      fixedAccumulator += std::max(elapsed_seconds, 0.0);
      // Ignore only floating-point rounding at an exact step boundary.
      const double whole_steps = std::floor((fixedAccumulator + 1.0e-12) / FIXED_STEP_SECONDS);
      fixedAccumulator = std::max(0.0, fixedAccumulator - whole_steps * FIXED_STEP_SECONDS);
      const int steps = static_cast<int>(std::min(whole_steps, double(MAX_FIXED_STEPS)));

      // Excess whole steps are discarded. Transitions requested by a hook take effect
      // on the next processRequests call; do not keep simulating the outgoing state.
      for (int step = 0; step < steps; ++step) {
        if (stopRequested || requestedState != nullptr || clearRequested) break;
        getCurrentState()->fixedUpdate(*this, static_cast<float>(FIXED_STEP_SECONDS));
      }

      if (enableCollisionDetector)
        getCollisionDetector()->detectAndResolveCollisions(physicsTimer.getDeltaTime());

      networkManager->process();
    }
  }

public:
  Glade::Renderer *getRenderer(void) { return renderer; }
  void clear(void) { clearRequested = true; }
  void setController(VirtualController &controller_) { this->controller = &controller_; }
  VirtualController *getController(void) { return controller; }

private:
  /**
   * Should be called only from rendering thread
   */
  void switchState(void) {
    clearNowFully();

    if (currentState != nullptr) {
      if (suspend) {
        ::log("Suspending current state");
        currentState->suspend(*this);
        ::log("Current state was suspended");
      } else {
        ::log("Shutting down current state");
        currentState->shutdown(*this);
        delete currentState;
        currentState = nullptr;
        ::log("Current state was shut down");
      }
    }

    if (requestedState != nullptr) {
      currentState = requestedState;
      requestedState = nullptr;

      if (currentState->isSuspended()) {
        ::log("Waking up requested state");
        currentState->wakeup(*this);
      } else {
        ::log("Initializing requested state");
        currentState->init(*this);
      }
    }
  }

  /**
   * Should not be called when containers are iterating
   */
  void removeNow(GladeObject *object) {
    renderer->remove(object);
    collisionDetector->remove(object);

    if (auto *collider = object->getCollider()) {
      collisionWorld.remove(*collider);
    }
  }

  /**
   * Should not be called when containers are iterating
   */
  void addNow(std::pair<GladeObject *, bool> &entry) {
    ::log("Adding now");
    renderer->add(entry.first);
    collisionDetector->add(entry.first);

    if (auto *collider = entry.first->getCollider()) {
      collisionWorld.add(*collider);
    }

    if (entry.second) {
      ::log("With replication");
      networkManager->addObject(entry.first);
    }
  }

  /**
   * Should be called only from rendering thread. Unconditionally force clear all resources
   */
  void clearNowFully(void) {
    ::log("Clearing fully");

    renderer->clear();
    collisionDetector->clear();
    collisionWorld.clear();
    aiContainer.clear();

    clearRequested = false;
  }
};
