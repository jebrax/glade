#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glade/Context.h>
#include <glade/collisions/RectangleCollisionShape.h>
#include <enet/enet.h>
#include <glade/GladeObject.h>
#include <glade/system.h>
#include <glade/util/DesktopFileManager.h>
#include <glade/util/ResourceManager.h>
#include <glade/exception/GladeException.h>

#include <stdexcept>
#include <cstdio>
#include <exception>

// Legacy Glade font code expects these application-owned resource pointers.
FileManager *file_manager = nullptr;
Glade::ResourceManager *resource_manager = nullptr;

namespace {

GLFWwindow *window = nullptr;

void report_glfw_error(int code, const char *description) {
  std::fprintf(::stderr, "GLFW error %d: %s\n", code, description);
}

} // namespace

namespace Glade::System {

void get_viewport_size(unsigned int *width, unsigned int *height) {
  int w = 0, h = 0;
  if (::window) ::glfwGetFramebufferSize(::window, &w, &h);
  *width = static_cast<unsigned int>(w);
  *height = static_cast<unsigned int>(h);
}

void toggle_mouse_cursor(bool enabled) {
  ::glfwSetInputMode(::window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void set_mouse_cursor_position(double x, double y) { ::glfwSetCursorPos(::window, x, y); }
void get_mouse_cursor_position(double &x, double &y) { ::glfwGetCursorPos(::window, &x, &y); }

} // namespace Glade::System

#include <cmath>
#include <limits>
#include <initializer_list>

namespace {

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}

struct Counts {
  int frames = 0;
  int steps = 0;
  int init = 0;
  int shutdown = 0;
  int suspends = 0;
  int wakeups = 0;
  bool stopOnFrame = false;
  bool stopOnStep = false;
  bool clearOnStep = false;
  State *nextState = nullptr;
};

class ProbeState final : public State {
public:
  explicit ProbeState(Counts &counts_) : counts(counts_), object(nullptr) {
    object.setCollider(shape);
  }

  ~ProbeState() override {
    object.clearCollider();
    GladeObject::indexById.erase(object.getId());
  }

  void init(Context &context) override {
    ++counts.init;
    context.add(&object);
  }

  void shutdown(Context &) override { ++counts.shutdown; }

  void suspend(Context &context) override {
    State::suspend(context);
    ++counts.suspends;
  }

  void wakeup(Context &context) override {
    State::wakeup(context);
    ++counts.wakeups;
    context.add(&object);
  }

  void update(Context &context) override {
    ++counts.frames;
    if (counts.stopOnFrame) context.requestStop();
  }

  void fixedUpdate(Context &context, float seconds) override {
    ::require(std::abs(seconds - 1.0f / 60.0f) < 1.0e-8f, "Unexpected fixed interval");
    ::require(counts.frames > 0, "Fixed update ran before per-frame rules");
    const auto registered = context.collisionWorld.queryBounds({{-1.0f, -1.0f}, {1.0f, 1.0f}});
    ::require(registered.size() == 1 && registered.front() == object.getCollider(),
              "Fixed update ran before object registration");
    ++counts.steps;
    if (counts.stopOnStep) context.requestStop();
    if (counts.clearOnStep) context.clear();
    if (counts.nextState) context.requestStateChange(counts.nextState);
  }

private:
  Counts &counts;
  RectangleCollisionShape shape{0.1f, 0.1f};
  GladeObject object;
};

class Fixture {
public:
  Counts counts;
  Glade::Renderer renderer;
  Context context{&renderer, false};

  Fixture() {
    context.enableCollisionDetector = false;
    context.enableAiContainer = false;
    context.requestStateChange(new ProbeState(counts));
    context.processRequests(10.0); // Switching states must not simulate this interval.
    ::require(counts.init == 1 && counts.frames == 0 && counts.steps == 0,
              "State switch simulated elapsed time");
  }

  ~Fixture() {
    context.requestStop();
    context.processRequests(0.0);
  }
};

void check_cadence(int render_hz) {
  Fixture fixture;

  for (int frame = 0; frame < render_hz * 10; ++frame) {
    fixture.context.processRequests(1.0 / render_hz);
  }

  ::require(fixture.counts.frames == render_hz * 10, "Per-frame hook repeated during catch-up");
  ::require(fixture.counts.steps == 600, "Render rate changed the simulation duration");
  std::printf("%d Hz: %d frame updates, %d fixed updates in 10 seconds\n",
              render_hz, fixture.counts.frames, fixture.counts.steps);
}

void check_timing() {
  Fixture f;
  constexpr double STEP = Context::FIXED_STEP_SECONDS;
  f.context.processRequests(STEP * 0.5);
  ::require(f.counts.frames == 1 && f.counts.steps == 0, "Partial step simulated too soon");
  f.context.processRequests(STEP * 0.5);
  ::require(f.counts.steps == 1, "Fractional time was lost");
  f.context.processRequests(STEP * 600.5);
  ::require(f.counts.steps == 7, "Catch-up exceeded six steps");
  f.context.processRequests(0.0);
  ::require(f.counts.steps == 7, "Discarded backlog was replayed");
  f.context.processRequests(STEP * 0.5);
  ::require(f.counts.steps == 8, "Overload lost the fractional remainder");
  f.context.processRequests(STEP * 0.5);
  f.context.resetSimulationTiming();
  f.context.processRequests(STEP * 0.5);
  ::require(f.counts.steps == 8, "Reset retained accumulated time");
  f.context.processRequests(-1.0);
  f.context.processRequests(STEP * 0.5);
  ::require(f.counts.steps == 9, "Negative interval corrupted scheduling");
  bool rejected = false;

  try {
    f.context.processRequests(std::numeric_limits<double>::infinity());
  } catch (const std::invalid_argument &) {
    rejected = true;
  }

  ::require(rejected, "Non-finite elapsed time was accepted");
  f.context.processRequests(STEP);
  ::require(f.counts.steps == 10, "Invalid elapsed time corrupted scheduling");
}

void check_lifecycle() {
  Counts replacement_counts;
  Fixture f;
  constexpr double STEP = Context::FIXED_STEP_SECONDS;
  auto *replacement = new ProbeState(replacement_counts);
  f.counts.nextState = replacement;
  f.context.processRequests(STEP * 6.5);
  ::require(f.counts.steps == 1, "Outgoing state kept running catch-up steps");
  f.context.processRequests(100.0);
  ::require(f.counts.shutdown == 1 && replacement_counts.init == 1 && replacement_counts.steps == 0,
            "Replacement state did not start with fresh timing");
  f.context.processRequests(STEP * 0.5);
  ::require(replacement_counts.steps == 0, "State transition retained fractional time");
  replacement_counts.clearOnStep = true;
  f.context.processRequests(STEP * 6.0);
  ::require(replacement_counts.steps == 1, "Clear request did not interrupt catch-up");
  // Stop takes precedence over clear; the registered object is removed before destruction.
  f.context.requestStop();
  f.context.processRequests(100.0);
  ::require(replacement_counts.shutdown == 1 && f.context.getCurrentState() == nullptr,
            "Stop did not shut down the state");
  f.context.processRequests(100.0);
}

void check_resume() {
  Counts other_counts;
  Fixture f;
  auto *original = f.context.getCurrentState();
  f.context.processRequests(Context::FIXED_STEP_SECONDS * 0.5);
  f.context.requestStateChange(new ProbeState(other_counts), true);
  f.context.processRequests(100.0);
  ::require(original->isSuspended() && f.counts.suspends == 1 && f.counts.shutdown == 0,
            "Switch did not suspend the original state");
  f.context.processRequests(Context::FIXED_STEP_SECONDS * 0.5);
  f.context.requestStateChange(original);
  f.context.processRequests(100.0);
  ::require(!original->isSuspended() && f.counts.wakeups == 1 && other_counts.shutdown == 1,
            "Suspended state did not resume");
  f.context.processRequests(Context::FIXED_STEP_SECONDS * 0.5);
  ::require(f.counts.steps == 0, "Resume retained time from another state");
  f.context.processRequests(Context::FIXED_STEP_SECONDS * 0.5);
  ::require(f.counts.steps == 1, "Resumed state did not receive fixed updates");
}

class FrameOnlyState final : public State {
public:
  int frames = 0;
  void update(Context &) override { ++frames; }
};

void check_frame_only_state() {
  Fixture f;
  // Complete the initial registration pass before replacing the owning state.
  f.context.processRequests(0.0);
  auto *legacy = new FrameOnlyState;
  f.context.requestStateChange(legacy);
  f.context.processRequests(0.0);
  f.context.processRequests(1.0);
  f.context.processRequests(0.0);
  ::require(legacy->frames == 2, "Frame-only hook changed cadence");
}

void check_stop_requests() {
  Fixture f;
  f.counts.stopOnFrame = true;
  f.context.processRequests(1.0);
  ::require(f.counts.frames == 1 && f.counts.steps == 0,
            "Fixed update ran after a per-frame stop request");
  f.context.processRequests(0.0);
  ::require(f.counts.shutdown == 1, "Per-frame stop was not processed");

  f.counts.stopOnFrame = false;
  f.counts.stopOnStep = true;
  f.context.requestStateChange(new ProbeState(f.counts));
  f.context.processRequests(0.0);
  f.context.processRequests(1.0);
  ::require(f.counts.steps == 1, "Stop request did not interrupt catch-up");
}

} // namespace

int main() {
  std::setvbuf(::stdout, nullptr, _IOLBF, 0);
  ::glfwSetErrorCallback(::report_glfw_error);
  if (!::glfwInit()) return 1;
  ::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  ::window = ::glfwCreateWindow(64, 64, "Context fixed update test", nullptr, nullptr);
  if (!::window) return 1;
  ::glfwMakeContextCurrent(::window);
  ::glewExperimental = GL_TRUE;
  if (::glewInit() != GLEW_OK) return 1;
  // GLEW context probing only.
  while (::glGetError() != GL_NO_ERROR) {}
  if (::enet_initialize() != 0) return 1;
  int result = 0;

  try {
    for (const int rate : {20, 30, 60, 120, 144}) {
      ::check_cadence(rate);
    }

    ::check_timing();
    ::check_lifecycle();
    ::check_resume();
    ::check_frame_only_state();
    ::check_stop_requests();
    ::require(::glGetError() == GL_NO_ERROR, "Context tests produced an OpenGL error");
    std::puts("Context fixed update: PASS (cadence, registration, reset, transitions, resume, frame-only, clear, stop)");
  } catch (const std::exception &error) {
    std::fprintf(::stderr, "Context fixed update: FAIL: %s\n", error.what());
    result = 1;
  }

  ::enet_deinitialize();
  ::glfwDestroyWindow(::window);
  ::glfwTerminate();

  return result;
}
