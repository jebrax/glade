#pragma once

#include <unordered_map>

struct Grid
{
  struct Cell
  {
    Cell(const Glade::Vector3i& cellIndex, float cellSize)
    {
      float x = cellIndex.x * cellSize;
      float y = cellIndex.y * cellSize;
      float z = cellIndex.z * cellSize;

      for (int i = 0; i < 8; ++i) {
        val[i] = 0.6;
      }

      p[0] = Glade::Vector3f(x, y, z);
      p[1] = Glade::Vector3f(x + cellSize, y, z),
      p[2] = Glade::Vector3f(x + cellSize, y, z + cellSize),
      p[3] = Glade::Vector3f(x, y, z + cellSize),
      p[4] = Glade::Vector3f(x, y + cellSize, z),
      p[5] = Glade::Vector3f(x + cellSize, y + cellSize, z),
      p[6] = Glade::Vector3f(x + cellSize, y + cellSize, z + cellSize),
      p[7] = Glade::Vector3f(x, y + cellSize, z + cellSize);
    }

    Glade::Vector3f p[8];
    float val[8];
  };

  Grid(int chunkSizeCells, float cellSize): chunkSizeCells(chunkSizeCells), cellSize(cellSize)
  {
    chunkSizeCoords = cellSize * chunkSizeCells;

    cubeAdjacencyMap[Glade::Vector3i(0, 0, 0)] = new std::vector<int>({   0,   1,   2,   3,   4,   5,   6,   7, });
    cubeAdjacencyMap[Glade::Vector3i(0, 0, 1)] = new std::vector<int>({   -1,   -1,   1,   0,   -1,   -1,   5,   4, });
    cubeAdjacencyMap[Glade::Vector3i(0, 0, -1)] = new std::vector<int>({   3,   2,   -1,   -1,   7,   6,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(0, 1, 0)] = new std::vector<int>({   -1,   -1,   -1,   -1,   0,   1,   2,   3, });
    cubeAdjacencyMap[Glade::Vector3i(0, 1, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   -1,   1,   0, });
    cubeAdjacencyMap[Glade::Vector3i(0, 1, -1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   3,   2,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(0, -1, 0)] = new std::vector<int>({   4,   5,   6,   7,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(0, -1, 1)] = new std::vector<int>({   -1,   -1,   5,   4,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(0, -1, -1)] = new std::vector<int>({   7,   6,   -1,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, 0, 0)] = new std::vector<int>({   -1,   0,   3,   -1,   -1,   4,   7,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, 0, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   0,   -1,   4,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, 0, -1)] = new std::vector<int>({   -1,   3,   -1,   -1,   -1,   7,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, 1, 0)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   0,   3,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, 1, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   -1,   0,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, 1, -1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   3,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, -1, 0)] = new std::vector<int>({   -1,   4,   7,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, -1, 1)] = new std::vector<int>({   -1,   -1,   4,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(1, -1, -1)] = new std::vector<int>({   -1,   7,   -1,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(-1, 0, 0)] = new std::vector<int>({   1,   -1,   -1,   2,   5,   -1,   -1,   6, });
    cubeAdjacencyMap[Glade::Vector3i(-1, 0, 1)] = new std::vector<int>({   -1,   -1,   -1,   1,   -1,   -1,   -1,   5, });
    cubeAdjacencyMap[Glade::Vector3i(-1, 0, -1)] = new std::vector<int>({   2,   -1,   -1,   -1,   6,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(-1, 1, 0)] = new std::vector<int>({   -1,   -1,   -1,   -1,   1,   -1,   -1,   2, });
    cubeAdjacencyMap[Glade::Vector3i(-1, 1, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   -1,   -1,   1, });
    cubeAdjacencyMap[Glade::Vector3i(-1, 1, -1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   2,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(-1, -1, 0)] = new std::vector<int>({   5,   -1,   -1,   6,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(-1, -1, 1)] = new std::vector<int>({   -1,   -1,   -1,   5,   -1,   -1,   -1,   -1, });
    cubeAdjacencyMap[Glade::Vector3i(-1, -1, -1)] = new std::vector<int>({   6,   -1,   -1,   -1,   -1,   -1,   -1,   -1, });
  }

  void addValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float adder)
  {
    auto i = cells.find(cellIndex);
    if (i == cells.end()) {
      log("Warning: trying to add value to nonexistent cell. Will create one");
      i = cells.emplace(cellIndex, Cell(cellIndex, cellSize)).first;
    }

    i->second.val[cubeVertIndex] = std::clamp(i->second.val[cubeVertIndex] + adder, 0.0f, 1.0f);
  }

  void setValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float val)
  {
    auto i = cells.find(cellIndex);
    if (i == cells.end()) {
      log("Warning: trying to set value of nonexistent cell. Will create one");
      i = cells.emplace(cellIndex, Cell(cellIndex, cellSize)).first;
    }

    i->second.val[cubeVertIndex] = std::clamp(val, 0.0f, 1.0f);
  }

  void addValueAtCell(const Glade::Vector3i &centralCellIndex, float adder, int cube_vert_index = -1)
  {
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        for (int z = -1; z <= 1; z++) {
          Glade::Vector3i adjMapIndex(x, y, z);
          Glade::Vector3i cellIndex(centralCellIndex.x + x, centralCellIndex.y + y, centralCellIndex.z + z);
          std::vector<int> *cubeVertIndices = cubeAdjacencyMap[adjMapIndex];

          if (cube_vert_index >= 0) {
            int cubeVertIndex = (*cubeVertIndices)[cube_vert_index];

            if (cubeVertIndex >= 0)
              addValueAtCubeVertex(cellIndex, cubeVertIndex, adder);

            continue;
          }

          for (int i = 0; i < 8; i++) {
            int cubeVertIndex = (*cubeVertIndices)[i];

            if (cubeVertIndex == -1)
              continue;

            addValueAtCubeVertex(cellIndex, cubeVertIndex, adder);
          }
        }
      }
    }
  }

  void setValueAtCell(const Glade::Vector3i &centralCellIndex, float value, int cube_vert_index = -1)
  {
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        for (int z = -1; z <= 1; z++) {
          Glade::Vector3i adjMapIndex(x, y, z);
          Glade::Vector3i cellIndex(centralCellIndex.x + x, centralCellIndex.y + y, centralCellIndex.z + z);
          std::vector<int> *cubeVertIndices = cubeAdjacencyMap[adjMapIndex];

          if (cube_vert_index >= 0) {
            int cubeVertIndex = (*cubeVertIndices)[cube_vert_index];

            if (cubeVertIndex >= 0)
              addValueAtCubeVertex(cellIndex, cubeVertIndex, value);

            continue;
          }

          for (int i = 0; i < 8; i++) {
            int cubeVertIndex = (*cubeVertIndices)[i];

            if (cubeVertIndex == -1)
              continue;

            addValueAtCubeVertex(cellIndex, cubeVertIndex, value);
          }
        }
      }
    }
  }

  void getCubeVertexWorldPositions(const Glade::Vector3i &cellIndex, Glade::Vector3f p[])
  {
    for (int i = 0; i < 8; i++) {
      p[i].x = cellIndex.x * cellSize;
      p[i].y = cellIndex.y * cellSize;
      p[i].z = cellIndex.z * cellSize;
    }

    p[1].x += cellSize;
    p[2].x += cellSize; p[2].z += cellSize;
    p[3].z += cellSize;
    p[4].y += cellSize;
    p[5].y += cellSize; p[5].x += cellSize;
    p[6].y += cellSize; p[6].x += cellSize; p[6].z += cellSize;
    p[7].y += cellSize; p[7].z += cellSize;
  }

  void addValueAtCellPerCubeVertex(const Glade::Vector3i &centralCellIndex, const Glade::Vector3f &hit_point, float adder)
  {
    Glade::Vector3f cubeVertexPositions[8];
    getCubeVertexWorldPositions(centralCellIndex, cubeVertexPositions); // Test?

    typedef struct {
      float distance;
      int cubeVertIndex;
    } CubeVertWithDistance;

    CubeVertWithDistance cubeVerts[8];

    for (int i = 0; i < 8; i++) {
      cubeVerts[i].distance = cubeVertexPositions[i].sqDist(hit_point);
      cubeVerts[i].cubeVertIndex = i;
    }

    std::sort(std::begin(cubeVerts), std::end(cubeVerts), [] (const CubeVertWithDistance &a, CubeVertWithDistance &b) {
        return a.distance > b.distance;
    });

    for (int i = 0; i < 8; i++) {
      float digFactor;
      if (adder > 0)
        digFactor = cubeVerts[7].distance / cubeVerts[i].distance;
      else
        digFactor = cubeVerts[0].distance / cubeVerts[i].distance;

      addValueAtCell(centralCellIndex, adder * digFactor, cubeVerts[i].cubeVertIndex);
    }
  }

  std::pair<Glade::Vector2i, Glade::Vector3i> getCellIndexByCoords(const Glade::Vector3f &coords) const
  {
    int ichunk = coords.x / chunkSizeCoords;
    int jchunk = coords.z / chunkSizeCoords;
    float xchunkstart = ichunk * chunkSizeCoords;
    float zchunkstart = jchunk * chunkSizeCoords;

    Glade::Vector3i cellIndex(coords.x / cellSize, coords.y / cellSize, coords.z / cellSize);

    return std::pair<Glade::Vector2i, Glade::Vector3i>(Glade::Vector2i(ichunk, jchunk), cellIndex);
  }

  void getAdjacentChunks(const Glade::Vector3i &cellIndex, std::vector<Glade::Vector2i> &result)
  {
    result.clear();

    int ichunk = cellIndex.x / chunkSizeCells;
    int jchunk = cellIndex.z / chunkSizeCells;

    int chunkxmod = 0;
    int chunkymod = 0;

    //log("Digged chunk (%d, %d)", ichunk, jchunk);
    //log("Digged cell (%d, %d)", cellIndex.x, cellIndex.z);

    if (cellIndex.x % chunkSizeCells == 0)
      chunkxmod = -1;

    if (cellIndex.x % chunkSizeCells == (chunkSizeCells - 1))
      chunkxmod = +1;

    if (cellIndex.z % chunkSizeCells == 0)
      chunkymod = -1;

    if (cellIndex.z % chunkSizeCells == (chunkSizeCells - 1))
      chunkymod = +1;

    if (chunkxmod)
      result.push_back(Glade::Vector2i(ichunk + chunkxmod, jchunk));

    if (chunkymod)
      result.push_back(Glade::Vector2i(ichunk, jchunk + chunkymod));

    if (chunkxmod && chunkymod)
      result.push_back(Glade::Vector2i(ichunk + chunkxmod, jchunk + chunkymod));
  }

  typedef std::unordered_map<Glade::Vector3i, Cell> Cells;
  typedef Cells::iterator CellsI;
  typedef std::unordered_map<Glade::Vector3i, std::vector<int>*> AdjacencyMap;
  typedef AdjacencyMap::iterator AdjacencyMapI;

  float chunkSizeCoords;
  float cellSize;
  int chunkSizeCells;
  Cells cells;

  AdjacencyMap cubeAdjacencyMap;
};

