#pragma once

#include <cstring>

#include <vector>

#include "../exception/GladeException.h"
#include "../debug/log.h"

class Texture {
public:
    const unsigned int textureWidth;
    const unsigned int textureHeight;
    const unsigned char numberOfFrames;
    const unsigned char numberOfAnimations;
    const float frameWidth;
    const float frameHeight;
    const float texCoordFrameWidth;  // Ширина кадра в координатах S
    const float texCoordFrameHeight; // Высота кадра в координатах T
  
private:
  unsigned char* data;
  int bufferId;
  
public:
    Texture(int texWidth, int texHeight, int numOfAnimations, int numOfFrames, std::vector<unsigned char> &textureData):
      data(NULL),
      bufferId(-1),
      textureWidth(texWidth),
      textureHeight(texHeight),
      numberOfFrames(numOfFrames),
      numberOfAnimations(numOfAnimations),
      frameWidth((float) textureWidth / (float) numberOfFrames),
      frameHeight((float) textureHeight / (float) numberOfAnimations),
      texCoordFrameWidth(1 / (float) numberOfFrames),
      texCoordFrameHeight(1 / (float) numberOfAnimations)
    {
      if (texWidth <= 0 || texHeight <= 0 || numberOfAnimations <= 0 || numberOfFrames <= 0) {
         throw GladeException("Texture::Texture(): Incorrect parameters for texture constructor method");
      }
      
      setData(&textureData[0], textureData.size());
    }
    
    Texture(int texWidth, int texHeight, int numOfAnimations, int numOfFrames, unsigned char *textureData, unsigned int size):
      data(NULL),
      bufferId(-1),
      textureWidth(texWidth),
      textureHeight(texHeight),
      numberOfFrames(numOfFrames),
      numberOfAnimations(numOfAnimations),
      frameWidth((float) textureWidth / (float) numberOfFrames),
      frameHeight((float) textureHeight / (float) numberOfAnimations),
      texCoordFrameWidth(1 / (float) numberOfFrames),
      texCoordFrameHeight(1 / (float) numberOfAnimations)
    {
      if (texWidth <= 0 || texHeight <= 0 || numberOfAnimations <= 0 || numberOfFrames <= 0) {
         throw GladeException("Texture::Texture(): Incorrect parameters for texture constructor");
      }
      
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
