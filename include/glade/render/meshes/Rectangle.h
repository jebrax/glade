#pragma once

#include "Mesh.h"

namespace Glade {
  class Rectangle: public Mesh
  {
    public:
      Rectangle() : Mesh() {}
      
      virtual float* getVertices()
      {
        static float vertices[] = {
          -1, -1, 0,
          0, 0, 0,
          0, 0,
          
          1, -1, 0,
          0, 0, 0,
          1, 0, 
          
          1, 1, 0,
          0, 0, 0,
          1, 1,
          
          -1, 1, 0,
          0, 0, 0,
          0, 1,   
        };
          
        return vertices;
      }

      virtual uint32_t* getIndices()
      {
        static uint32_t indices[] = { 2, 1, 0, 3, 2, 0 };
        return indices;
      }

      virtual unsigned int getVertexBufferSize()
      {
        return 48;
      }
      
      virtual unsigned int getIndexBufferSize()
      {
        return 6;
      }
  };
}