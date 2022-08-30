#include <glade/generation/Grid.h>

#include <glade/debug/log.h>

Grid::Grid(int chunkSizeCells, float cellSize): chunkSizeCells(chunkSizeCells), cellSize(cellSize)
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

void Grid::addValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float adder)
{
  auto i = cells.find(cellIndex);
  if (i == cells.end()) {
    log("Warning: trying to add value to nonexistent cell. Will create one");
    i = cells.emplace(cellIndex, Cell(cellIndex, cellSize)).first;
  }

  i->second.val[cubeVertIndex] = std::clamp(i->second.val[cubeVertIndex] + adder, 0.0f, 1.0f);
}

void Grid::setValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float val)
{
  auto i = cells.find(cellIndex);
  if (i == cells.end()) {
    log("Warning: trying to set value of nonexistent cell. Will create one");
    i = cells.emplace(cellIndex, Cell(cellIndex, cellSize)).first;
  }

  i->second.val[cubeVertIndex] = std::clamp(val, 0.0f, 1.0f);
}

void Grid::addValueAtCell(const Glade::Vector3i &centralCellIndex, float adder, int cube_vert_index)
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

void Grid::setValueAtCell(const Glade::Vector3i &centralCellIndex, float value, int cube_vert_index)
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
            setValueAtCubeVertex(cellIndex, cubeVertIndex, value);

          continue;
        }

        for (int i = 0; i < 8; i++) {
          int cubeVertIndex = (*cubeVertIndices)[i];

          if (cubeVertIndex == -1)
            continue;

          setValueAtCubeVertex(cellIndex, cubeVertIndex, value);
        }
      }
    }
  }
}

void Grid::getCubeVertexWorldPositions(const Glade::Vector3i &cellIndex, Glade::Vector3f p[])
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

std::pair<Glade::Vector2i, Glade::Vector3i> Grid::getCellIndexByCoords(const Glade::Vector3f &coords) const
{
  int ichunk = coords.x / chunkSizeCoords;
  int jchunk = coords.z / chunkSizeCoords;
  float xchunkstart = ichunk * chunkSizeCoords;
  float zchunkstart = jchunk * chunkSizeCoords;

  Glade::Vector3i cellIndex(coords.x / cellSize, coords.y / cellSize, coords.z / cellSize);

  return std::pair<Glade::Vector2i, Glade::Vector3i>(Glade::Vector2i(ichunk, jchunk), cellIndex);
}

void Grid::getAdjacentChunks(const Glade::Vector3i &cellIndex, std::vector<Glade::Vector2i> &result)
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

int Grid::coordToCellCoord(float coord) const
{
  return (int) (coord / cellSize);
}

Glade::Vector3i Grid::getCellForPoint(const Glade::Vector3f &point) const
{
  return Glade::Vector3i(coordToCellCoord(point.x), coordToCellCoord(point.y), coordToCellCoord(point.z));
}

bool Grid::doesCubeVertBelongInTheCell(float x, float y, float z, Glade::Vector3i& cellCoord)
{
  std::vector<float> shift = { -cellSize / 2, cellSize / 2 };

  for (float xshift: shift) {
    for (float yshift: shift) {
      for (float zshift: shift) {
        Glade::Vector3f checkingPoint(x + xshift, y + yshift, z + zshift);
        Glade::Vector3i checkingCellCoord = getCellForPoint(checkingPoint);
        if (checkingCellCoord == cellCoord) {
          return true;
        }
      }
    }
  }

  return false;
}

