#pragma once

#include "glade/math/Vector.h"
#include "Layout.h"
#include "glade/ui/Widget.h"
#include "glade/math/Transform.h"

class SequenceLayout : public Layout {
public:
  virtual void calculateTransformsForDirectChildrenOf(Widget *widget) const {
    rescaleChildrenOf(widget);

    Transform *parent_transform = widget->getTransform();

    float current_x = parent_transform->getPosition()->x + padding.w,
          current_y = parent_transform->getPosition()->y + padding.x,
          current_z = parent_transform->getPosition()->z;

    Widget::Children *children = widget->getChildren();

    for (Widget::ChildrenI child = children->begin(); child != children->end(); ++child) {
      Transform *child_transform = (*child)->getTransform();
      Transform::SharedVector child_scale = child_transform->getScale();
      Transform::SharedVector child_position = child_transform->getPosition();
      Transform::SharedVector parent_scale = parent_transform->getScale();

      child_position->x = current_x - parent_scale->x + child_scale->x;
      child_position->y = current_y - parent_scale->y + child_scale->y;
      child_position->z = current_z;

      current_x += 2 * child_scale->x;
    }
  }
};
