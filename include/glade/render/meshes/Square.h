#include "Mesh.h"
#include <iterator>

namespace Glade {

class Square : public Mesh {
private:
  inline static float vertexData[] = {
      0.5f,  0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 0.000000f, 0.000000f,

      0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.000000f, 1.000000f,

      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.000000f, 0.000000f,

      -0.5f, 0.5f,  0.0f, 0.0f, 0.0f, 1.0f, 1.000000f, 1.000000f,
  };

  inline static uint32_t indexData[] = {
      2, 1, 0, 2, 0, 3,
  };

public:
  Square() : Mesh() {}
  virtual float *getVertices() { return vertexData; }
  virtual uint32_t *getIndices() { return indexData; }

  virtual unsigned int getVertexBufferSize() {
    return static_cast<unsigned int>(std::size(vertexData));
  }

  virtual unsigned int getIndexBufferSize() {
    return static_cast<unsigned int>(std::size(indexData));
  }
};

}
