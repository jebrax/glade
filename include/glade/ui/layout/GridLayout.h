#pragma once

#include <glade/math/Transform.h>
#include <glade/math/Vector.h>
#include <glade/ui/Widget.h>

class GridLayout : public Layout {
private:
  int rowLength;

public:
  GridLayout(int row_length = 0) : rowLength(row_length) {}

  virtual void calculateTransformsForDirectChildrenOf(Widget *widget) const {
    rescaleChildrenOf(widget);

    float cell_width, cell_height;
    cell_width = cell_height = 2 * widget->getTransform()->getScale()->x / rowLength;

    Transform *parent_transform = widget->getTransform();
    const Glade::Vector4f *padding = getPadding();

    float home_x = parent_transform->getPosition()->x + padding->w, current_x = home_x,
          current_y = parent_transform->getPosition()->y + padding->x,
          current_z = parent_transform->getPosition()->z;

    int number_in_a_row = 0;

    Widget::ChildrenI child = widget->getChildren()->begin();

    while (child != widget->getChildren()->end()) {
      ++number_in_a_row;
      Transform::SharedVector child_scale = (*child)->getTransform()->getScale();
      (*child)->getTransform()->getPosition()->x =
          current_x - parent_transform->getScale()->x + child_scale->x;
      (*child)->getTransform()->getPosition()->y =
          current_y - parent_transform->getScale()->y + child_scale->y;
      (*child)->getTransform()->getPosition()->z = current_z;

      if (number_in_a_row >= rowLength) {
        current_y += cell_height;
        current_x = home_x;
        number_in_a_row = 0;
      } else {
        current_x += cell_width;
      }

      ++child;
    }
  }

  /**
     *
     * @param rowLength Number of cells in the row
     */
  void setRowLength(int row_length) { this->rowLength = row_length; }
};
