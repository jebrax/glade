#include <glade/generation/AdvancedMeshGenerator.h>

#include <glade/generation/MarchingCubesTables.h>
#include <glade/generation/Grid.h>
#include <glade/generation/noise.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/math/Vector.h>

#include <cmath>

// heightmap
void AdvancedMeshGenerator::generate(
    const std::function<void(Glade::Vector3i &, float, float, float, float)> &callback, int size) {
  int x, z;
  float y1, y2, y3, y4;

  for (x = 0; x < size; ++x) {
    for (z = 0; z < size; ++z) {
      y1 = ::height_function2(x, z + 1);
      y2 = ::height_function2(x, z);
      y3 = ::height_function2(x + 1, z);
      y4 = ::height_function2(x + 1, z + 1);

      int y = (y1 + y2 + y3 + y4) / 4.0;

      //log("%d, %d, %d", x, y, z);
      Glade::Vector3i grid_coord(x, y, z);
      callback(grid_coord, y1, y2, y3, y4);
    }
  }
}

Glade::Vector3f AdvancedMeshGenerator::vertexInterp(float isolevel, const Glade::Vector3f &p1,
                                                    const Glade::Vector3f &p2, float valp1,
                                                    float valp2) {
  double mu;
  Glade::Vector3f p;

  if (std::fabsf(isolevel - valp1) < 0.00001) return (p1);

  if (std::fabsf(isolevel - valp2) < 0.00001) return (p2);

  if (std::fabsf(valp1 - valp2) < 0.00001) return (p1);

  mu = (isolevel - valp1) / (valp2 - valp1);
  p.x = p1.x + mu * (p2.x - p1.x);
  p.y = p1.y + mu * (p2.y - p1.y);
  p.z = p1.z + mu * (p2.z - p1.z);

  return (p);
}

void AdvancedMeshGenerator::faceNormalFromThreeVertices(const Glade::Vector3f &a,
                                                        const Glade::Vector3f &b,
                                                        const Glade::Vector3f &c,
                                                        Glade::Vector3f &result) {
  Glade::Vector3f first, second;
  first.set(a);
  first.subtract(b);
  second.set(a);
  second.subtract(c);
  first.cross(second, result);
  result.normalize();
}

