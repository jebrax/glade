#pragma once

namespace Glade {
namespace System {

void get_viewport_size(unsigned int *width, unsigned int *height);
void toggle_mouse_cursor(bool enable);
void set_mouse_cursor_position(double x, double y);
void get_mouse_cursor_position(double &x, double &y);

}
}
