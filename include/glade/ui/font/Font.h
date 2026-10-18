#pragma once

#include <glade/GladeObject.h>

class ShaderProgram;

class Font {
protected:
  std::shared_ptr<ShaderProgram> shaderProgram;
  float stringScaleX, stringScaleY;

public:
  Font() : stringScaleX(0), stringScaleY(0) {}
  virtual void setFontSizePixels(unsigned int width, unsigned int height = 0) = 0;
  virtual GladeObject::Drawables *createDrawablesForString(const std::string &string) = 0;

  void setShaderProgram(std::shared_ptr<ShaderProgram> &shader_program) {
    shaderProgram = shader_program;
  }

  float getLastStringScaleX() { return stringScaleX; }
  float getLastStringScaleY() { return stringScaleY; }
};