void AdvancedMeshGenerator::mcGenChunk(const Glade::Vector2i &chunk_index, Grid &grid,
                                       TerrainGeneratorSettings &settings) {
  Glade::Vector3f chunk_point = grid.chunkPoint(chunk_index);

  ::log("Chunk (%d, %d) | xstart: %f, zstart: %f", chunk_index.x, chunk_index.y, chunk_point.x,
        chunk_point.z);

  // Generate heightmap for ISOSURFACE_HEIGHTMAP first
  std::unordered_map<Glade::Vector3i, float> height_map;

  for (int i = 0; i < grid.chunkSizeCells; i++) {
    for (int k = 0; k < grid.chunkSizeCells; k++) {
      Glade::Vector3i relative_cell_index(i, 0, k);
      Glade::Vector3i absolute_cell_index =
          grid.absoluteCellIndex(relative_cell_index, chunk_index);
      Glade::Vector3f relative_cell_point = grid.relativeCellPoint(relative_cell_index);

      // I'm not sure that we need cell here at all (fells like refactoring for Grid::Cell)
      Grid::Cell &cell = grid.getOrCreateCell(absolute_cell_index, true);

      Glade::Vector3i height_map_key;

      for (int cube_vert_num = 0; cube_vert_num < 4; ++cube_vert_num) {
        height_map_key.x = absolute_cell_index.x;
        height_map_key.z = absolute_cell_index.z;
        height_map_key.y = cube_vert_num;

        float height_noise_value =
            ::height_function4(cell.p[cube_vert_num].x + chunk_point.x,
                               cell.p[cube_vert_num].z + chunk_point.z, settings);

        height_map[height_map_key] = height_noise_value;
      }
    }
  }

  // Now generate volume from the heightmap

  for (int i = 0; i < grid.chunkSizeCells; i++) {
    for (int j = 0; j < Grid::CHUNK_HEIGHT; j++) {
      for (int k = 0; k < grid.chunkSizeCells; k++) {
        Glade::Vector3i relative_cell_index(i, j, k);
        Glade::Vector3i absolute_cell_index =
            grid.absoluteCellIndex(relative_cell_index, chunk_index);
        Glade::Vector3f relative_cell_point = grid.relativeCellPoint(relative_cell_index);

        Grid::Cell &cell = grid.getOrCreateCell(absolute_cell_index, true);

        switch (settings.type) {
        case ISOSURFACE_NOISE_3D:

          for (int cube_vert_num = 0; cube_vert_num < 8; cube_vert_num++) {
            cell.val[cube_vert_num] =
                ::noise3_d(cell.p[cube_vert_num].x * 0.04, cell.p[cube_vert_num].y * 0.04,
                           cell.p[cube_vert_num].z * 0.04);
          }

          break;
        case ISOSURFACE_HEIGHTMAP:

          for (int cube_vert_num = 0; cube_vert_num < 4; cube_vert_num++) {
            Glade::Vector3i height_map_key(absolute_cell_index.x, cube_vert_num,
                                           absolute_cell_index.z);
            float height_noise_value = height_map[height_map_key];
            cell.val[cube_vert_num] =
                ::isosurface_from_height_map(cell.p[cube_vert_num].y, height_noise_value,
                                             Grid::CHUNK_HEIGHT * grid.cellSize, settings.isolevel);

            cell.val[cube_vert_num + 4] =
                ::isosurface_from_height_map(cell.p[cube_vert_num + 4].y, height_noise_value,
                                             Grid::CHUNK_HEIGHT * grid.cellSize, settings.isolevel);
          }

          break;
        case CENTER_CELL_ONLY:

          for (int cube_vert_num = 0; cube_vert_num < 8; cube_vert_num++) {
            Glade::Vector3i center_cell_index = grid.chunkCenterCellIndex(chunk_index);
            //bool inTheCenter = grid.doesCubeVertBelongInTheCell(cellIndex, cubeVertNum, centerCellIndex);
            bool in_the_center =
                grid.doesCubeVertBelongInTheCell(cell.p[cube_vert_num].x, cell.p[cube_vert_num].y,
                                                 cell.p[cube_vert_num].z, center_cell_index);
            cell.val[cube_vert_num] = in_the_center ? 0.1 : 0.8;
          }

          break;
        }

        // PROBLEM: Adjancent cell cube vertices probably do not have a corresponding value! Use grid.setValueAtCell() but check it's implementation first
        grid.setCell(absolute_cell_index, cell);
      }
    }
  }
}

