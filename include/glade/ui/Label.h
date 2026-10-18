#pragma once

#include <cassert>

#include "Widget.h"
#include "font/BitmapFont.h"
#include "layout/CenterLayout.h"

extern Glade::ResourceManager *resource_manager;

class Label : public Widget {
private:
  std::string text;

public:
  Label(const std::string &string, std::shared_ptr<Font> &font, float r = 0.0f, float g = 0.0f,
        float b = 0.0f)
      : Widget(), text(string) {
    assert(font != nullptr);

    Layout::Unique layout(new CenterLayout());
    setLayout(layout);

    setFocusable(false);
    /*
      Drawable *debug = new Drawable(Rectangle::INSTANCE, resource_manager->getShaderProgram("texcoord_frames.vertex.glsl", "constant.fragment.glsl"));
      debug->setUniform("uColor", Glade::Vector4f(1.0f, 0.0f, 0.0f, 1.0f));
      addDrawable(debug);
      debug->preserveScale = false;
      */

    addDrawables(*font->createDrawablesForString(text));

    ::log("SCALE X: %3.3f  Y: %3.3f", font->getLastStringScaleX(), font->getLastStringScaleY());
    getTransform()->getScale()->set(font->getLastStringScaleX(), font->getLastStringScaleY(), 1.0f);
    setTextColor(r, g, b);
  }

  virtual ~Label() {
    // TODO delete drawables
  }

  virtual void addDrawable(Drawable *view) {
    Widget::addDrawable(view);
    view->preserveScale = true;
  }

  void setTextColor(float r, float g, float b) {
    for (DrawablesI di = drawables.begin(); di != drawables.end(); ++di) {
      (*di)->setUniform("uColor", Glade::Vector4f(r, g, b, 1.0f));
    }
  }

  std::string getText() { return text; }
};
