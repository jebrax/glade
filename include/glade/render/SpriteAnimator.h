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
    explicit SpriteAnimator(std::shared_ptr<const TextureAtlas> atlas)
        : atlas_(std::move(atlas)) {
        if (!atlas_) throw std::invalid_argument("Animator requires an atlas");
    }

    // Selecting an animation explicitly starts it at entry zero.
    void play(const std::string& name) {
        animationIndex_ = atlas_->getAnimationIndex(name);
        selected_ = true;
        staticFrame_ = false;
        entryIndex_ = 0;
        elapsedMs_ = 0;
        playing_ = true;
        finished_ = false;
    }
    // Select a static atlas frame without starting playback.
    void setFrame(const std::string& name) {
        frameIndex_ = atlas_->getFrameNames().at(name);
        selected_ = true;
        staticFrame_ = true;
        playing_ = false;
        finished_ = false;
        elapsedMs_ = 0;
    }
    bool hasFrame() const { return selected_; }
    void pause() { playing_ = false; }
    void resume() {
        if (selected_ && !staticFrame_ && !finished_) playing_ = true;
    }

    void update(double deltaMs) {
        if (!std::isfinite(deltaMs) || deltaMs < 0)
            throw std::invalid_argument("Elapsed time must be finite and non-negative");
        if (!playing_) return;
        const auto& animation = atlas_->getAnimation(animationIndex_);
        // Discard whole cycles without iterating over every elapsed frame.
        if (animation.loop) {
            double cycleMs = 0;
            for (const auto& entry : animation.frames) cycleMs += entry.durationMs;
            deltaMs = std::fmod(deltaMs, cycleMs);
        }
        while (deltaMs >= animation.frames[entryIndex_].durationMs - elapsedMs_) {
            deltaMs -= animation.frames[entryIndex_].durationMs - elapsedMs_;
            elapsedMs_ = 0;
            if (entryIndex_ + 1 < animation.frames.size()) {
                ++entryIndex_;
            } else if (animation.loop) {
                entryIndex_ = 0;
            } else {
                playing_ = false;
                finished_ = true;
                return; // Keep the final frame visible.
            }
        }
        elapsedMs_ += deltaMs;
    }

    bool hasAnimation() const { return selected_ && !staticFrame_; }
    bool isPlaying() const { return playing_; }
    bool isFinished() const { return finished_; }
    std::size_t getCurrentEntryIndex() const {
        requireSelection();
        if (staticFrame_) throw std::logic_error("Static frame has no animation entry");
        return entryIndex_;
    }
    const SpriteFrame& getCurrentFrame() const {
        requireSelection();
        if (staticFrame_) return atlas_->getFrame(frameIndex_);
        return atlas_->getFrame(
            atlas_->getAnimation(animationIndex_).frames[entryIndex_].frameIndex);
    }
    const std::shared_ptr<const TextureAtlas>& getAtlas() const { return atlas_; }

private:
    std::shared_ptr<const TextureAtlas> atlas_;
    std::size_t animationIndex_ = 0, entryIndex_ = 0;
    std::size_t frameIndex_ = 0;
    bool staticFrame_ = false;
    double elapsedMs_ = 0;
    bool selected_ = false, playing_ = false, finished_ = false;

    void requireSelection() const {
        if (!selected_) throw std::logic_error("No sprite animation selected");
    }
};

} // namespace Glade
