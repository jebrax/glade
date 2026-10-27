# State updates and fixed simulation

State.h defines the trivial default hooks inline. Context.h keeps one-line
methods inline; src/Context.cpp implements scheduling and lifecycle processing.
Context schedules state updates on the rendering thread. Rendering remains the
host application's responsibility and is not called by Context.

## Hooks

- `State::update(Context&)` runs once per normal `processRequests()` call.
  Keep frame-based rules here. This hook was renamed from applyRules;
  callers and overrides must use update. Its per-frame cadence is unchanged.
- `State::fixedUpdate(Context&, float seconds)` defaults to no work. Override it
  for movement, gravity, collision requests and simulation-driven animation.
  It runs zero or more times per normal call, always with 1/60 second.

For example:

```cpp
void MyState::fixedUpdate(Context& context, float seconds) {
  // Use seconds for velocity integration and displacement requests.
  actor.simulate(seconds);
}
```

## Scheduling and lifecycle

Call `processRequests()` once per application frame. It measures elapsed time
with a monotonic clock. The overload `processRequests(double elapsed_seconds)`
lets a host provide its own elapsed interval, including deterministic tests;
choose one overload per frame. Negative supplied intervals contribute zero time;
non-finite supplied intervals throw `std::invalid_argument`.

A stop or state switch is processed first and returns without invoking either
update hook. State switching, including waking a suspended state, and stopping
reset fixed timing. As before, objects queued by state initialization are
registered on the next call. On a normal call, removals/additions are processed,
then update runs, followed by the available fixed updates. Requests made by
update hooks remain queued until the next call; stop, state-change and clear
requests interrupt further catch-up steps for the outgoing scene. Object changes
queued by a hook are not registered between catch-up steps.

Existing queue-lifetime limitation: switching away immediately after init, before
its queued objects have been registered, can leave dangling additions when the
outgoing state owns those objects. A normal registration pass before replacement
avoids that case. Fixed scheduling does not repair this pre-existing ownership
issue; do not treat arbitrary immediate transitions as verified safe.

Context accumulates fractional seconds. It runs at most six fixed steps per frame,
dropping excess whole steps and preserving the fraction. Under sustained overload,
simulation slows rather than building an unbounded backlog. The interval and limit
are exposed as `Context::FIXED_STEP_SECONDS` and `Context::MAX_FIXED_STEPS`.
A 1e-12-second tolerance only compensates rounding at a step boundary.

The existing legacy CollisionDetector and networking processing remain after the
state hooks at their previous per-frame cadence. Existing update-only states
remain valid; this change does not migrate their movement or legacy physics to a
fixed timestep. CollisionWorld has no autonomous tick: fixedUpdate should issue
kinematic movement requests through the existing mover.

## Pauses, input and rendering

Call `Context::resetSimulationTiming()` after waiting during a pause, including
the wait that resumes the application. It resets the clock and fractional backlog
without changing state-owned input. Queue one-shot input until a fixed step
consumes it, so frames without a step do not lose an action and catch-up steps do
not replay it. Define any policy for discarding input on pause in the game.

Rendering uses the latest simulated state. Context does not add interpolation.
Fixed stepping does not replace swept collision checks or prevent tunnelling by
itself.

## Verification

`tests/ContextFixedUpdateTest.cpp` exercises the real Context/State path with
explicit frame intervals, collider registration, frame/fixed hook counts,
catch-up limits, fractional time, timing reset, state replacement/resume,
update-only compatibility, clear and stop
requests. It needs a hidden GLFW window for Context's renderer lifecycle and ENet
initialization. The host must supply the usual Glade dependencies and platform
hooks; `tests/CMakeLists.txt` defines `glade_context_fixed_update_test` when the
host provides `glade` and `glfw` targets. It is not a rendering correctness test.
