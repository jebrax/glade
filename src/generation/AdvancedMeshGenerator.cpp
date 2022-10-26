#include <glade/generation/AdvancedMeshGenerator.h>

#include <glade/generation/MarchingCubesTables.h>
#include <glade/generation/Grid.h>
#include <glade/generation/noise.h>
#include <glade/render/meshes/Mesh.h>
#include <glade/math/Vector.h>

#include <cmath>

// heightmap
void AdvancedMeshGenerator::generate(const std::function<void(Glade::Vector3i&, float, float, float, float)>& callback, int size)
{
  int x, z;
  float y1, y2, y3, y4;
  
  for (x = 0; x < size; ++x)
  {
    for (z = 0; z < size; ++z) {
      y1 = heightFunction2(x, z + 1);
      y2 = heightFunction2(x, z);
      y3 = heightFunction2(x + 1, z);
      y4 = heightFunction2(x + 1, z + 1);

      int y = (y1 + y2 + y3 + y4) / 4.0;

      //log("%d, %d, %d", x, y, z);
      Glade::Vector3i gridCoord(x, y, z);
      callback(gridCoord, y1, y2, y3, y4);
    }
  }
}

Glade::Vector3f AdvancedMeshGenerator::VertexInterp(float isolevel, const Glade::Vector3f &p1, const Glade::Vector3f &p2, float valp1, float valp2)
{
  double mu;
  Glade::Vector3f p;

  if (std::fabsf(isolevel - valp1) < 0.00001)
    return(p1);
  if (std::fabsf(isolevel - valp2) < 0.00001)
    return(p2);
  if (std::fabsf(valp1 - valp2) < 0.00001)
    return(p1);

  mu = (isolevel - valp1) / (valp2 - valp1);
  p.x = p1.x + mu * (p2.x - p1.x);
  p.y = p1.y + mu * (p2.y - p1.y);
  p.z = p1.z + mu * (p2.z - p1.z);

  return(p);
}

void AdvancedMeshGenerator::faceNormalFromThreeVertices(const Glade::Vector3f &a, const Glade::Vector3f &b, const Glade::Vector3f &c, Glade::Vector3f &result)
{
  Glade::Vector3f first, second;
  first.set(a);
  first.subtract(b);
  second.set(a);
  second.subtract(c);
  first.cross(second, result);
  result.normalize();
}

