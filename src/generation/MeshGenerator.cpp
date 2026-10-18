#include "glade/generation/AdvancedMeshGenerator.h"
#include <glade/generation/MeshGenerator.h>
#include <glade/generation/noise.h>
#include <glade/math/Vector.h>

#include <cmath>

MeshGenerator::MeshGenerator() : tileSize(0.25f) {}

void MeshGenerator::extractVertexCoordsFromArray(int vertex_number, Glade::Mesh::Vertices &vertices,
                                                 Glade::Vector3f &result) {
  result.x = vertices[vertex_number * VERTEX_SIZE + 0];
  result.y = vertices[vertex_number * VERTEX_SIZE + 1];
  result.z = vertices[vertex_number * VERTEX_SIZE + 2];
}

void MeshGenerator::extractVertexNormalFromTheMesh(int index, Glade::Mesh &mesh,
                                                   Glade::Vector3f &result) {
  result.x = mesh.vertices[index * VERTEX_SIZE + 3];
  result.y = mesh.vertices[index * VERTEX_SIZE + 4];
  result.z = mesh.vertices[index * VERTEX_SIZE + 5];
}

void MeshGenerator::saveVertexNormalIntoTheMesh(int index, Glade::Mesh &mesh,
                                                Glade::Vector3f &normal) {
  mesh.vertices[index * VERTEX_SIZE + 3] = normal.x;
  mesh.vertices[index * VERTEX_SIZE + 4] = normal.y;
  mesh.vertices[index * VERTEX_SIZE + 5] = normal.z;
}

void MeshGenerator::surfaceNormalFromThreeVertices(Glade::Vector3f &a, Glade::Vector3f &b,
                                                   Glade::Vector3f &c, Glade::Vector3f &result) {
  Glade::Vector3f first, second;
  first.set(a);
  first.subtract(b);
  second.set(a);
  second.subtract(c);
  first.cross(second, result);
  result.normalize();
}

void MeshGenerator::addFaceNormalComponentToItsVertices(Glade::Mesh &mesh, int index1, int index2,
                                                        int index3) {
  Glade::Vector3f coords1, coords2, coords3;
  extractVertexCoordsFromArray(index1, mesh.vertices, coords1);
  extractVertexCoordsFromArray(index2, mesh.vertices, coords2);
  extractVertexCoordsFromArray(index3, mesh.vertices, coords3);

  Glade::Vector3f face_normal;
  surfaceNormalFromThreeVertices(coords1, coords2, coords3, face_normal);

  Glade::Vector3f normal1, normal2, normal3;
  extractVertexNormalFromTheMesh(index1, mesh, normal1);
  extractVertexNormalFromTheMesh(index2, mesh, normal2);
  extractVertexNormalFromTheMesh(index3, mesh, normal3);

  // why add and not set?
  normal1.add(face_normal);
  normal1.normalize();
  normal2.add(face_normal);
  normal2.normalize();
  normal3.add(face_normal);
  normal3.normalize();

  saveVertexNormalIntoTheMesh(index1, mesh, normal1);
  saveVertexNormalIntoTheMesh(index2, mesh, normal2);
  saveVertexNormalIntoTheMesh(index3, mesh, normal3);
}

