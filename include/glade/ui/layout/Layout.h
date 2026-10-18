#pragma once

#include <memory>
#include "glade/math/Vector.h"
#include "glade/exception/GladeException.h"

class Widget;

class Layout {
public:
  typedef std::unique_ptr<Layout> Unique;

  enum SizeType {
    MANUAL,
    PERCENT_OF_PARENT,
    PERCENT_OF_SELF_WIDTH,
    PERCENT_OF_SELF_HEIGHT,
    WRAP_CONTENT // TODO Not implemented
  };

  enum HorizontalAlign { LEFT, RIGHT, CENTER };
  enum VerticalAlign { TOP, BOTTOM, MIDDLE };

protected:
  Glade::Vector2f minContentPosition;
  Glade::Vector2f maxContentPosition;
  Glade::Vector4f padding;
  Glade::Vector4f margin;
  HorizontalAlign horizontalAlign;
  VerticalAlign verticalAlign;
  float horizontalSizePercent;
  float verticalSizePercent;
  SizeType horizontalSizeType;
  SizeType verticalSizeType;

public:
  Layout(HorizontalAlign hor = LEFT, VerticalAlign vert = TOP, SizeType hor_size = MANUAL,
         SizeType vert_size = MANUAL)
      : horizontalAlign(hor), verticalAlign(vert), horizontalSizeType(hor_size),
        verticalSizeType(vert_size) {}

  virtual ~Layout() {};
  virtual void calculateTransformsForDirectChildrenOf(Widget *widget) const = 0;
  void setVerticalAlign(VerticalAlign vertical_align) { this->verticalAlign = vertical_align; }

  void setHorizontalAlign(HorizontalAlign horizontal_align) {
    this->horizontalAlign = horizontal_align;
  }

  const Glade::Vector4f *getPadding() const { return &padding; }

  void setPadding(float top, float right, float bottom, float left) {
    padding.set(top, right, bottom, left);
  }

  const Glade::Vector4f *getMargin() const { return &margin; }

  void setMargin(float top, float right, float bottom, float left) {
    margin.set(top, right, bottom, left);
  }

  float getHorizontalSizePercent() const { return horizontalSizePercent; }

  void setHorizontalSizePercent(float horizontal_size) {
    this->horizontalSizePercent = horizontal_size;
  }

  float getVerticalSizePercent() const { return verticalSizePercent; }
  void setVerticalSizePercent(float vertical_size) { this->verticalSizePercent = vertical_size; }

  void setHorizontalSizeType(SizeType size_type) {
    if (size_type == PERCENT_OF_SELF_WIDTH) {
      throw GladeException("Horizontal size cannot be specified as percents of itself.");
    }

    if (size_type == PERCENT_OF_SELF_HEIGHT && verticalSizeType == PERCENT_OF_SELF_WIDTH) {
      throw GladeException(
          "Cannot specify horizontal size as percents of self height,"
          " because height is already specified as percents of this horizontal size.");
    }

    this->horizontalSizeType = size_type;
  }

  SizeType getHorizontalSizeType() { return horizontalSizeType; }

  void setVerticalSizeType(SizeType size_type) {
    if (size_type == PERCENT_OF_SELF_HEIGHT) {
      throw GladeException("Vertical size cannot be specified as percents of itself.");
    }

    if (size_type == PERCENT_OF_SELF_WIDTH && verticalSizeType == PERCENT_OF_SELF_HEIGHT) {
      throw GladeException(
          "Cannot specify vertical size as percents of self width,"
          " because width is already specified as percents of this vertical size.");
    }

    this->verticalSizeType = size_type;
  }

  SizeType getVerticalSizeType() const { return verticalSizeType; }

protected:
  void rescaleChildrenOf(Widget *widget) const;

  /** Возвращает true если в результате были расчитаны оба размера X и Y. Если был расчитан только X - возвращает false */
  bool rescaleX(Widget *widget, Widget *his_parent) const;

  /** Возвращает true если в результате были расчитаны оба размера X и Y. Если был расчитан только Y - возвращает false */
  bool rescaleY(Widget *widget, Widget *his_parent) const;
};