void AdvancedMeshGenerator::mcGenChunk(const Glade::Vector2i &chunkIndex, Grid &grid, Glade::Mesh &mesh, float isolevel, bool regenerate, FunctionType type)
{
  Glade::Vector3f chunkPoint = grid.chunkPoint(chunkIndex);

  log("Chunk (%d, %d) | xstart: %f, zstart: %f", chunkIndex.x, chunkIndex.y, chunkPoint.x, chunkPoint.z);

  // Generate heightmap for ISOSURFACE_HEIGHTMAP first
  std::unordered_map<Glade::Vector3i, float> heightMap;

  for (int i = 0; i < grid.chunkSizeCells; i++) {
    for (int k = 0; k < grid.chunkSizeCells; k++) {
      Glade::Vector3i relativeCellIndex(i, 0, k);
      Glade::Vector3i absoluteCellIndex = grid.absoluteCellIndex(relativeCellIndex, chunkIndex);
      Glade::Vector3f relativeCellPoint = grid.relativeCellPoint(relativeCellIndex);

      Grid::Cell &cell = grid.getOrCreateCell(absoluteCellIndex);

      Glade::Vector3i heightMapKey;
      for (int cubeVertNum = 0; cubeVertNum < 4; ++cubeVertNum) {
        heightMapKey.x = absoluteCellIndex.x;
        heightMapKey.z = absoluteCellIndex.z;
        heightMapKey.y = cubeVertNum;

        float heightNoiseValue = heightFunction4(cell.p[cubeVertNum].x + chunkPoint.x, cell.p[cubeVertNum].z + chunkPoint.z);

        heightMap[heightMapKey] = heightNoiseValue;
      }
    }
  }

  mesh.erase();
  int index = 0;

  for (int i = 0; i < grid.chunkSizeCells; i++) {
    for (int j = 0; j < Grid::CHUNK_HEIGHT; j++) {
      for (int k = 0; k < grid.chunkSizeCells; k++) {
        Glade::Vector3i relativeCellIndex(i, j, k);
        Glade::Vector3i absoluteCellIndex = grid.absoluteCellIndex(relativeCellIndex, chunkIndex);
        Glade::Vector3f relativeCellPoint = grid.relativeCellPoint(relativeCellIndex);

        Grid::Cell &cell = grid.getOrCreateCell(absoluteCellIndex, regenerate);

        // If regenerate is false it means that vertices are there, but they were altered by other means.
        // In this case we don't want to generate new vertices, but we must recalculate normals
        if (regenerate) {
          switch (type) {
            case ISOSURFACE_NOISE_3D:
              for (int cubeVertNum = 0; cubeVertNum < 8; cubeVertNum++) {
                cell.val[cubeVertNum] = noise3D(cell.p[cubeVertNum].x * 0.04, cell.p[cubeVertNum].y * 0.04, cell.p[cubeVertNum].z * 0.04);
              }
              break;
            case ISOSURFACE_HEIGHTMAP:
              for (int cubeVertNum = 0; cubeVertNum < 4; cubeVertNum++) {
                Glade::Vector3i heightMapKey(absoluteCellIndex.x, cubeVertNum, absoluteCellIndex.z);
                float heightNoiseValue = heightMap[heightMapKey];
                cell.val[cubeVertNum] = isosurfaceFromHeightMap(
                  cell.p[cubeVertNum].y,
                  heightNoiseValue,
                  Grid::CHUNK_HEIGHT * grid.cellSize,
                  isolevel
                );

                cell.val[cubeVertNum + 4] = isosurfaceFromHeightMap(
                  cell.p[cubeVertNum + 4].y,
                  heightNoiseValue,
                  Grid::CHUNK_HEIGHT * grid.cellSize,
                  isolevel
                );
              }
              break;
            case CENTER_CELL_ONLY:
              for (int cubeVertNum = 0; cubeVertNum < 8; cubeVertNum++) {
                Glade::Vector3i centerCellIndex = grid.chunkCenterCellIndex(chunkIndex);
                //bool inTheCenter = grid.doesCubeVertBelongInTheCell(cellIndex, cubeVertNum, centerCellIndex);
                bool inTheCenter = grid.doesCubeVertBelongInTheCell(cell.p[cubeVertNum].x, cell.p[cubeVertNum].y, cell.p[cubeVertNum].z, centerCellIndex);
                cell.val[cubeVertNum] = inTheCenter ? 0.1 : 0.8;
              }
              break;
          }
        }

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

        if (MarchingCubesTables::edgeTable[cubeindex] & 1)
          vertlist[0] = VertexInterp(isolevel, cell.p[0], cell.p[1], cell.val[0], cell.val[1]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 2)
          vertlist[1] = VertexInterp(isolevel, cell.p[1], cell.p[2], cell.val[1], cell.val[2]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 4)
          vertlist[2] = VertexInterp(isolevel, cell.p[2], cell.p[3], cell.val[2], cell.val[3]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 8)
          vertlist[3] = VertexInterp(isolevel, cell.p[3], cell.p[0], cell.val[3], cell.val[0]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 16)
          vertlist[4] = VertexInterp(isolevel, cell.p[4], cell.p[5], cell.val[4], cell.val[5]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 32)
          vertlist[5] = VertexInterp(isolevel, cell.p[5], cell.p[6], cell.val[5], cell.val[6]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 64)
          vertlist[6] = VertexInterp(isolevel, cell.p[6], cell.p[7], cell.val[6], cell.val[7]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 128)
          vertlist[7] = VertexInterp(isolevel, cell.p[7], cell.p[4], cell.val[7], cell.val[4]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 256)
          vertlist[8] = VertexInterp(isolevel, cell.p[0], cell.p[4], cell.val[0], cell.val[4]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 512)
          vertlist[9] = VertexInterp(isolevel, cell.p[1], cell.p[5], cell.val[1], cell.val[5]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 1024)
          vertlist[10] = VertexInterp(isolevel, cell.p[2], cell.p[6], cell.val[2], cell.val[6]);
        if (MarchingCubesTables::edgeTable[cubeindex] & 2048)
          vertlist[11] = VertexInterp(isolevel, cell.p[3], cell.p[7], cell.val[3], cell.val[7]);

        cell.startingVertexIndex = index;

        for (int i = 0; MarchingCubesTables::triTable[cubeindex][i] != -1; i += 3) {
          Glade::Vector3f &v1 = vertlist[MarchingCubesTables::triTable[cubeindex][i + 0]];
          Glade::Vector3f &v2 = vertlist[MarchingCubesTables::triTable[cubeindex][i + 1]];
          Glade::Vector3f &v3 = vertlist[MarchingCubesTables::triTable[cubeindex][i + 2]];

          Glade::Vector3f faceNormal;
          faceNormalFromThreeVertices(v1, v2, v3, faceNormal);

          for (int coordi = 0; coordi < 3; coordi++) {
            // position
            Glade::Vector3f &v = vertlist[MarchingCubesTables::triTable[cubeindex][i + coordi]];
            mesh.vertices.push_back(v.x);
            mesh.vertices.push_back(v.y);
            mesh.vertices.push_back(v.z);

            Glade::Vector3f normal;
            mesh.vertices.push_back(faceNormal.x);
            mesh.vertices.push_back(faceNormal.y);
            mesh.vertices.push_back(faceNormal.z);

            // tex coord
            mesh.vertices.push_back(0.0);
            mesh.vertices.push_back(0.0);

            mesh.indices.push_back(index++);
          }
        }

        cell.numVertices = index - cell.startingVertexIndex;

        if (regenerate) {
          // PROBLEM: Adjancent cell cube vertices probably do not have a corresponding value! Use grid.setValueAtCell() but check it's implementation first
          grid.cells.insert_or_assign(absoluteCellIndex, cell);
        }
      }
    }
  }
}
