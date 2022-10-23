#include "Mesh.h"

namespace Glade {
  class Cube: public Mesh
  {
    public:
      Cube(): Mesh() {}
      
      virtual float* getVertices()
      {
        static float vertices[] = {
          // front face
          0.5f, 0.5f, -0.5f,
          0.0f, 0.0f, -1.0f,
          0.0f, 0.0f,

          0.5f, -0.5f, -0.5f,
          0.0f, 0.0f,  -1.0f,
          0.0f, 1.0f,

          -0.5f, -0.5f, -0.5f,
          0.0f, 0.0f, -1.0f,
          1.0f, 0.0f,

          -0.5f, 0.5f, -0.5f,
          0.0f, 0.0f, -1.0f,
          1.0f, 1.0f,
          
          // back face

          -0.5f, 0.5f, 0.5f,
          0.0f, 0.0f, 1.0f,
          1.0f, 1.0f,

          -0.5f, -0.5f, 0.5f,
          0.0f, 0.0f, 1.0f,
          1.0f, 0.0f,

          0.5f, -0.5f, 0.5f,
          0.0f, 0.0f, 1.0f,
          0.0f, 1.0f,
          
          0.5f, 0.5f, 0.5f,
          0.0f, 0.0f, 1.0f,
          0.0f, 0.0f,

          // left face
          -0.5f, 0.5f, -0.5f,
          -1.0f, 0.0f, 0.0f,
           0.0f, 1.0f,

          -0.5f, -0.5f, -0.5f,
          -1.0f, 0.0f, 0.0f,
           1.0f, 0.0f,
          
          -0.5f, -0.5f, 0.5f,
          -1.0f, 0.0f, 0.0f,
           1.0f, 0.0f,

          -0.5f, 0.5f, 0.5f,
          -1.0f, 0.0f, 0.0f,
          1.0f, 1.0f,
          
          // right face
          0.5f, 0.5f, 0.5f,
          1.0f, 0.0f, 0.0f,
          0.0f, 0.0f,

          0.5f, -0.5f, 0.5f,
          1.0f, 0.0f,  0.0f,
          0.0f, 1.0f,

          0.5f, -0.5f, -0.5f,
          1.0f, 0.0f, 0.0f,
          1.0f, 0.0f,

          0.5f, 0.5f, -0.5f,
          1.0f, 0.0f, 0.0f,
          1.0f, 1.0f,
          
          // top face

          -0.5f, 0.5f, -0.5f,
          0.0f, 1.0f, 0.0f,
          1.0f, 1.0f,

          -0.5f, 0.5f, 0.5f,
          0.0f, 1.0f, 0.0f,
          1.0f, 0.0f,

          0.5f, 0.5f, 0.5f,
          0.0f, 1.0f,  0.0f,
          0.0f, 1.0f,

          0.5f, 0.5f, -0.5f,
          0.0f, 1.0f, 0.0f,
          0.0f, 0.0f,

          // bottom face

          0.5f, -0.5f, -0.5f,
          0.0f, -1.0f, 0.0f,
          0.0f, 0.0f,
          
          0.5f, -0.5f, 0.5f,
          0.0f, -1.0f,  0.0f,
          0.0f, 1.0f,
          
          -0.5f, -0.5f, 0.5f,
          0.0f, -1.0f, 0.0f,
          1.0f, 0.0f,
          
          -0.5f, -0.5f, -0.5f,
          0.0f, -1.0f, 0.0f,
          1.0f, 1.0f,
        };

        return vertices;
      }

      virtual uint32_t* getIndices()
      {
        static uint32_t indices[] = {
          0, 1, 2,
          3, 0, 2,

          4, 5, 6,
          7, 4, 6,
          
          8,  9, 10,
          11, 8, 10,
          
          12, 13, 14,
          15, 12, 14,
          
          16, 17, 18,
          19, 16, 18,
          
          20, 21, 22,
          23, 20, 22,
        };
        
        return indices;
      }

      virtual unsigned int getVertexBufferSize()
      {
        return 192;
      }
      
      virtual unsigned int getIndexBufferSize()
      {
        return 36;
      }
  };
}