#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glade/render/Texture.h>
#include <glade/render/SpriteFrame.h>
#include <glade/render/SpriteAnimation.h>

namespace Glade {

// Validated, immutable definitions shared by sprite instances. Names map to
// stable vector indices; animations reference these indices, not copied frames.
class TextureAtlas {
public:
    using Names = std::map<std::string, std::size_t>;

    TextureAtlas(std::shared_ptr<Texture> texture,
                 std::vector<SpriteFrame> frames, Names frameNames,
                 std::vector<SpriteAnimation> animations, Names animationNames)
        : texture_(std::move(texture)), frames_(std::move(frames)),
          frameNames_(std::move(frameNames)), animations_(std::move(animations)),
          animationNames_(std::move(animationNames)) {
        if (!texture_) throw std::invalid_argument("Atlas requires a texture");
        validateNames(frameNames_, frames_.size());
        validateNames(animationNames_, animations_.size());
        for (const auto& frame : frames_) {
            if (frame.x < 0 || frame.y < 0 || frame.width <= 0 || frame.height <= 0 ||
                static_cast<unsigned>(frame.x) > texture_->textureWidth ||
                static_cast<unsigned>(frame.y) > texture_->textureHeight ||
                static_cast<unsigned>(frame.width) > texture_->textureWidth - frame.x ||
                static_cast<unsigned>(frame.height) > texture_->textureHeight - frame.y ||
                !std::isfinite(frame.pivotX) || !std::isfinite(frame.pivotY))
                throw std::invalid_argument("Invalid atlas frame rectangle or pivot");
        }
        for (const auto& animation : animations_) {
            if (animation.frames.empty())
                throw std::invalid_argument("Animation must contain frames");
            double total = 0;
            for (const auto& entry : animation.frames) {
                if (entry.frameIndex >= frames_.size() ||
                    !std::isfinite(entry.durationMs) || entry.durationMs <= 0)
                    throw std::invalid_argument("Invalid animation frame reference or duration");
                total += entry.durationMs;
            }
            if (!std::isfinite(total))
                throw std::invalid_argument("Animation duration overflow");
        }
    }

    const std::shared_ptr<Texture>& getTexture() const { return texture_; }
    const SpriteFrame& getFrame(std::size_t index) const { return frames_.at(index); }
    const SpriteFrame& getFrame(const std::string& name) const {
        return getFrame(frameNames_.at(name));
    }
    const SpriteAnimation& getAnimation(std::size_t index) const {
        return animations_.at(index);
    }
    const SpriteAnimation& getAnimation(const std::string& name) const {
        return getAnimation(getAnimationIndex(name));
    }
    std::size_t getAnimationIndex(const std::string& name) const {
        return animationNames_.at(name);
    }
    const Names& getFrameNames() const { return frameNames_; }
    const Names& getAnimationNames() const { return animationNames_; }

private:
    // Const members also prevent assignment from invalidating live references.
    const std::shared_ptr<Texture> texture_;
    const std::vector<SpriteFrame> frames_;
    const Names frameNames_;
    const std::vector<SpriteAnimation> animations_;
    const Names animationNames_;

    static void validateNames(const Names& names, std::size_t count) {
        if (count == 0 || names.size() != count)
            throw std::invalid_argument("Every atlas definition requires a unique name");
        std::vector<bool> seen(count, false);
        for (const auto& [name, index] : names) {
            if (name.empty() || index >= count || seen[index])
                throw std::invalid_argument("Invalid atlas name or index");
            seen[index] = true;
        }
    }
};

} // namespace Glade
