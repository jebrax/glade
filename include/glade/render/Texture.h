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
  unsigned char* data;
  int bufferId;
  
public:
    Texture(int texWidth, int texHeight, const std::vector<unsigned char>& textureData)
      : Texture(texWidth, texHeight, textureData.data(), textureData.size()) {}

    Texture(int texWidth, int texHeight, const unsigned char* textureData, unsigned int size)
      : textureWidth(texWidth), textureHeight(texHeight), data(nullptr), bufferId(-1) {
      if (texWidth <= 0 || texHeight <= 0 || textureData == nullptr || size == 0)
        throw GladeException("Texture requires positive dimensions and pixel data");
      setData(textureData, size);
    }

    ~Texture(void) {
      erase();
    }
  
    bool isTextureSizePowerOfTwo() {
      return isPowerOfTwo(textureWidth) && isPowerOfTwo(textureHeight);
    }
    
    bool hasVideoBufferHandle() {
      return bufferId >= 0;
    }
    
    void erase() {
      log("Not erasing texture for debug purposes (THIS IS MEMORY LEAK!)"); // FIXME
      //if (data != NULL) {
      //  delete [] data;
      //  data = NULL;
      //}
    }
    
    unsigned char* getData() {
      return data;
    }
    
    void setVideoBufferHandle(int bufferId) {
      this->bufferId = bufferId;
    }
    
    int getVideoBufferHandle() {
      return bufferId;
    }
    
private:
  bool isPowerOfTwo(int x) {
      return (x <= 0) ? false : (x & (x - 1)) == 0;    
  }
  
  void setData(const unsigned char * input_data, unsigned int size)
  {
    erase();
    data = new unsigned char[size];
    memcpy(data, input_data, size);
  }
};
