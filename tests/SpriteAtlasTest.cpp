// Standalone CPU test (no renderer or GL context):
// clang++ -std=c++20 -Iinclude tests/SpriteAtlasTest.cpp -o /tmp/glade-atlas-test
#include <glade/render/SpriteAnimator.h>
#include <cassert>
#include <cstdarg>
#include <iostream>
#include <limits>

// Texture currently logs during construction/destruction. No engine linkage is
// otherwise needed by this CPU-only test.
void log(const char *, ...) {}

template <class Exception, class Function> void rejects(Function function) {
  bool rejected = false;

  try {
    function();
  } catch (const Exception &) {
    rejected = true;
  }

  assert(rejected);
}

std::shared_ptr<const Glade::TextureAtlas>
make_atlas(std::shared_ptr<Texture> texture, const std::vector<Glade::SpriteFrame> &frames,
           const Glade::TextureAtlas::Names &names,
           const std::vector<Glade::SpriteAnimation> &animations,
           const Glade::TextureAtlas::Names &animation_names) {
  Glade::TextureAtlasMetadata metadata{"", frames, names, animations, animation_names};
  const auto *frame_storage = metadata.frames.data();
  const auto *entry_storage = metadata.animations[0].frames.data();
  auto atlas = std::make_shared<const Glade::TextureAtlas>(texture, metadata);
  assert(&atlas->getFrame(0) == frame_storage);
  assert(atlas->getAnimation(0).frames.data() == entry_storage);

  return atlas; // Metadata can be destroyed independently of the atlas.
}

int main() {
  using namespace Glade;
  std::vector<unsigned char> pixels(100 * 80 * 4, 255);
  auto texture = std::make_shared<Texture>(100, 80, pixels);
  const std::vector<SpriteFrame> frames = {{2, 3, 20, 30, 10, 29}, {40, 10, 45, 50, -2.5, 55}};
  const TextureAtlas::Names names = {{"idle", 0}, {"punch", 1}};
  const std::vector<SpriteAnimation> animations = {
      {{{0, 80}, {1, 120}, {0, 50}}, true}, {{{0, 80}, {1, 120}}, false}, {{{1, 25}}, true}};
  const TextureAtlas::Names animation_names = {{"loop", 0}, {"once", 1}, {"single", 2}};
  auto atlas = ::make_atlas(texture, frames, names, animations, animation_names);
  assert(atlas->getTexture() == texture);
  assert(atlas->getFrame("punch").pivotX == -2.5);
  SpriteAnimator first(atlas), second(atlas);
  ::rejects<std::logic_error>([&] { first.getCurrentFrame(); });
  first.play("loop");
  second.play("loop");
  first.update(79);
  assert(first.getCurrentEntryIndex() == 0);
  first.update(1);
  assert(first.getCurrentEntryIndex() == 1);
  assert(second.getCurrentEntryIndex() == 0); // Independent instances.
  first.pause();
  first.update(1000);
  first.resume();
  first.update(119);
  assert(first.getCurrentEntryIndex() == 1);
  first.update(1);
  assert(first.getCurrentEntryIndex() == 2);
  assert(&first.getCurrentFrame() == &atlas->getFrame("idle"));
  first.update(50);
  assert(first.getCurrentEntryIndex() == 0);
  first.update(250000080);
  assert(first.getCurrentEntryIndex() == 1);
  first.play("once");
  first.update(199);
  assert(first.isPlaying() && first.getCurrentEntryIndex() == 1);
  first.update(1);
  assert(first.isFinished() && !first.isPlaying());
  first.resume();
  first.update(1000);
  assert(!first.isPlaying() && first.getCurrentEntryIndex() == 1);
  first.play("once");
  assert(!first.isFinished());
  first.update(1e300);
  assert(first.isFinished());
  first.play("single");
  first.update(1e300);
  assert(first.isPlaying() && first.getCurrentEntryIndex() == 0);
  ::rejects<std::out_of_range>([&] { first.play("missing"); });
  ::rejects<std::invalid_argument>([&] { first.update(-1); });
  ::rejects<std::invalid_argument>([&] { first.update(std::numeric_limits<double>::infinity()); });
  ::rejects<std::invalid_argument>([&] { SpriteAnimator invalid(nullptr); });
  TextureAtlasMetadata retained{"", frames, names, animations, animation_names};
  retained.frames[0].width = 101;
  const auto *retained_storage = retained.frames.data();
  ::rejects<std::invalid_argument>([&] { TextureAtlas invalid(texture, retained); });
  assert(retained.frames.data() == retained_storage && retained.frames[0].width == 101);
  auto invalid_frame = frames;
  invalid_frame[0].width = 101;
  ::rejects<std::invalid_argument>([&] {
    auto invalid = ::make_atlas(texture, invalid_frame, names, animations, animation_names);
  });
  invalid_frame = frames;
  invalid_frame[0].pivotX = std::numeric_limits<double>::quiet_NaN();
  ::rejects<std::invalid_argument>([&] {
    auto invalid = ::make_atlas(texture, invalid_frame, names, animations, animation_names);
  });
  auto invalid_animation = animations;
  invalid_animation[0].frames[0].frameIndex = 2;
  ::rejects<std::invalid_argument>([&] {
    auto invalid = ::make_atlas(texture, frames, names, invalid_animation, animation_names);
  });
  invalid_animation = animations;
  invalid_animation[0].frames[0].durationMs = 0;
  ::rejects<std::invalid_argument>([&] {
    auto invalid = ::make_atlas(texture, frames, names, invalid_animation, animation_names);
  });
  invalid_animation = animations;
  invalid_animation[0].frames.clear();
  ::rejects<std::invalid_argument>([&] {
    auto invalid = ::make_atlas(texture, frames, names, invalid_animation, animation_names);
  });
  auto invalid_names = names;
  invalid_names["punch"] = 0;
  ::rejects<std::invalid_argument>([&] {
    auto invalid = ::make_atlas(texture, frames, invalid_names, animations, animation_names);
  });
  const std::vector<SpriteAnimation> fractional = {{{{0, 0.5}, {1, 1.25}}, true}};
  auto fractional_atlas =
      ::make_atlas(texture, frames, names, fractional, TextureAtlas::Names{{"fractional", 0}});
  SpriteAnimator fractional_player(fractional_atlas);
  fractional_player.play("fractional");
  fractional_player.update(0.25);
  fractional_player.pause();
  fractional_player.update(50);
  fractional_player.resume();
  fractional_player.update(0.25);
  assert(fractional_player.getCurrentEntryIndex() == 1);
  fractional_player.update(1.25);
  assert(fractional_player.getCurrentEntryIndex() == 0);
  first.setFrame("punch");
  first.resume();
  first.update(1000);
  assert(!first.hasAnimation() && first.hasFrame() && !first.isPlaying());
  assert(&first.getCurrentFrame() == &atlas->getFrame("punch"));
  ::rejects<std::logic_error>([&] { first.getCurrentEntryIndex(); });
  first.play("loop");
  assert(first.hasAnimation() && first.getCurrentEntryIndex() == 0);
  std::cout
      << "Sprite atlas tests passed: definitions, validation, variable durations, "
         "frame reuse, independent playback, pause/resume, looping, completion, large deltas.\n";
}
