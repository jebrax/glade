#pragma once

#include <glade/render/meshes/Mesh.h>
#include <glade/generation/AdvancedMeshGenerator.h>

namespace Glade {
  class Vector3f;
}

class MeshGenerator
{
  public:
    MeshGenerator();

    void generateHull(Glade::Mesh &mesh, float radius = 3.0, bool sphere = false);
    void generatePlane(Glade::Mesh &mesh, int meshSizeTiles);
    void generate(Glade::Mesh &mesh, int meshSizeTiles, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings);

  private:
    void extractVertexCoordsFromArray(int vertex_number, Glade::Mesh::Vertices &vertices, Glade::Vector3f &result);
    void extractVertexNormalFromTheMesh(int index, Glade::Mesh& mesh, Glade::Vector3f &result);
    void saveVertexNormalIntoTheMesh(int index, Glade::Mesh& mesh, Glade::Vector3f &normal); 
    void surfaceNormalFromThreeVertices(Glade::Vector3f &a, Glade::Vector3f &b, Glade::Vector3f &c, Glade::Vector3f &result);
    void addFaceNormalComponentToItsVertices(Glade::Mesh &mesh, int index1, int index2, int index3);

  private:
    static int const VERTEX_SIZE = 8;

    float        tileSize;
};
