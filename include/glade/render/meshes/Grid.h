#pragma once

#include <unordered_map>

struct Grid
{
  struct Cell
  {
    Glade::Vector3f p[8];
    float val[8];
  };

  Grid(int chunkSizeCells, float cellSize): chunkSizeCells(chunkSizeCells), cellSize(cellSize)
  {
    chunkSizeCoords = cellSize * chunkSizeCells;

    cubeAdjancencyMap[Glade::Vector3i(0, 0, 0)] = new std::vector<int>({   0,   1,   2,   3,   4,   5,   6,   7, });
    cubeAdjancencyMap[Glade::Vector3i(0, 0, 1)] = new std::vector<int>({   -1,   -1,   1,   0,   -1,   -1,   5,   4, });
    cubeAdjancencyMap[Glade::Vector3i(0, 0, -1)] = new std::vector<int>({   3,   2,   -1,   -1,   7,   6,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(0, 1, 0)] = new std::vector<int>({   -1,   -1,   -1,   -1,   0,   1,   2,   3, });
    cubeAdjancencyMap[Glade::Vector3i(0, 1, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   -1,   1,   0, });
    cubeAdjancencyMap[Glade::Vector3i(0, 1, -1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   3,   2,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(0, -1, 0)] = new std::vector<int>({   4,   5,   6,   7,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(0, -1, 1)] = new std::vector<int>({   -1,   -1,   5,   4,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(0, -1, -1)] = new std::vector<int>({   7,   6,   -1,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, 0, 0)] = new std::vector<int>({   -1,   0,   3,   -1,   -1,   4,   7,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, 0, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   0,   -1,   4,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, 0, -1)] = new std::vector<int>({   -1,   3,   -1,   -1,   -1,   7,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, 1, 0)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   0,   3,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, 1, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   -1,   0,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, 1, -1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   3,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, -1, 0)] = new std::vector<int>({   -1,   4,   7,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, -1, 1)] = new std::vector<int>({   -1,   -1,   4,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(1, -1, -1)] = new std::vector<int>({   -1,   7,   -1,   -1,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(-1, 0, 0)] = new std::vector<int>({   1,   -1,   -1,   2,   5,   -1,   -1,   6, });
    cubeAdjancencyMap[Glade::Vector3i(-1, 0, 1)] = new std::vector<int>({   -1,   -1,   -1,   1,   -1,   -1,   -1,   5, });
    cubeAdjancencyMap[Glade::Vector3i(-1, 0, -1)] = new std::vector<int>({   2,   -1,   -1,   -1,   6,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(-1, 1, 0)] = new std::vector<int>({   -1,   -1,   -1,   -1,   1,   -1,   -1,   2, });
    cubeAdjancencyMap[Glade::Vector3i(-1, 1, 1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   -1,   -1,   -1,   1, });
    cubeAdjancencyMap[Glade::Vector3i(-1, 1, -1)] = new std::vector<int>({   -1,   -1,   -1,   -1,   2,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(-1, -1, 0)] = new std::vector<int>({   5,   -1,   -1,   6,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(-1, -1, 1)] = new std::vector<int>({   -1,   -1,   -1,   5,   -1,   -1,   -1,   -1, });
    cubeAdjancencyMap[Glade::Vector3i(-1, -1, -1)] = new std::vector<int>({   6,   -1,   -1,   -1,   -1,   -1,   -1,   -1, });
  }

  void addValueAt(const Glade::Vector3i &centralCellIndex, float adder)
  {
    for (int x = -1; x <= 1; x++) {
      for (int y = -1; y <= 1; y++) {
        for (int z = -1; z <= 1; z++) {
          Glade::Vector3i adjMapIndex(x, y, z);
          Glade::Vector3i cellIndex(centralCellIndex.x + x, centralCellIndex.y + y, centralCellIndex.z + z);
          std::vector<int> *cubeVertIndices = cubeAdjancencyMap[adjMapIndex];

          for (int i = 0; i < 8; i++) {
            int cubeVertIndex = (*cubeVertIndices)[i];

            if (cubeVertIndex == -1)
              continue;

            cells[cellIndex].val[cubeVertIndex] += std::clamp(cells[cellIndex].val[cubeVertIndex] + adder, 0.0f, 1.0f);
          }
        }
      }
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
  typedef std::unordered_map<Glade::Vector3i, std::vector<int>*> AdjancencyMap;
  typedef AdjancencyMap::iterator AdjancencyMapI;

  float chunkSizeCoords;
  float cellSize;
  int chunkSizeCells;
  Cells cells;

  AdjancencyMap cubeAdjancencyMap;
};

