#pragma once

#include <queue>
#include <assert.h>

#include "render/GladeRenderer.h"
#include "State.h"
#include "ui/layout/Layout.h"
#include "controls/VirtualController.h"
#include "physics/CollisionDetector.h"
#include "ai/AiContainer.h"
#include "Timer.h"
#include "debug/log.h"
#include "EventBus.h"

class Context {
public:
  Timer physicsTimer;
  Glade::Renderer* renderer;

  CollisionDetector *collisionDetector;
  Glade::EventBus eventBus;
  AiContainer aiContainer;

  bool enableCollisionDetector, enableAiContainer;

private:
  State *currentState, *requestedState;
  bool stopRequested, clearRequested, suspend;
  VirtualController* controller;
  std::queue<GladeObject*> objectsToAdd;
  std::queue<GladeObject*> objectsToRemove;

public:
  Context(Glade::Renderer* renderer):
    renderer(renderer),
    enableCollisionDetector(true),
    enableAiContainer(true),
    stopRequested(false),
    clearRequested(false),
    suspend(false),
    currentState(nullptr),
    requestedState(nullptr),
    collisionDetector(new CollisionDetector(this)),
    physicsTimer(true),
    controller(nullptr)
  {
  }

  ~Context() {
    delete collisionDetector;
  }

  void requestStateChange(State* state, bool suspend = false) {
    this->suspend = suspend;
    requestedState = state;
  }

  void requestStop(void) {
    stopRequested = true;
  }

  void add(GladeObject* object) {
    objectsToAdd.push(object);
  }

  void remove(GladeObject* object) {
    objectsToRemove.push(object);
  }

  State* getCurrentState(void) {
    return currentState;
  }

  CollisionDetector* getCollisionDetector(void) {
    return collisionDetector;
  }

  AiContainer* getAiContainer(void) {
    return &aiContainer;
  }

  /**
   * Should be called only from a rendering thread
   */
  void processRequests(void) {
    if (stopRequested) {
      log("Context stop requested");
      stopRequested = false;
      clearNowFully();

      if (currentState != nullptr) {
        currentState->shutdown(*this);
        delete currentState;
        currentState = nullptr;
      }

      return;
    }

    if (requestedState != nullptr) {
      log("State switch requested");
      switchState();

      return;
    }

    if (clearRequested) {
      log("Clear requested");
      clearNowFully();
    }

    bool gladeObjectsListsChanged = false;
  
    if (!objectsToRemove.empty()) {
      log("Context: some objects will be deleted");
      gladeObjectsListsChanged = true;
      
      while (!objectsToRemove.empty()) {
        removeNow(objectsToRemove.front());
        objectsToRemove.pop();
      }
    }
    
    if (!objectsToAdd.empty()) {
      log("Context: new objects will be loaded");
     
      while (!objectsToAdd.empty()) {
        addNow(objectsToAdd.front());
        objectsToAdd.pop();
      }
      
      gladeObjectsListsChanged = true;
    }

    if (getCurrentState() != NULL) {
      getCurrentState()->applyRules(*this);

      if (enableCollisionDetector)
        getCollisionDetector()->detectAndResolveCollisions(physicsTimer.getDeltaTime());
    }
  }

  Glade::Renderer* getRenderer(void) {
    return renderer;
  }

  void clear(void) {
    clearRequested = true;
  }

  void setController(VirtualController &controller) {
    controller.initController();
    this->controller = &controller;
  }

  VirtualController* getController(void) {
    return controller;
  }

private:
  /**
   * Should be called only from rendering thread
   */
  void switchState(void) {  
    clearNowFully();

    if (currentState != nullptr) {
      if (suspend) {
        log("Suspending current state");
        currentState->suspend(*this);
        log("Current state was suspended");
      } else {
        log("Shutting down current state");
        currentState->shutdown(*this);
        delete currentState;
        currentState = nullptr;
        log("Current state was shut down");
      }
    }

    if (requestedState != nullptr) {
      currentState = requestedState;
      requestedState = nullptr;

      if (currentState->isSuspended()) {
        log("Waking up requested state");
        currentState->wakeup(*this);
      } else {
        log("Initializing requested state");
        currentState->init(*this);
      }
    }
  }

  /**
   * Should not be called when containers are iterating
   */
  void removeNow(GladeObject* object) {
    renderer->remove(object);
    collisionDetector->remove(object);
    //aiContainer->remove(object);
  }
  
  /**
   * Should not be called when containers are iterating
   */
  void addNow(GladeObject* object) {
    //log("Adding now");
    renderer->add(object);
    collisionDetector->add(object);
    //aiContainer->add(object);
  }

  /**
   * Should be called only from rendering thread. Unconditionally force clear all resources
   */
  void clearNowFully(void) {
    log("Clearing fully");

    renderer->clear();
    collisionDetector->clear();
    aiContainer.clear();

    clearRequested = false;
  }
};
