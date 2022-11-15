#include "glade/generation/AdvancedMeshGenerator.h"
#include <glade/generation/MeshGenerator.h>
#include <glade/generation/noise.h>
#include <glade/math/Vector.h>

#include <cmath>

MeshGenerator::MeshGenerator():
  tileSize(0.25f)
{
}

void MeshGenerator::extractVertexCoordsFromArray(int vertex_number, Glade::Mesh::Vertices &vertices, Glade::Vector3f &result)
{
  result.x = vertices[vertex_number * VERTEX_SIZE + 0];
  result.y = vertices[vertex_number * VERTEX_SIZE + 1];
  result.z = vertices[vertex_number * VERTEX_SIZE + 2];
}

void MeshGenerator::extractVertexNormalFromTheMesh(int index, Glade::Mesh& mesh, Glade::Vector3f &result)
{
  result.x = mesh.vertices[index * VERTEX_SIZE + 3];
  result.y = mesh.vertices[index * VERTEX_SIZE + 4];
  result.z = mesh.vertices[index * VERTEX_SIZE + 5];
}

void MeshGenerator::saveVertexNormalIntoTheMesh(int index, Glade::Mesh& mesh, Glade::Vector3f &normal)
{
  mesh.vertices[index * VERTEX_SIZE + 3] = normal.x;
  mesh.vertices[index * VERTEX_SIZE + 4] = normal.y;
  mesh.vertices[index * VERTEX_SIZE + 5] = normal.z;
}

void MeshGenerator::surfaceNormalFromThreeVertices(Glade::Vector3f &a, Glade::Vector3f &b, Glade::Vector3f &c, Glade::Vector3f &result)
{
  Glade::Vector3f first, second;
  first.set(a);
  first.subtract(b);
  second.set(a);
  second.subtract(c);
  first.cross(second, result);
  result.normalize();
}

void MeshGenerator::addFaceNormalComponentToItsVertices(Glade::Mesh &mesh, int index1, int index2, int index3)
{
  Glade::Vector3f coords1, coords2, coords3;
  extractVertexCoordsFromArray(index1, mesh.vertices, coords1);
  extractVertexCoordsFromArray(index2, mesh.vertices, coords2);
  extractVertexCoordsFromArray(index3, mesh.vertices, coords3);

  Glade::Vector3f faceNormal;
  surfaceNormalFromThreeVertices(coords1, coords2, coords3, faceNormal);
  
  Glade::Vector3f normal1, normal2, normal3;
  extractVertexNormalFromTheMesh(index1, mesh, normal1);
  extractVertexNormalFromTheMesh(index2, mesh, normal2);
  extractVertexNormalFromTheMesh(index3, mesh, normal3);

  // why add and not set?
  normal1.add(faceNormal);
  normal1.normalize();
  normal2.add(faceNormal);
  normal2.normalize();
  normal3.add(faceNormal);
  normal3.normalize();

  saveVertexNormalIntoTheMesh(index1, mesh, normal1);
  saveVertexNormalIntoTheMesh(index2, mesh, normal2);
  saveVertexNormalIntoTheMesh(index3, mesh, normal3);
}

