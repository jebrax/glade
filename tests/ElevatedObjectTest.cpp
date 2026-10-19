#include <glade/ElevatedObject.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace {

void require(bool condition, const char *message) {
  if (!condition) throw std::runtime_error(message);
}

template <class Function> void rejects(Function function) {
  bool rejected = false;

  try {
    function();
  } catch (const std::invalid_argument &) {
    rejected = true;
  }

  ::require(rejected, "Expected std::invalid_argument");
}

class TestObject final : public ElevatedObject {
public:
  explicit TestObject(float height = 0.0f) : ElevatedObject(height) {}
  ~TestObject() override { indexById.erase(getId()); }

  int updates = 0;
  float visualElevation = 0.0f;

private:
  void updateElevation() override {
    ++updates;
    visualElevation = getElevation();
  }
};

void test_elevated_object() {
  static_assert(!std::is_copy_constructible_v<ElevatedObject>);
  static_assert(std::has_virtual_destructor_v<ElevatedObject>);
  const auto registry_size = GladeObject::indexById.size();
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float infinity = std::numeric_limits<float>::infinity();

  for (float height : {-1.0f, nan, infinity}) {
    ::rejects([&] { TestObject object(height); });
    ::require(GladeObject::indexById.size() == registry_size,
              "Invalid construction left a dangling registry entry");
  }

  TestObject actor;
  TestObject box(0.3f);
  ::require(actor.getHeight() == 0.0f && actor.getElevation() == 0.0f &&
                actor.getTopElevation() == 0.0f && actor.updates == 0,
            "Default height/elevation or constructor hook is incorrect");
  ::require(actor.getTransform()->position->z == 0.0f,
            "Engine base imposed game-specific render Z");
  actor.getTransform()->setPosition(1.0f, 2.0f, -7.0f);
  ::require(actor.isGrounded() && !actor.hasSupportingSurface(), "Incorrect initial support state");
  box.setSupportingSurface(true);
  ::require(box.hasSupportingSurface(), "Support flag was not stored");
  actor.setElevation(0.4f);
  ::require(!actor.isGrounded(), "Direct elevated placement retained floor support");
  ::require(actor.getTransform()->position->x == 1.0f &&
                actor.getTransform()->position->y == 2.0f &&
                actor.getTransform()->position->z == -7.0f &&
                actor.visualElevation == 0.4f && actor.updates == 1,
            "Elevation changed ground/render coordinates or failed to update visuals");
  box.setElevation(0.2f);
  ::require(std::abs(box.getTopElevation() - 0.5f) < 0.00001f && box.getHeight() == 0.3f,
            "Top elevation did not combine elevation and height");
  box.setElevation(-1.0f);
  ::require(box.isGrounded(), "Clamped floor placement did not restore grounded state");
  ::require(box.getElevation() == 0.0f && box.getTopElevation() == 0.3f && box.updates == 2,
            "Negative elevation was not clamped and applied");

  for (float elevation : {nan, infinity, -infinity}) {
    ::rejects([&] { box.setElevation(elevation); });
    ::require(box.getElevation() == 0.0f && box.updates == 2,
              "Rejected elevation changed state or invoked the hook");
  }

  TestObject large(std::numeric_limits<float>::max());
  ::rejects([&] { large.setElevation(std::numeric_limits<float>::max()); });
}

}

int main() {
  try {
    ::test_elevated_object();
    ::require(GladeObject::indexById.empty(), "Test objects did not unregister");
    std::cout << "ElevatedObject tests passed: height, elevation, top, clamp, hook, coordinates and invalid inputs.\n";
  } catch (const std::exception &error) {
    std::cerr << "FAIL: " << error.what() << '\n';
    return 1;
  }

  return 0;
}
