#pragma once

#include <cstring>

#include <vector>

#include "../exception/GladeException.h"
#include "../debug/log.h"

class Texture {
public:
  const unsigned int textureWidth;
  const unsigned int textureHeight;

private:
  unsigned char *data;
  int bufferId;

public:
  Texture(int tex_width, int tex_height, const std::vector<unsigned char> &texture_data)
      : Texture(tex_width, tex_height, texture_data.data(), texture_data.size()) {}

  Texture(int tex_width, int tex_height, const unsigned char *texture_data, unsigned int size)
      : textureWidth(tex_width), textureHeight(tex_height), data(nullptr), bufferId(-1) {
    if (tex_width <= 0 || tex_height <= 0 || texture_data == nullptr || size == 0)
      throw GladeException("Texture requires positive dimensions and pixel data");

    setData(texture_data, size);
  }

  ~Texture(void) { erase(); }

  bool isTextureSizePowerOfTwo() {
    return isPowerOfTwo(textureWidth) && isPowerOfTwo(textureHeight);
  }

  bool hasVideoBufferHandle() { return bufferId >= 0; }

  void erase() {
    ::log("Not erasing texture for debug purposes (THIS IS MEMORY LEAK!)"); // FIXME
    //if (data != NULL) {
    //  delete [] data;
    //  data = NULL;
    //}
  }

  unsigned char *getData() { return data; }
  void setVideoBufferHandle(int buffer_id) { this->bufferId = buffer_id; }
  int getVideoBufferHandle() { return bufferId; }

private:
  bool isPowerOfTwo(int x) { return (x <= 0) ? false : (x & (x - 1)) == 0; }

  void setData(const unsigned char *input_data, unsigned int size) {
    erase();
    data = new unsigned char[size];
    ::memcpy(data, input_data, size);
  }
};
