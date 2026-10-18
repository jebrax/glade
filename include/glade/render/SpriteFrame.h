#pragma once

namespace Glade {

// Pixel rectangle in the atlas, with a pivot relative to its top-left corner.
// X increases rightward and Y downward. Pivots may lie outside the rectangle.
struct SpriteFrame {
  int x, y, width, height;
  double pivotX, pivotY;
};

} // namespace Glade
