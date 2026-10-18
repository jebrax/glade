#pragma once

#include "glade/math/Vector.h"
#include "Layout.h"

class CenterLayout : public Layout {
public:
  virtual void calculateTransformsForDirectChildrenOf(Widget *widget) const {
    rescaleChildrenOf(widget);

    Transform *parent_transform = widget->getTransform();
    Widget::Children *children = widget->getChildren();

    for (Widget::ChildrenI child = children->begin(); child != children->end(); ++child) {
      (*child)->getTransform()->getPosition()->x = parent_transform->getPosition()->x;
      (*child)->getTransform()->getPosition()->y = parent_transform->getPosition()->y;
    }
  }
};
