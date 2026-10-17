#pragma once

#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <glade/render/Texture.h>
#include <glade/render/TextureAtlasMetadata.h>

namespace Glade {

// Validated, immutable definitions shared by sprite instances. Names map to
// stable vector indices; animations reference these indices, not copied frames.
class TextureAtlas {
public:
    using Names = TextureAtlasMetadata::Names;

    // Consumes metadata containers without copying. Validation happens before
    // moving, so validation failure leaves metadata unchanged. On success the
    // metadata remains valid, but its moved-from containers are unspecified.
    TextureAtlas(std::shared_ptr<Texture> texture, TextureAtlasMetadata& metadata)
        : texture_(validate(std::move(texture), metadata)),
          frames_(std::move(metadata.frames)), frameNames_(std::move(metadata.frameNames)),
          animations_(std::move(metadata.animations)),
          animationNames_(std::move(metadata.animationNames)) {}

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

    static std::shared_ptr<Texture> validate(std::shared_ptr<Texture> texture,
                                            const TextureAtlasMetadata& metadata) {
        if (!texture) throw std::invalid_argument("Atlas requires a texture");
        metadata.validate();
        for (const auto& frame : metadata.frames) {
            if (static_cast<unsigned>(frame.x) > texture->textureWidth ||
                static_cast<unsigned>(frame.y) > texture->textureHeight ||
                static_cast<unsigned>(frame.width) > texture->textureWidth - frame.x ||
                static_cast<unsigned>(frame.height) > texture->textureHeight - frame.y)
                throw std::invalid_argument("Atlas frame rectangle exceeds texture bounds");
        }
        return texture;
    }
};

} // namespace Glade
