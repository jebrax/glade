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
  TextureAtlas(std::shared_ptr<Texture> texture_, TextureAtlasMetadata &metadata)
      : texture(validate(std::move(texture_), metadata)), frames(std::move(metadata.frames)),
        frameNames(std::move(metadata.frameNames)), animations(std::move(metadata.animations)),
        animationNames(std::move(metadata.animationNames)) {}

  const std::shared_ptr<Texture> &getTexture() const { return texture; }
  const SpriteFrame &getFrame(std::size_t index) const { return frames.at(index); }

  const SpriteFrame &getFrame(const std::string &name) const {
    return getFrame(frameNames.at(name));
  }

  const SpriteAnimation &getAnimation(std::size_t index) const { return animations.at(index); }

  const SpriteAnimation &getAnimation(const std::string &name) const {
    return getAnimation(getAnimationIndex(name));
  }

  std::size_t getAnimationIndex(const std::string &name) const { return animationNames.at(name); }
  const Names &getFrameNames() const { return frameNames; }
  const Names &getAnimationNames() const { return animationNames; }

private:
  // Const members also prevent assignment from invalidating live references.
  const std::shared_ptr<Texture> texture;
  const std::vector<SpriteFrame> frames;
  const Names frameNames;
  const std::vector<SpriteAnimation> animations;
  const Names animationNames;

  static std::shared_ptr<Texture> validate(std::shared_ptr<Texture> texture_,
                                           const TextureAtlasMetadata &metadata) {
    if (!texture_) throw std::invalid_argument("Atlas requires a texture");
    metadata.validate();

    for (const auto &frame : metadata.frames) {
      if (static_cast<unsigned>(frame.x) > texture_->textureWidth ||
          static_cast<unsigned>(frame.y) > texture_->textureHeight ||
          static_cast<unsigned>(frame.width) > texture_->textureWidth - frame.x ||
          static_cast<unsigned>(frame.height) > texture_->textureHeight - frame.y)
        throw std::invalid_argument("Atlas frame rectangle exceeds texture bounds");
    }

    return texture_;
  }
};

} // namespace Glade
