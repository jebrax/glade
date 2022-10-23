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

Grid::Cell& Grid::getOrCreateCell(const Glade::Vector3i& cellIndex, bool forceCreate)
{
  CellsI i = cells.find(cellIndex);

  if (i == cells.end()) {
    //log("Warning: trying to access a nonexistent cell. Will create one");
    forceCreate = true;
  }

  if (forceCreate) {
    Glade::Vector2i chunkIndex = cellIndexToChunkIndex(cellIndex);
    ChunksI chunki = chunks.find(chunkIndex);
    assert(chunki != chunks.end() && "Tried to access nonexistent terrain chunk");
    GladeObject *chunkEntity = chunki->second;
    Glade::Vector3i relativeCellIndex(relativeCellCoord(cellIndex.x), cellIndex.y, relativeCellCoord(cellIndex.z));

    i = cells.emplace(cellIndex, Cell(relativeCellIndex, cellSize, chunkEntity)).first;
  }

  return i->second;
}

void Grid::addValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float adder)
{
  Cell &cell = getOrCreateCell(cellIndex);
  cell.val[cubeVertIndex] = std::clamp(cell.val[cubeVertIndex] + adder, 0.0f, 1.0f);
}

void Grid::setValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float val)
{
  Cell &cell = getOrCreateCell(cellIndex);
  cell.val[cubeVertIndex] = std::clamp(val, 0.0f, 1.0f);
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

void Grid::getAdjacentChunks(const Glade::Vector3i &cellIndex, std::vector<Glade::Vector2i> &result)
{
  result.clear();

  int ichunk = cellIndex.x / chunkSizeCells;
  int jchunk = cellIndex.z / chunkSizeCells;

  int chunkxmod = 0;
  int chunkymod = 0;

  int xmod = cellIndex.x % chunkSizeCells;
  int zmod = cellIndex.z % chunkSizeCells;

  if (xmod == 0)
    chunkxmod = -1;

  if ((xmod == chunkSizeCells - 1) || (xmod == -1))
    chunkxmod = +1;

  if (zmod == 0)
    chunkymod = -1;

  if ((zmod == chunkSizeCells - 1) || (zmod == -1))
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
  return std::floorf(coord / cellSize);
}

int Grid::cellCoordToChunkCoord(int cellCoord) const
{
  if (cellCoord >= 0) {
    return cellCoord / chunkSizeCells;
  } else {
    return ((cellCoord + 1) / chunkSizeCells) - 1;
  }
}

Glade::Vector2i Grid::cellIndexToChunkIndex(const Glade::Vector3i &cellIndex) const
{
  return Glade::Vector2i(cellCoordToChunkCoord(cellIndex.x), cellCoordToChunkCoord(cellIndex.z));
}

Glade::Vector3i Grid::pointToCellIndex(const Glade::Vector3f &point) const
{
  return Glade::Vector3i(coordToCellCoord(point.x), coordToCellCoord(point.y), coordToCellCoord(point.z));
}

Glade::Vector3f Grid::chunkPoint(const Glade::Vector2i &chunkIndex) const
{
  Glade::Vector3f result;
  result.x = chunkIndex.x * chunkSizeCells * cellSize;
  result.y = 0;
  result.z = chunkIndex.y * chunkSizeCells * cellSize;

  return result;
}

int Grid::relativeCellCoord(int absoluteCellCoord) const
{
  if (absoluteCellCoord >= 0)
    return absoluteCellCoord % chunkSizeCells; 
  else
    return chunkSizeCells + absoluteCellCoord % chunkSizeCells;
}

Glade::Vector3i Grid::absoluteCellIndex(const Glade::Vector3i &relativeCellIndex, const Glade::Vector2i &chunkIndex) const
{
  Glade::Vector3i result;

  result.x = chunkIndex.x * chunkSizeCells + relativeCellIndex.x;
  result.y = relativeCellIndex.y;
  result.z = chunkIndex.y * chunkSizeCells + relativeCellIndex.z;

  return result;
}

Glade::Vector3f Grid::relativeCellPoint(const Glade::Vector3i &relativeCellIndex) const
{
  Glade::Vector3f result;
  result.x = relativeCellIndex.x * cellSize;
  result.y = relativeCellIndex.y * cellSize;
  result.z = relativeCellIndex.z * cellSize;

  return result;
}

Glade::Vector3i Grid::chunkCenterCellIndex(const Glade::Vector2i &chunkIndex) const
{
  Glade::Vector3i result;
  result.x = chunkSizeCells / 2 + chunkIndex.x * chunkSizeCells;
  result.y = CHUNK_HEIGHT / 2;
  result.z = chunkSizeCells / 2 + chunkIndex.y * chunkSizeCells;

  return result;
}

/* // looks like this is a better approach but some points get broken
bool Grid::doesCubeVertBelongInTheCell(Glade::Vector3i &sourceCellIndex, int sourceCubeVertNum, Glade::Vector3i &targetCellIndex)
{
  Glade::Vector3i adjacencyKey(targetCellIndex);
  adjacencyKey.subtract(sourceCellIndex);
  auto adj = cubeAdjacencyMap.find(adjacencyKey);
  
  if (adj == cubeAdjacencyMap.end())
    return false;
  
  auto& adjList = adj->second;
  return (*adjList)[sourceCubeVertNum] >= 0;
}
*/

bool Grid::doesCubeVertBelongInTheCell(float x, float y, float z, Glade::Vector3i& cellCoord)
{
  std::vector<float> shift = { -cellSize / 2, cellSize / 2 };

  for (float xshift: shift) {
    for (float yshift: shift) {
      for (float zshift: shift) {
        Glade::Vector3f checkingPoint(x + xshift, y + yshift, z + zshift);
        Glade::Vector3i checkingCellCoord = pointToCellIndex(checkingPoint);
        if (checkingCellCoord == cellCoord) {
          return true;
        }
      }
    }
  }

  return false;
}

