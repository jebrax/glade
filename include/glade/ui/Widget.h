#pragma once

#include <list>

#include "../GladeObject.h"
#include "../Callable.h"
#include "../math/Transform.h"
#include "layout/Layout.h"

class Widget : public GladeObject {
  // Tree functions
public:
  typedef std::list<Widget *> Children;
  typedef Children::iterator ChildrenI;

  class WalkFunctor {
  public:
    virtual void operator()(Widget &widget) = 0;
  };

  static void walkDepthFirstPostfix(Widget &widget, WalkFunctor &func);
  static void walkDepthFirstPrefix(Widget &widget, WalkFunctor &func);

private:
  Children children;

public:
  void addChild(Widget *child) { children.push_back(child); }
  Children *getChildren() { return &children; }
  // End of tree functions

protected:
  bool focusable;

private:
  typedef GladeObject super;

  Layout::Unique layout;
  Drawables unfocusedViews;
  Drawables focusedViews;
  bool focused;
  Callable::Callables onFocusListeners;
  Callable::Callables onBlurListeners;
  Callable::AutoDispose action;

public:
  Widget(Layout::Unique &layout_);
  Widget();
  void setLayout(Layout::Unique &layout_);

  void addFocusedView(Drawable *view) {
    addDrawable(view);
    toggleView(*view, focused);
    focusedViews.push_back(view);
  }

  void addUnfocusedView(Drawable *view) {
    addDrawable(view);
    toggleView(*view, !focused);
    unfocusedViews.push_back(view);
  }

  Layout::Unique *getLayout() { return &layout; }
  bool isFocusable() { return focusable; }
  void setFocusable(bool focusable_) { this->focusable = focusable_; }
  void addOnFocusListener(Callable *listener) { onFocusListeners.insert(listener); }
  void addOnBlurListener(Callable *listener) { onBlurListeners.insert(listener); }
  void removeOnFocusListener(Callable *listener) { onFocusListeners.erase(listener); }
  void removeOnBlurListener(Callable *listener) { onBlurListeners.erase(listener); }
  void setAction(Callable::AutoDispose &action_) { this->action = std::move(action_); }

  void performAction() {
    if (action.get() != nullptr) {
      action->call();
    }
  }

  bool isPointInside(float x, float y);
  Widget *getWidgetAtPoint(float x, float y);
  void setFocused(bool focused_);
};
