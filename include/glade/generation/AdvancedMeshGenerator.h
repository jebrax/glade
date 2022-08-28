#pragma once

#include <functional>

namespace Glade {
  class Vector3f;
  class Vector2i;
  class Vector3i;
  class Mesh;
}

class Grid;

class AdvancedMeshGenerator
{
  public:
    enum FunctionType {
      ISOSURFACE_HEIGHTMAP,
      ISOSURFACE_NOISE_3D,
      CENTER_CELL_ONLY
    };

    AdvancedMeshGenerator() {}

    void mcGenChunk(const Glade::Vector2i &chunkIndex, Grid &grid, Glade::Mesh &mesh, float isolevel = 0.5, bool regenerate = true, FunctionType type = ISOSURFACE_HEIGHTMAP);
    // heightmap
    void generate(const std::function<void(Glade::Vector3i&, float, float, float, float)>& callback, int size);

  private:
    Glade::Vector3f VertexInterp(float isolevel, const Glade::Vector3f &p1, const Glade::Vector3f &p2, float valp1, float valp2);
    void faceNormalFromThreeVertices(const Glade::Vector3f &a, const Glade::Vector3f &b, const Glade::Vector3f &c, Glade::Vector3f &result);
};
