#pragma once

#include "Mesh.h"
#include <iterator>

namespace Glade {
  class Triangle : public Mesh
  {
    private:
      inline static float vertexData[] = {
          0.0f, 0.5f, 0.0f,  // position
          0.0f, 0.0f, 0.0f, // normal
          0.0f, 0.0f, // texcoord

          -0.5f, -0.5f, 0.0f,
          0.0f, 0.0f, 0.0f,
          0.0f, 1.0f,

          0.5f, -0.5f, 0.0f,
          0.0f, 0.0f, 0.0f,
          1.0f, 1.0f
        };

      inline static uint32_t indexData[] = { 0, 1, 2 };

    public:
      Triangle() : Mesh() {}

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