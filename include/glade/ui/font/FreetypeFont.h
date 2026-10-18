#pragma once

#include <memory>
#include <vector>

#include "Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class FreetypeFont : public Font {
private:
  static unsigned int instances;

  std::unique_ptr<std::vector<unsigned char>> faceBuffer;
  float minViewportDimension;
  FT_Face face;

  float stringScaleX, stringScaleY;

public:
  FreetypeFont(std::unique_ptr<std::vector<unsigned char>> &face_buffer, int viewport_width,
               int viewport_height);

  ~FreetypeFont();
  virtual void setFontSizePixels(unsigned int width, unsigned int height = 0);
  virtual GladeObject::Drawables *createDrawablesForString(const std::string &string);
};