void AdvancedMeshGenerator::updateMeshVerticesFromGridIsoValues(const Glade::Vector2i &chunk_index,
                                                                Grid &grid, Glade::Mesh &mesh,
                                                                float isolevel) {
  mesh.erase();
  int index = 0;

  for (int i = 0; i < grid.chunkSizeCells; i++) {
    for (int j = 0; j < Grid::CHUNK_HEIGHT; j++) {
      for (int k = 0; k < grid.chunkSizeCells; k++) {
        Glade::Vector3i relative_cell_index(i, j, k);
        Glade::Vector3i absolute_cell_index =
            grid.absoluteCellIndex(relative_cell_index, chunk_index);

        Grid::Cell &cell = grid.getOrCreateCell(absolute_cell_index);

        int cubeindex = 0;
        if (cell.val[0] < isolevel) cubeindex |= 1;
        if (cell.val[1] < isolevel) cubeindex |= 2;
        if (cell.val[2] < isolevel) cubeindex |= 4;
        if (cell.val[3] < isolevel) cubeindex |= 8;
        if (cell.val[4] < isolevel) cubeindex |= 16;
        if (cell.val[5] < isolevel) cubeindex |= 32;
        if (cell.val[6] < isolevel) cubeindex |= 64;
        if (cell.val[7] < isolevel) cubeindex |= 128;

        Glade::Vector3f vertlist[12];

        if (MarchingCubesTables::edge_table[cubeindex] & 1)
          vertlist[0] = vertexInterp(isolevel, cell.p[0], cell.p[1], cell.val[0], cell.val[1]);

        if (MarchingCubesTables::edge_table[cubeindex] & 2)
          vertlist[1] = vertexInterp(isolevel, cell.p[1], cell.p[2], cell.val[1], cell.val[2]);

        if (MarchingCubesTables::edge_table[cubeindex] & 4)
          vertlist[2] = vertexInterp(isolevel, cell.p[2], cell.p[3], cell.val[2], cell.val[3]);

        if (MarchingCubesTables::edge_table[cubeindex] & 8)
          vertlist[3] = vertexInterp(isolevel, cell.p[3], cell.p[0], cell.val[3], cell.val[0]);

        if (MarchingCubesTables::edge_table[cubeindex] & 16)
          vertlist[4] = vertexInterp(isolevel, cell.p[4], cell.p[5], cell.val[4], cell.val[5]);

        if (MarchingCubesTables::edge_table[cubeindex] & 32)
          vertlist[5] = vertexInterp(isolevel, cell.p[5], cell.p[6], cell.val[5], cell.val[6]);

        if (MarchingCubesTables::edge_table[cubeindex] & 64)
          vertlist[6] = vertexInterp(isolevel, cell.p[6], cell.p[7], cell.val[6], cell.val[7]);

        if (MarchingCubesTables::edge_table[cubeindex] & 128)
          vertlist[7] = vertexInterp(isolevel, cell.p[7], cell.p[4], cell.val[7], cell.val[4]);

        if (MarchingCubesTables::edge_table[cubeindex] & 256)
          vertlist[8] = vertexInterp(isolevel, cell.p[0], cell.p[4], cell.val[0], cell.val[4]);

        if (MarchingCubesTables::edge_table[cubeindex] & 512)
          vertlist[9] = vertexInterp(isolevel, cell.p[1], cell.p[5], cell.val[1], cell.val[5]);

        if (MarchingCubesTables::edge_table[cubeindex] & 1024)
          vertlist[10] = vertexInterp(isolevel, cell.p[2], cell.p[6], cell.val[2], cell.val[6]);

        if (MarchingCubesTables::edge_table[cubeindex] & 2048)
          vertlist[11] = vertexInterp(isolevel, cell.p[3], cell.p[7], cell.val[3], cell.val[7]);

        // used in collision detection
        cell.startingVertexIndex = index;

        for (int i = 0; MarchingCubesTables::tri_table[cubeindex][i] != -1; i += 3) {
          Glade::Vector3f &v1 = vertlist[MarchingCubesTables::tri_table[cubeindex][i + 0]];
          Glade::Vector3f &v2 = vertlist[MarchingCubesTables::tri_table[cubeindex][i + 1]];
          Glade::Vector3f &v3 = vertlist[MarchingCubesTables::tri_table[cubeindex][i + 2]];

          Glade::Vector3f face_normal;
          faceNormalFromThreeVertices(v1, v2, v3, face_normal);

          for (int coordi = 0; coordi < 3; coordi++) {
            // position
            Glade::Vector3f &v = vertlist[MarchingCubesTables::tri_table[cubeindex][i + coordi]];
            mesh.vertices.push_back(v.x);
            mesh.vertices.push_back(v.y);
            mesh.vertices.push_back(v.z);

            Glade::Vector3f normal;
            mesh.vertices.push_back(face_normal.x);
            mesh.vertices.push_back(face_normal.y);
            mesh.vertices.push_back(face_normal.z);

            // tex coord
            mesh.vertices.push_back(0.0);
            mesh.vertices.push_back(0.0);

            mesh.indices.push_back(index++);
          }
        }

        // used in collision detection
        cell.numVertices = index - cell.startingVertexIndex;
      }
    }
  }
}