void MeshGenerator::generateHull(Glade::Mesh &mesh, float radius, bool sphere) {
  mesh.vertices.clear();
  mesh.indices.clear();

  float theta, phi, r;
  float x, y, z;
  int mesh_size_vertices = 0;

  int verts_in_a_belt = 0;
  int number_of_belts = 0;

  float phi_min = 0.0f, theta_min = 0.0f, phi_max = 3.1415f, theta_max = 2.0f * 3.1415f,
        phi_step = 0.1f, theta_step = 0.1f;

  bool add_poles = false;

  for (phi = std::max(phi_min, phi_step); phi < phi_max; phi += phi_step) {
    verts_in_a_belt = 0;

    for (theta = theta_min; theta < theta_max; theta += theta_step) {
      y = ::cos(phi);
      x = ::cos(theta) * ::sin(phi);
      z = ::sin(theta) * ::sin(phi);
      r = sphere ? radius : ::noise3_d(x, y, z);
      //r = heightFunction1(x, z);
      x *= r;
      y *= r;
      z *= r;

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

      ++mesh_size_vertices;
      ++verts_in_a_belt;
    }

    //log("Verts in a belt: %d", vertsInABelt);
    ++number_of_belts;
  }

  // South pole

  if (add_poles) {
    y = ::cos(phi_max);
    y *= sphere ? radius : ::noise3_d(0, ::cos(phi_max), 0);
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

    ++mesh_size_vertices;
  }

  // North pole

  if (add_poles) {
    y = ::cos(phi_min);
    y *= sphere ? radius : ::noise3_d(0, y, 0);
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

    ++mesh_size_vertices;
  }

  //log("Number of belts: %d", numberOfBelts);
  ::log("Total vertices (including 2 poles): %d", mesh_size_vertices);

  int i, belt;

  // fill index array

  for (belt = 0; belt < number_of_belts - 1; ++belt) {
    int belt_start = belt * verts_in_a_belt;

    for (i = belt_start; i + 1 < belt_start + verts_in_a_belt; i += 1) {
      int index1 = i + verts_in_a_belt, index2 = i, index3 = i + 1,
          index4 = i + 1 + verts_in_a_belt;

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
      int index1 = i + verts_in_a_belt, index2 = i, index3 = belt_start,
          index4 = belt_start + verts_in_a_belt;

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

  if (add_poles) {
    int belt_start = belt * verts_in_a_belt;

    for (i = belt_start; i + 1 < belt_start + verts_in_a_belt; i += 1) {
      int index1 = mesh_size_vertices - 2, index2 = i, index3 = i + 1;

      mesh.indices.push_back(index1);
      mesh.indices.push_back(index2);
      mesh.indices.push_back(index3);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
    }

    // Connect the last and the first triangles in the fan
    int index1 = mesh_size_vertices - 2, index2 = i, index3 = belt_start;

    mesh.indices.push_back(index1);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index3);

    addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
  }

  // Connect first belt vertices with the north pole

  if (add_poles) {
    for (i = 0; i + 1 < verts_in_a_belt; i += 1) {
      int index1 = mesh_size_vertices - 1, index2 = i, index3 = i + 1;

      mesh.indices.push_back(mesh_size_vertices - 1);
      mesh.indices.push_back(i + 1);
      mesh.indices.push_back(i);

      addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
    }

    // Connect the last and the first triangles in the fan
    int index1 = mesh_size_vertices - 1, index2 = 0, index3 = i;

    mesh.indices.push_back(index1);
    mesh.indices.push_back(index2);
    mesh.indices.push_back(index3);

    addFaceNormalComponentToItsVertices(mesh, index1, index2, index3);
  }
}

void MeshGenerator::generatePlane(Glade::Mesh &mesh, int mesh_size_tiles) {
  mesh.vertices.clear();
  mesh.indices.clear();

  int mesh_size_vertices = mesh_size_tiles + 1;

  // calculate vertex positions
  float x, y, z;

  for (int i = 0; i < mesh_size_vertices * mesh_size_vertices; ++i) {
    x = i % mesh_size_vertices;
    z = -i / mesh_size_vertices;
    y = ::height_function2(x, z);

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
  int last_vertex = mesh_size_vertices * mesh_size_vertices - mesh_size_vertices;

  for (int i = 0; i < last_vertex; ++i) {
    if ((i + 1) % mesh_size_vertices == 0) continue;

    int index1 = i + 1, index2 = i + mesh_size_vertices, index3 = i,
        index4 = i + mesh_size_vertices + 1;

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

void MeshGenerator::generate(
    Glade::Mesh &mesh, int mesh_size_tiles,
    const std::vector<AdvancedMeshGenerator::TerrainGeneratorSettings> &settings) {
  mesh.vertices.clear();
  mesh.indices.clear();

  int mesh_size_vertices = mesh_size_tiles + 1;

  // calculate vertex positions
  float x, y, z;

  for (int i = 0; i < mesh_size_vertices * mesh_size_vertices; ++i) {
    x = i % mesh_size_vertices;
    z = -i / mesh_size_vertices;
    y = ::height_function5(x, z, settings);
    //y = heightFunction1(x, z);

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
  int last_vertex = mesh_size_vertices * mesh_size_vertices - mesh_size_vertices;

  for (int i = 0; i < last_vertex; ++i) {
    if ((i + 1) % mesh_size_vertices == 0) continue;

    int index1 = i + 1, index2 = i + mesh_size_vertices, index3 = i,
        index4 = i + mesh_size_vertices + 1;

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
