#pragma once

#include "Mesh.h"
#include <iterator>

namespace Glade {
  class Rectangle: public Mesh
  {
    private:
      inline static float vertexData[] = {
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

      inline static uint32_t indexData[] = { 0, 1, 2, 0, 2, 3 };

    public:
      Rectangle() : Mesh() {}

      virtual float* getVertices()
      {
        return vertexData;
      }

      virtual uint32_t* getIndices()
      {
        return indexData;
      }

      virtual unsigned int getVertexBufferSize()
      {
        return static_cast<unsigned int>(std::size(vertexData));
      }
      
      virtual unsigned int getIndexBufferSize()
      {
        return static_cast<unsigned int>(std::size(indexData));
      }
  };
}