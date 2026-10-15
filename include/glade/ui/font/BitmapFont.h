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

class BitmapFont: public Font
{
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
    BitmapFont(std::shared_ptr<Texture> &atlas,
      std::vector<std::vector<std::string> > &parsed_csv,
      float viewport_width, float viewport_height):
        fontSize(1),
        atlas(atlas),
        minViewportDimension(
          (float) (viewport_width < viewport_height ? viewport_width : viewport_height)
        )
    {
      int declaredAtlasWidth  = ::atoi(parsed_csv[0][1].c_str());
      
      cellWidth            = ::atoi(parsed_csv[2][1].c_str());
      cellHeight           = ::atoi(parsed_csv[3][1].c_str());
      numberOfGlyphsInARow = declaredAtlasWidth / cellWidth;
      glyphWidths.resize(256, cellWidth);
      setFirstGlyphAsciiCode((unsigned char) ::atoi((parsed_csv[4][1].c_str())));
      setGlyphHeight(::atoi(parsed_csv[6][1].c_str()));
      
      for (int i = 8; i < parsed_csv.size(); ++i) {
        std::string paramName = parsed_csv[i][0];
        std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::tolower);
        
        if (paramName.compare(0, 4, "char") == 0) {
          if (paramName.find("base width") != std::string::npos) {
            unsigned char extractedAsciiCode = extractAsciiCode(paramName);
            setGlyphWidth(extractedAsciiCode, ::atoi(parsed_csv[i][1].c_str()));
          }
        }
      }
    }

    virtual void setFontSizePixels(unsigned int width, unsigned int height = 0)
    {
      width  = width  ? width  : height;
      height = height ? height :  width;
      
      fontSize = height / minViewportDimension;
    }
    
    // Should be manually freed after use
    virtual GladeObject::Drawables* createDrawablesForString(const std::string &string)
    {
      GladeObject::Drawables* text = new GladeObject::Drawables(); 
      Drawable* rectangle = NULL;
      int glyphWidth;
      Glade::Vector2i glyphPosition;
      float nextOffsetX = 0;
      stringScaleX = 0;
      stringScaleY = fontSize;
      
      for (size_t i = 0; i < string.length(); ++i) {      
        stringScaleX += ((float) glyphWidths[string[i]] / (float) glyphHeight) * fontSize;
      }

      for (size_t i = 0; i < string.length(); ++i) {
        glyphPosition = getGlyphPositionForIndex(getGlyphIndexForAsciiCode(string[i]));
        glyphWidth = getGlyphWidth(string[i]);
        
        rectangle = new Drawable(resource_manager->getMesh(Glade::ResourceManager::MESH_RECTANGLE), shaderProgram);
        rectangle->getTransform()->getScale()->x = ((float) glyphWidth / (float) glyphHeight) * fontSize;
        rectangle->getTransform()->getScale()->y = fontSize;
        
        nextOffsetX += rectangle->getTransform()->getScale()->x;
        rectangle->getTransform()->getPosition()->x = nextOffsetX - stringScaleX;
        nextOffsetX += rectangle->getTransform()->getScale()->x;

        rectangle->setUniform("uTextureSampler0", atlas);
//      rectangle->setConstantColor((float)(rand() % 100)/100.0f, (float)(rand() % 100)/100.0f, (float)(rand() % 100)/100.0f, 0.5f);
        rectangle->getTexture2DTransform()->setCurrentFrameNumber(glyphPosition.x);
        rectangle->getTexture2DTransform()->setCurrentAnimationNumber(glyphPosition.y);
        rectangle->getTexture2DTransform()->setCurrentFrameWidth(glyphWidth);
        rectangle->getTexture2DTransform()->setCurrentFrameHeight(glyphHeight);
        
        text->push_back(rectangle);
      }

      return text;
    }
    
    void setFontSize(float fontSize)
    {
      this->fontSize = fontSize;
    }
    
    float getFontSize() const
    {
      return fontSize;
    }

  private:
    std::shared_ptr<Texture> getAtlas()
    {
      return atlas;
    }
    
    int getCellWidth() const
    {
      return cellWidth;
    }
    
    int getCellHeight() const
    {
      return cellHeight;
    }
    
    int getGlyphWidth(unsigned char asciiCode) const
    {
      return glyphWidths[asciiCode];
    }
    
    int getGlyphHeight() const
    {
      return glyphHeight;
    }
    
    int getNumberOfChars() const
    {
      return glyphWidths.size();
    }
    
    unsigned getGlyphIndexForAsciiCode(unsigned char asciiCode) const
    {
      unsigned glyphIndex = asciiCode - firstGlyphAsciiCode;
      
      if (glyphIndex < 0 || glyphIndex >= glyphWidths.size()) {
        glyphIndex = glyphWidths.size() - 1;
      }
      
      return glyphIndex;
    }
    
    Glade::Vector2i getGlyphPositionForIndex(int glyphIndex) const
    {
      Glade::Vector2i position;
      
      if (glyphIndex < 0 || glyphIndex >= glyphWidths.size()) {
        glyphIndex = glyphWidths.size() - 1;
      }
      
      position.x = glyphIndex % numberOfGlyphsInARow;
      position.y = glyphIndex / numberOfGlyphsInARow;
      
      return position;
    }
    
    unsigned char extractAsciiCode(const std::string &paramName) const
    {
      size_t start = paramName.find(' ');
      size_t end   = paramName.find(' ', start + 1);
      
      if (start == std::string::npos || end == std::string::npos) {
        throw GladeException("BitmapFont: Could not read ASCII code");
      }
      
      std::string asciiCodeStr = paramName.substr(start + 1, end - start);
        
      return (unsigned char) ::atoi(asciiCodeStr.c_str());
    }
    
    void setFirstGlyphAsciiCode(char firstGlyphAsciiCode)
    {
      this->firstGlyphAsciiCode = firstGlyphAsciiCode;
    }
    
    void setGlyphHeight(int glyphHeight)
    {
      this->glyphHeight = glyphHeight;
    }
    
    void setGlyphWidth(unsigned char asciiCode, int width)
    {
      if (asciiCode < 0 || asciiCode >= glyphWidths.size()) {
        return;
      }
      
      if (width > cellWidth) {
        width = cellWidth;
      }
      
      glyphWidths[asciiCode] = width;
    }
};
