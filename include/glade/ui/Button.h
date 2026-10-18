#pragma once

#include "glade/ui/Widget.h"
#include "glade/ui/Label.h"
#include "glade/ui/font/BitmapFont.h"
#include "glade/ui/layout/CenterLayout.h"

class Button : public Widget {
private:
  Label *label;

public:
  Button() : Widget(), label(NULL) { initialize(); }

  Button(const std::string &text, std::shared_ptr<Font> &font) : Widget(), label(NULL) {
    initialize();
    label = new Label(text, font);

    getTransform()->getScale()->x = label->getTransform()->getScale()->x;
    getTransform()->getScale()->y = label->getTransform()->getScale()->y;
    addChild(label);
  }

  virtual ~Button() {
    if (label) {
      delete label;
    }
  }

  Label *getLabel() { return label; }

private:
  void initialize() {
    Layout::Unique layout(new CenterLayout());
    setLayout(layout);

    setFocusable(true);
  }
};
