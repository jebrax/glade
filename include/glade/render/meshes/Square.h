#include "Mesh.h"

namespace Glade {
  class Square: public Mesh
  {
    public:
      Square(): Mesh() {}
      
      virtual float* getVertices()
      {
        static float vertices[] = {
          0.5f, 0.5f, 0.0f,
          0.577349f, 0.577349f, -0.577349f,
          0.000000f, 0.000000f,

          0.5f, -0.5f, 0.0f,
          0.577349f, -0.577349f, -0.577349f,
          0.000000f, 1.000000f,

          -0.5f, -0.5f, 0.0f,
          -0.577349f, -0.577349f, -0.577349f,
          1.000000f, 0.000000f,

          -0.5f, 0.5f, 0.0f,
          -0.577349f, 0.577349f, -0.577349f,
          1.000000f, 1.000000f,
        };
        
        return vertices;
      }

      virtual uint32_t* getIndices()
      {
        static uint32_t indices[] = {
          2, 1, 0,
          2, 0, 3,
        };

        return indices;
      }

      virtual unsigned int getVertexBufferSize()
      {
        return 32;
      }

      virtual unsigned int getIndexBufferSize()
      {
        return 6;
      }
  };
}