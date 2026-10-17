#pragma once

#include <vector>
#include <glade/render/AnimationFrame.h>

namespace Glade {

struct SpriteAnimation {
    std::vector<AnimationFrame> frames;
    bool loop;
};

} // namespace Glade
