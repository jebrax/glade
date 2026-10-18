#pragma once

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <glade/render/TextureAtlas.h>

namespace Glade {

// Per-instance playback. update() consumes milliseconds supplied by the caller;
// getters never advance time. No OpenGL context or wall-clock timer is needed.
class SpriteAnimator {
public:
  explicit SpriteAnimator(std::shared_ptr<const TextureAtlas> atlas_) : atlas(std::move(atlas_)) {
    if (!atlas) throw std::invalid_argument("Animator requires an atlas");
  }

  // Selecting an animation explicitly starts it at entry zero.
  void play(const std::string &name) {
    animationIndex = atlas->getAnimationIndex(name);
    selected = true;
    staticFrame = false;
    entryIndex = 0;
    elapsedMs = 0;
    playing = true;
    finished = false;
  }

  // Select a static atlas frame without starting playback.
  void setFrame(const std::string &name) {
    frameIndex = atlas->getFrameNames().at(name);
    selected = true;
    staticFrame = true;
    playing = false;
    finished = false;
    elapsedMs = 0;
  }

  bool hasFrame() const { return selected; }
  void pause() { playing = false; }

  void resume() {
    if (selected && !staticFrame && !finished) playing = true;
  }

  void update(double delta_ms) {
    if (!std::isfinite(delta_ms) || delta_ms < 0)
      throw std::invalid_argument("Elapsed time must be finite and non-negative");

    if (!playing) return;
    const auto &animation = atlas->getAnimation(animationIndex);

    // Discard whole cycles without iterating over every elapsed frame.
    if (animation.loop) {
      double cycle_ms = 0;

      for (const auto &entry : animation.frames)
        cycle_ms += entry.durationMs;

      delta_ms = std::fmod(delta_ms, cycle_ms);
    }

    while (delta_ms >= animation.frames[entryIndex].durationMs - elapsedMs) {
      delta_ms -= animation.frames[entryIndex].durationMs - elapsedMs;
      elapsedMs = 0;

      if (entryIndex + 1 < animation.frames.size()) {
        ++entryIndex;
      } else if (animation.loop) {
        entryIndex = 0;
      } else {
        playing = false;
        finished = true;

        return; // Keep the final frame visible.
      }
    }

    elapsedMs += delta_ms;
  }

  bool hasAnimation() const { return selected && !staticFrame; }
  bool isPlaying() const { return playing; }
  bool isFinished() const { return finished; }

  std::size_t getCurrentEntryIndex() const {
    requireSelection();
    if (staticFrame) throw std::logic_error("Static frame has no animation entry");

    return entryIndex;
  }

  const SpriteFrame &getCurrentFrame() const {
    requireSelection();

    if (staticFrame) return atlas->getFrame(frameIndex);

    return atlas->getFrame(atlas->getAnimation(animationIndex).frames[entryIndex].frameIndex);
  }

  const std::shared_ptr<const TextureAtlas> &getAtlas() const { return atlas; }

private:
  std::shared_ptr<const TextureAtlas> atlas;
  std::size_t animationIndex = 0, entryIndex = 0;
  std::size_t frameIndex = 0;
  bool staticFrame = false;
  double elapsedMs = 0;
  bool selected = false, playing = false, finished = false;

  void requireSelection() const {
    if (!selected) throw std::logic_error("No sprite animation selected");
  }
};

} // namespace Glade
