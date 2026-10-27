#include <glade/Context.h>

#include <algorithm>
#include <cmath>
#include <stdexcept>

Context::Context(Glade::Renderer *renderer_, bool server)
    : renderer(renderer_), enableCollisionDetector(true), enableAiContainer(true),
      stopRequested(false), clearRequested(false), suspend(false), currentState(nullptr),
      requestedState(nullptr), collisionDetector(new CollisionDetector(this)), physicsTimer(true),
      controller(nullptr), networkManager(new Glade::NetworkManager(this, server)) {}

Context::~Context() {
  delete collisionDetector;
  delete networkManager;
}

void Context::requestStateChange(State *state, bool suspend_) {
  this->suspend = suspend_;
  requestedState = state;
}

void Context::resetSimulationTiming() {
  lastUpdateTime = UpdateClock::now();
  fixedAccumulator = 0.0;
}

void Context::processRequests() {
  const auto now = UpdateClock::now();
  const double elapsed_seconds = std::chrono::duration<double>(now - lastUpdateTime).count();
  lastUpdateTime = now;
  processFrame(elapsed_seconds);
}

void Context::processRequests(double elapsed_seconds) {
  if (!std::isfinite(elapsed_seconds))
    throw std::invalid_argument("Context elapsed seconds must be finite");

  lastUpdateTime = UpdateClock::now();
  processFrame(elapsed_seconds);
}

void Context::processFrame(double elapsed_seconds) {
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
    getCurrentState()->update(*this);
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

void Context::switchState(void) {
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

void Context::removeNow(GladeObject *object) {
  renderer->remove(object);
  collisionDetector->remove(object);

  if (auto *collider = object->getCollider()) {
    collisionWorld.remove(*collider);
  }
}

void Context::addNow(std::pair<GladeObject *, bool> &entry) {
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

void Context::clearNowFully(void) {
  ::log("Clearing fully");

  renderer->clear();
  collisionDetector->clear();
  collisionWorld.clear();
  aiContainer.clear();

  clearRequested = false;
}
