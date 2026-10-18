#include <list>

#include "glade/ui/Widget.h"
#include "glade/GladeObject.h"
#include "glade/Callable.h"
#include "glade/math/Transform.h"
#include "glade/ui/layout/Layout.h"

// Tree functions

void Widget::walkDepthFirstPostfix(Widget &widget, WalkFunctor &func) {
  ChildrenI child = widget.getChildren()->begin();

  while (child != widget.getChildren()->end()) {
    walkDepthFirstPostfix(**child, func);
    ++child;
  }

  func(widget);
}

void Widget::walkDepthFirstPrefix(Widget &widget, WalkFunctor &func) {
  func(widget);

  ChildrenI child = widget.getChildren()->begin();

  while (child != widget.getChildren()->end()) {
    walkDepthFirstPrefix(**child, func);
    ++child;
  }
}

// End of tree functions

Widget::Widget(Layout::Unique &layout_)
    : GladeObject(&GladeObject::UNUSED_ENTITY_ID), focused(false), focusable(true) {
  setLayout(layout_);
}

Widget::Widget() : GladeObject(&GladeObject::UNUSED_ENTITY_ID), focused(false), focusable(true) {}

void Widget::setLayout(Layout::Unique &layout_) {
  if (layout_.get() == nullptr) {
    throw GladeException("Layout for widget cannot be null");
  }

  this->layout = std::move(layout_);
}

bool Widget::isPointInside(float x, float y) {
  Transform::SharedVector position = getTransform()->getPosition();
  Transform::SharedVector scale = getTransform()->getScale();

  if (x > position->x - scale->x && x < position->x + scale->x && y > position->y - scale->y &&
      y < position->y + scale->y) {
    return true;
  }

  return false;
}

Widget *Widget::getWidgetAtPoint(float x, float y) {
  if (isPointInside(x, y)) {
    ChildrenI child = children.begin();
    Widget *result;

    while (
        child !=
        children
            .end()) { // FIXME Если виджеты наслаиваются друг на друга, то порядок обхода может быть неверным
      result = (*child)->getWidgetAtPoint(x, y);

      if (result != NULL && result->isFocusable()) {
        return result;
      }

      ++child;
    }

    if (this->isFocusable()) {
      return this;
    }
  }

  return NULL;
}

void Widget::setFocused(bool focused_) {
  for (DrawablesI drawable = focusedViews.begin(); drawable != focusedViews.end(); ++drawable) {
    toggleView(**drawable, focused_);
  }

  for (DrawablesI drawable = unfocusedViews.begin(); drawable != unfocusedViews.end(); ++drawable) {
    toggleView(**drawable, !focused_);
  }

  if (focused_ && !this->focused) {
    for (Callable::CallablesI listener = onFocusListeners.begin();
         listener != onFocusListeners.end(); ++listener) {
      (*listener)->call();
    }
  } else if (!focused_ && this->focused) {
    for (Callable::CallablesI listener = onBlurListeners.begin(); listener != onBlurListeners.end();
         ++listener) {
      (*listener)->call();
    }
  }

  this->focused = focused_;
}
