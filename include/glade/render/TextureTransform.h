#pragma once

#include <cmath>

// Ordinary UV transforms. For atlas sprites only the flip flags (scale signs)
// apply: atlas rectangles control UV scale/offset, Drawable controls world scale.
class TextureTransform {
public:
    float textureScaleX = 1.0f, textureScaleY = 1.0f;
    float textureOffsetX = 0.0f, textureOffsetY = 0.0f;

    void flipTextureHorizontally(bool flip) {
        textureScaleX = flip ? -std::abs(textureScaleX) : std::abs(textureScaleX);
    }
    void flipTextureVertically(bool flip) {
        textureScaleY = flip ? -std::abs(textureScaleY) : std::abs(textureScaleY);
    }
    bool isFlippedHorizontally() const { return textureScaleX < 0; }
    bool isFlippedVertically() const { return textureScaleY < 0; }
};
