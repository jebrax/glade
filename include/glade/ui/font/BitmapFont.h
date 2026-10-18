#pragma once

#include <string>
#include <algorithm>
#include <memory>

#include "glade/ui/font/Font.h"
#include "glade/render/Texture.h"
#include "glade/util/CSVReader.h"
#include "glade/util/ResourceManager.h"
#include "glade/util/Path.h"
#include "glade/GladeObject.h"
#include "glade/render/meshes/Rectangle.h"
#include "glade/render/Drawable.h"

extern Glade::ResourceManager *resource_manager;

class BitmapFont : public Font {
private:
  std::shared_ptr<Texture> atlas;
  int cellWidth, cellHeight;
  int firstGlyphAsciiCode;
  int glyphHeight;
  std::vector<int> glyphWidths;
  int numberOfGlyphsInARow;
  float minViewportDimension;

  // Desired result font height in screen coordinates
  float fontSize;

public:
  BitmapFont(std::shared_ptr<Texture> &atlas_, std::vector<std::vector<std::string>> &parsed_csv,
             float viewport_width, float viewport_height)
      : fontSize(1), atlas(atlas_),
        minViewportDimension(
            (float)(viewport_width < viewport_height ? viewport_width : viewport_height)) {
    int declared_atlas_width = ::atoi(parsed_csv[0][1].c_str());

    cellWidth = ::atoi(parsed_csv[2][1].c_str());
    cellHeight = ::atoi(parsed_csv[3][1].c_str());
    numberOfGlyphsInARow = declared_atlas_width / cellWidth;
    glyphWidths.resize(256, cellWidth);
    setFirstGlyphAsciiCode((unsigned char)::atoi((parsed_csv[4][1].c_str())));
    setGlyphHeight(::atoi(parsed_csv[6][1].c_str()));

    for (int i = 8; i < parsed_csv.size(); ++i) {
      std::string param_name = parsed_csv[i][0];
      std::transform(param_name.begin(), param_name.end(), param_name.begin(), ::tolower);

      if (param_name.compare(0, 4, "char") == 0) {
        if (param_name.find("base width") != std::string::npos) {
          unsigned char extracted_ascii_code = extractAsciiCode(param_name);
          setGlyphWidth(extracted_ascii_code, ::atoi(parsed_csv[i][1].c_str()));
        }
      }
    }
  }

  virtual void setFontSizePixels(unsigned int width, unsigned int height = 0) {
    width = width ? width : height;
    height = height ? height : width;

    fontSize = height / minViewportDimension;
  }

  // Should be manually freed after use
  virtual GladeObject::Drawables *createDrawablesForString(const std::string &string) {
    GladeObject::Drawables *text = new GladeObject::Drawables();
    Drawable *rectangle = NULL;
    int glyph_width;
    Glade::Vector2i glyph_position;
    float next_offset_x = 0;
    stringScaleX = 0;
    stringScaleY = fontSize;

    for (size_t i = 0; i < string.length(); ++i) {
      stringScaleX += ((float)glyphWidths[string[i]] / (float)glyphHeight) * fontSize;
    }

    for (size_t i = 0; i < string.length(); ++i) {
      glyph_position = getGlyphPositionForIndex(getGlyphIndexForAsciiCode(string[i]));
      glyph_width = getGlyphWidth(string[i]);

      rectangle = new Drawable(::resource_manager->getMesh(Glade::ResourceManager::MESH_RECTANGLE),
                               shaderProgram);
      rectangle->getTransform()->getScale()->x =
          ((float)glyph_width / (float)glyphHeight) * fontSize;
      rectangle->getTransform()->getScale()->y = fontSize;

      next_offset_x += rectangle->getTransform()->getScale()->x;
      rectangle->getTransform()->getPosition()->x = next_offset_x - stringScaleX;
      next_offset_x += rectangle->getTransform()->getScale()->x;

      rectangle->setUniform("uTextureSampler0", atlas);
      //      rectangle->setConstantColor((float)(rand() % 100)/100.0f, (float)(rand() % 100)/100.0f, (float)(rand() % 100)/100.0f, 0.5f);
      auto *uv = rectangle->getTexture2DTransform();
      uv->textureScaleX = float(glyph_width) / atlas->textureWidth;
      uv->textureScaleY = float(glyphHeight) / atlas->textureHeight;
      uv->textureOffsetX = float(glyph_position.x * cellWidth) / atlas->textureWidth;
      uv->textureOffsetY = float(glyph_position.y * cellHeight) / atlas->textureHeight;

      text->push_back(rectangle);
    }

    return text;
  }

  void setFontSize(float font_size) { this->fontSize = font_size; }
  float getFontSize() const { return fontSize; }

private:
  std::shared_ptr<Texture> getAtlas() { return atlas; }
  int getCellWidth() const { return cellWidth; }
  int getCellHeight() const { return cellHeight; }
  int getGlyphWidth(unsigned char ascii_code) const { return glyphWidths[ascii_code]; }
  int getGlyphHeight() const { return glyphHeight; }
  int getNumberOfChars() const { return glyphWidths.size(); }

  unsigned getGlyphIndexForAsciiCode(unsigned char ascii_code) const {
    unsigned glyph_index = ascii_code - firstGlyphAsciiCode;

    if (glyph_index < 0 || glyph_index >= glyphWidths.size()) {
      glyph_index = glyphWidths.size() - 1;
    }

    return glyph_index;
  }

  Glade::Vector2i getGlyphPositionForIndex(int glyph_index) const {
    Glade::Vector2i position;

    if (glyph_index < 0 || glyph_index >= glyphWidths.size()) {
      glyph_index = glyphWidths.size() - 1;
    }

    position.x = glyph_index % numberOfGlyphsInARow;
    position.y = glyph_index / numberOfGlyphsInARow;

    return position;
  }

  unsigned char extractAsciiCode(const std::string &param_name) const {
    size_t start = param_name.find(' ');
    size_t end = param_name.find(' ', start + 1);

    if (start == std::string::npos || end == std::string::npos) {
      throw GladeException("BitmapFont: Could not read ASCII code");
    }

    std::string ascii_code_str = param_name.substr(start + 1, end - start);

    return (unsigned char)::atoi(ascii_code_str.c_str());
  }

  void setFirstGlyphAsciiCode(char first_glyph_ascii_code) {
    this->firstGlyphAsciiCode = first_glyph_ascii_code;
  }

  void setGlyphHeight(int glyph_height) { this->glyphHeight = glyph_height; }

  void setGlyphWidth(unsigned char ascii_code, int width) {
    if (ascii_code < 0 || ascii_code >= glyphWidths.size()) {
      return;
    }

    if (width > cellWidth) {
      width = cellWidth;
    }

    glyphWidths[ascii_code] = width;
  }
};