void MeshGenerator::generateHull(Glade::Mesh &mesh, float radius, bool sphere)
{
  mesh.vertices.clear();
  mesh.indices.clear();

  float theta, phi, r;
  float x, y, z;
  int meshSizeVertices = 0;

  int vertsInABelt = 0;
  int numberOfBelts = 0;

  float phiMin = 0.0f,
        thetaMin = 0.0f,
        phiMax = 3.1415f,
        thetaMax = 2.0f * 3.1415f,
        phiStep = 0.1f,
        thetaStep = 0.1f;

  bool addPoles = false;

  for (phi = std::max(phiMin, phiStep); phi < phiMax; phi += phiStep) {
    vertsInABelt = 0;
    for (theta = thetaMin; theta < thetaMax; theta += thetaStep) {
      y = cos(phi);
      x = cos(theta) * sin(phi);
      z = sin(theta) * sin(phi);
      r = sphere ? radius : noise3D(x, y, z);
      //r = heightFunction1(x, z);
      x *= r; y *= r; z *= r;

      // position
      mesh.vertices.push_back(x);
      mesh.vertices.push_back(y);
      mesh.vertices.push_back(z);

      // normals will be calculated on the second path
      mesh.vertices.push_back(0.0f);
      mesh.vertices.push_back(0.0f);
      mesh.vertices.push_back(0.0f);

      // tex coord (TODO)
      mesh.vertices.push_back(0.0f);
      mesh.vertices.push_back(0.0f);

      ++meshSizeVertices;
      ++vertsInABelt;
    }

    //log("Verts in a belt: %d", vertsInABelt);
    ++numberOfBelts;
  }

  // South pole
  if (addPoles) {
    y = cos(phiMax);
    y *= sphere ? radius : noise3D(0, cos(phiMax), 0);
    x = 0;
    z = 0;

    // position
    mesh.vertices.push_back(x);
    mesh.vertices.push_back(y);
    mesh.vertices.push_back(z);
    // normals
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(-1.0f);
    mesh.vertices.push_back(0.0f);
    // tex coord
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);

    ++meshSizeVertices;
  }

  // North pole
  if (addPoles) {
    y = cos(phiMin);
    y *= sphere ? radius : noise3D(0, y, 0);
    x = 0;
    z = 0;

    // position
    mesh.vertices.push_back(x);
    mesh.vertices.push_back(y);
    mesh.vertices.push_back(z);
    // normals
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(1.0f);
    mesh.vertices.push_back(0.0f);
    // tex coord
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);

    ++meshSizeVertices;
  }

  //log("Number of belts: %d", numberOfBelts);
  log("Total vertices (including 2 poles): %d", meshSizeVertices);

  int i, belt;

  // fill index array
  for (belt = 0; belt < numberOfBelts - 1; ++belt) {
    int beltStart = belt * vertsInABelt;
    for (i = beltStart; i + 1 < beltStart + vertsInABelt; i += 1)
    {
      int index1 = i + vertsInABelt,
          index2 = i,
          index3 = i + 1,
          index4 = i + 1 + vertsInABelt;

      // upper triangle
      mesh.indices.push_back(index1);
      mesh.indices.push_back(index2);
      mesh.indices.push_back(index3);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);

      // lower triangle
      mesh.indices.push_back(index4);
      mesh.indices.push_back(index1);
      mesh.indices.push_back(index3);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
    }

    // Connect the first and the last one in the belt
    {
      int index1 = i + vertsInABelt,
          index2 = i,
          index3 = beltStart,
          index4 = beltStart + vertsInABelt;

      // upper triangle
      mesh.indices.push_back(index1);
      mesh.indices.push_back(index2);
      mesh.indices.push_back(index3);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);

      // lower triangle
      mesh.indices.push_back(index4);
      mesh.indices.push_back(index1);
      mesh.indices.push_back(index3);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
    }
  }

  // Connect last belt vertices with the south pole
  if (addPoles) {
    int beltStart = belt * vertsInABelt;
    for (i = beltStart; i + 1 < beltStart + vertsInABelt; i += 1) {
      int index1 = meshSizeVertices - 2,
          index2 = i,
          index3 = i + 1;

      mesh.indices.push_back(index1);
      mesh.indices.push_back(index2);
      mesh.indices.push_back(index3);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
    }

    // Connect the last and the first triangles in the fan
    int index1 = meshSizeVertices - 2,
        index2 = i,
        index3 = beltStart;

    mesh.indices.push_back(index1);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index3);

    addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
  }

  // Connect first belt vertices with the north pole
  if (addPoles) {
    for (i = 0; i + 1 < vertsInABelt; i += 1) {
      int index1 = meshSizeVertices - 1,
          index2 = i,
          index3 = i + 1;

      mesh.indices.push_back(meshSizeVertices - 1);
      mesh.indices.push_back(i + 1);
      mesh.indices.push_back(i);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
    }

    // Connect the last and the first triangles in the fan
    int index1 = meshSizeVertices - 1,
        index2 = 0,
        index3 = i;

    mesh.indices.push_back(index1);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index3);

    addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
  }
}

void MeshGenerator::generatePlane(Glade::Mesh &mesh, int meshSizeTiles)
{
  mesh.vertices.clear();
  mesh.indices.clear();
  
  int meshSizeVertices = meshSizeTiles + 1;
  
  // calculate vertex positions
  float x, y, z;
  
  for (int i = 0; i < meshSizeVertices * meshSizeVertices; ++i)
  {
    x = i % meshSizeVertices;
    z = -i / meshSizeVertices;
    y = heightFunction2(x, z);

    // position
    mesh.vertices.push_back(x);
    mesh.vertices.push_back(y);
    mesh.vertices.push_back(z);

    // normals
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);

    // tex coord
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
  }

  // fill index array
  int lastVertex = meshSizeVertices * meshSizeVertices - meshSizeVertices;

  for (int i = 0; i < lastVertex; ++i)
  {
    if ((i + 1) % meshSizeVertices == 0)
      continue;

    int index1 = i + 1,
        index2 = i + meshSizeVertices,
        index3 = i,
        index4 = i + meshSizeVertices + 1;
        
    // upper triangle
    mesh.indices.push_back(index1);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index3);

    addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);

    // lower triangle
    mesh.indices.push_back(index4);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index1);

    addFaceNormalComponentToItsVertices(mesh, index4, index2, index1);
  }
}

void MeshGenerator::generate(Glade::Mesh &mesh, int meshSizeTiles, const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings)
{
  mesh.vertices.clear();
  mesh.indices.clear();
  
  int meshSizeVertices = meshSizeTiles + 1;
  
  // calculate vertex positions
  float x, y, z;
  
  for (int i = 0; i < meshSizeVertices * meshSizeVertices; ++i)
  {
    x = i % meshSizeVertices;
    z = -i / meshSizeVertices;
    y = heightFunction5(x, z, settings);

    // position
    mesh.vertices.push_back(x);
    mesh.vertices.push_back(y);
    mesh.vertices.push_back(z);

    // normals
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);

    // tex coord
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(0.0f);
  }

  // fill index array
  int lastVertex = meshSizeVertices * meshSizeVertices - meshSizeVertices;

  for (int i = 0; i < lastVertex; ++i)
  {
    if ((i + 1) % meshSizeVertices == 0)
      continue;

    int index1 = i + 1,
        index2 = i + meshSizeVertices,
        index3 = i,
        index4 = i + meshSizeVertices + 1;
        
    // upper triangle
    mesh.indices.push_back(index1);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index3);

    addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);

    // lower triangle
    mesh.indices.push_back(index4);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index1);

    addFaceNormalComponentToItsVertices(mesh, index4, index2, index1);
  }
}
