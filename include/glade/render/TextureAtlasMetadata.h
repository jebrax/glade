#pragma once

#include <cmath>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <glade/render/SpriteFrame.h>
#include <glade/render/SpriteAnimation.h>

namespace Glade {

// Image-independent definitions parsed from JSON, or assembled directly in C++.
// Frame references and duration defaults have already been resolved.
struct TextureAtlasMetadata {
  using Names = std::map<std::string, std::size_t>;

  std::string textureFilename;
  std::vector<SpriteFrame> frames;
  Names frameNames;
  std::vector<SpriteAnimation> animations;
  Names animationNames;

  void validate() const {
    validateNames(frameNames, frames.size());
    validateNames(animationNames, animations.size());

    for (const auto &frame : frames) {
      if (frame.x < 0 || frame.y < 0 || frame.width <= 0 || frame.height <= 0 ||
          !std::isfinite(frame.pivotX) || !std::isfinite(frame.pivotY))
        throw std::invalid_argument("Invalid atlas frame rectangle or pivot");
    }

    for (const auto &animation : animations) {
      if (animation.frames.empty()) throw std::invalid_argument("Animation must contain frames");
      double total = 0;

      for (const auto &entry : animation.frames) {
        if (entry.frameIndex >= frames.size() || !std::isfinite(entry.durationMs) ||
            entry.durationMs <= 0)
          throw std::invalid_argument("Invalid animation frame reference or duration");

        total += entry.durationMs;
      }

      if (!std::isfinite(total)) throw std::invalid_argument("Animation duration overflow");
    }
  }

private:
  static void validateNames(const Names &names, std::size_t count) {
    if (count == 0 || names.size() != count)
      throw std::invalid_argument("Every atlas definition requires a unique name");

    std::vector<bool> seen(count, false);

    for (const auto &[name, index] : names) {
      if (name.empty() || index >= count || seen[index])
        throw std::invalid_argument("Invalid atlas name or index");

      seen[index] = true;
    }
  }
};

} // namespace Glade
