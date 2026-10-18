#pragma once

#include <cstddef>

namespace Glade {

// References a frame owned by the same TextureAtlas. Duration is resolved
// from durationMs/defaultFrameDuration before constructing the atlas.
struct AnimationFrame {
  std::size_t frameIndex;
  double durationMs;
};

} // namespace Glade
