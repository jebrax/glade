#pragma once

#include "glade/math/Vector.h"
#include "Layout.h"
#include "glade/math/Transform.h"

class ListLayout : public Layout {
public:
  virtual void calculateTransformsForDirectChildrenOf(Widget *widget) const {
    rescaleChildrenOf(widget);

    Transform *parent_transform = widget->getTransform();
    Transform::SharedVector parent_position = parent_transform->getPosition();

    float current_x = parent_position->x, current_y = parent_position->y,
          current_z = parent_position->z;

    Widget::Children *children = widget->getChildren();

    for (Widget::ChildrenI child = children->begin(); child != children->end(); ++child) {
      Transform *child_transform = (*child)->getTransform();
      Transform::SharedVector child_scale = child_transform->getScale();
      Transform::SharedVector parent_scale = parent_transform->getScale();

      float horizontal_align_offset = 0;
      float vertical_align_offset = 0;

      switch (horizontalAlign) {
      case LEFT:
        horizontal_align_offset = -parent_scale->x + child_scale->x + padding.w;
        break;
      case RIGHT:
        horizontal_align_offset = parent_scale->x - child_scale->x - padding.y;
        break;
      }

      switch (verticalAlign) {
      case TOP:
        vertical_align_offset = -parent_scale->y + child_scale->y + padding.x;
        break;
      case BOTTOM:
        vertical_align_offset = parent_scale->y - child_scale->y + padding.z;
        break;
      }

      Transform::SharedVector child_position = child_transform->getPosition();

      child_position->x = current_x + horizontal_align_offset;
      child_position->y = current_y + vertical_align_offset;
      child_position->z = current_z;

      current_y += 2 * child_scale->y;
    }
  }
};
