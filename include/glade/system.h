#pragma once

namespace Glade {
  namespace System {
    void getViewportSize(unsigned int *width, unsigned int *height);
    void toggleMouseCursor(bool enable);
    void setMouseCursorPosition(double x, double y);
    void getMouseCursorPosition(double &x, double &y);
  }
}

