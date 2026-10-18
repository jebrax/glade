#include <cassert>
#include <glade/generation/Grid.h>

#include <glade/debug/log.h>

Grid::Grid(int chunk_size_cells, float cell_size)
    : chunkSizeCells(chunk_size_cells), cellSize(cell_size) {
  chunkSizeCoords = cell_size * chunk_size_cells;

  cubeAdjacencyMap[Glade::Vector3i(0, 0, 0)] = new std::vector<int>({
      0,
      1,
      2,
      3,
      4,
      5,
      6,
      7,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, 0, 1)] = new std::vector<int>({
      -1,
      -1,
      1,
      0,
      -1,
      -1,
      5,
      4,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, 0, -1)] = new std::vector<int>({
      3,
      2,
      -1,
      -1,
      7,
      6,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, 1, 0)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      0,
      1,
      2,
      3,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, 1, 1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
      1,
      0,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, 1, -1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      3,
      2,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, -1, 0)] = new std::vector<int>({
      4,
      5,
      6,
      7,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, -1, 1)] = new std::vector<int>({
      -1,
      -1,
      5,
      4,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(0, -1, -1)] = new std::vector<int>({
      7,
      6,
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, 0, 0)] = new std::vector<int>({
      -1,
      0,
      3,
      -1,
      -1,
      4,
      7,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, 0, 1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      0,
      -1,
      4,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, 0, -1)] = new std::vector<int>({
      -1,
      3,
      -1,
      -1,
      -1,
      7,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, 1, 0)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      -1,
      0,
      3,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, 1, 1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
      0,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, 1, -1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      -1,
      3,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, -1, 0)] = new std::vector<int>({
      -1,
      4,
      7,
      -1,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, -1, 1)] = new std::vector<int>({
      -1,
      -1,
      4,
      -1,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(1, -1, -1)] = new std::vector<int>({
      -1,
      7,
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, 0, 0)] = new std::vector<int>({
      1,
      -1,
      -1,
      2,
      5,
      -1,
      -1,
      6,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, 0, 1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      1,
      -1,
      -1,
      -1,
      5,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, 0, -1)] = new std::vector<int>({
      2,
      -1,
      -1,
      -1,
      6,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, 1, 0)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      1,
      -1,
      -1,
      2,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, 1, 1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
      1,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, 1, -1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      -1,
      2,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, -1, 0)] = new std::vector<int>({
      5,
      -1,
      -1,
      6,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, -1, 1)] = new std::vector<int>({
      -1,
      -1,
      -1,
      5,
      -1,
      -1,
      -1,
      -1,
  });
  cubeAdjacencyMap[Glade::Vector3i(-1, -1, -1)] = new std::vector<int>({
      6,
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
      -1,
  });

  ::pthread_mutex_init(&mCellsLock, nullptr);
}

Grid::~Grid() { ::pthread_mutex_destroy(&mCellsLock); }

Grid::Cell *Grid::getCell(const Glade::Vector3i &cell_index) {
  ::pthread_mutex_lock(&mCellsLock);

  CellsI i = cells.find(cell_index);

  if (i == cells.end()) {
    ::pthread_mutex_unlock(&mCellsLock);
    return nullptr;
  }

  Cell *result = &i->second;
  ::pthread_mutex_unlock(&mCellsLock);

  return result;
}

void Grid::setCell(const Glade::Vector3i &cell_index, const Grid::Cell &cell) {
  ::pthread_mutex_lock(&mCellsLock);
  cells.insert_or_assign(cell_index, cell);
  ::pthread_mutex_unlock(&mCellsLock);
}

Grid::Cell &Grid::getOrCreateCell(const Glade::Vector3i &cell_index, bool force_create) {
  ::pthread_mutex_lock(&mCellsLock);
  CellsI i = cells.find(cell_index);

  if (i == cells.end() && !force_create) {
    ::log("Warning: trying to access a nonexistent cell. Will create one");
    force_create = true;
  }

  if (force_create) {
    Glade::Vector2i chunk_index = cellIndexToChunkIndex(cell_index);
    ChunksI chunki = chunks.find(chunk_index);
    assert(chunki != chunks.end() && "Tried to access nonexistent terrain chunk");
    GladeObject *chunk_entity = chunki->second;
    Glade::Vector3i relative_cell_index(relativeCellCoord(cell_index.x), cell_index.y,
                                        relativeCellCoord(cell_index.z));

    i = cells.emplace(cell_index, Cell(relative_cell_index, cellSize, chunk_entity)).first;
  }

  Cell &cell = i->second;
  ::pthread_mutex_unlock(&mCellsLock);

  return cell;
}

void Grid::addValueAtCubeVertex(const Glade::Vector3i &cell_index, int cube_vert_index,
                                float adder) {
  Cell &cell = getOrCreateCell(cell_index);
  cell.val[cube_vert_index] = std::clamp(cell.val[cube_vert_index] + adder, 0.0f, 1.0f);
}

void Grid::setValueAtCubeVertex(const Glade::Vector3i &cell_index, int cube_vert_index, float val) {
  Cell &cell = getOrCreateCell(cell_index);
  cell.val[cube_vert_index] = std::clamp(val, 0.0f, 1.0f);
}

void Grid::addValueAtCell(const Glade::Vector3i &central_cell_index, float adder,
                          int cube_vert_index) {
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        Glade::Vector3i adj_map_index(x, y, z);
        Glade::Vector3i cell_index(central_cell_index.x + x, central_cell_index.y + y,
                                   central_cell_index.z + z);
        std::vector<int> *cube_vert_indices = cubeAdjacencyMap[adj_map_index];

        if (cube_vert_index >= 0) {
          int cube_vert_index = (*cube_vert_indices)[cube_vert_index];

          if (cube_vert_index >= 0) addValueAtCubeVertex(cell_index, cube_vert_index, adder);

          continue;
        }

        for (int i = 0; i < 8; i++) {
          int cube_vert_index = (*cube_vert_indices)[i];

          if (cube_vert_index == -1) continue;

          addValueAtCubeVertex(cell_index, cube_vert_index, adder);
        }
      }
    }
  }
}

void Grid::setValueAtCell(const Glade::Vector3i &central_cell_index, float value,
                          int cube_vert_index) {
  for (int x = -1; x <= 1; x++) {
    for (int y = -1; y <= 1; y++) {
      for (int z = -1; z <= 1; z++) {
        Glade::Vector3i adj_map_index(x, y, z);
        Glade::Vector3i cell_index(central_cell_index.x + x, central_cell_index.y + y,
                                   central_cell_index.z + z);
        std::vector<int> *cube_vert_indices = cubeAdjacencyMap[adj_map_index];

        if (cube_vert_index >= 0) {
          int cube_vert_index = (*cube_vert_indices)[cube_vert_index];

          if (cube_vert_index >= 0) setValueAtCubeVertex(cell_index, cube_vert_index, value);

          continue;
        }

        for (int i = 0; i < 8; i++) {
          int cube_vert_index = (*cube_vert_indices)[i];

          if (cube_vert_index == -1) continue;

          setValueAtCubeVertex(cell_index, cube_vert_index, value);
        }
      }
    }
  }
}

void Grid::getCubeVertexWorldPositions(const Glade::Vector3i &cell_index, Glade::Vector3f p[]) {
  for (int i = 0; i < 8; i++) {
    p[i].x = cell_index.x * cellSize;
    p[i].y = cell_index.y * cellSize;
    p[i].z = cell_index.z * cellSize;
  }

  p[1].x += cellSize;
  p[2].x += cellSize;
  p[2].z += cellSize;
  p[3].z += cellSize;
  p[4].y += cellSize;
  p[5].y += cellSize;
  p[5].x += cellSize;
  p[6].y += cellSize;
  p[6].x += cellSize;
  p[6].z += cellSize;
  p[7].y += cellSize;
  p[7].z += cellSize;
}

void Grid::getAdjacentChunks(const Glade::Vector3i &cell_index,
                             std::vector<Glade::Vector2i> &result) {
  result.clear();

  int ichunk = cell_index.x / chunkSizeCells;
  int jchunk = cell_index.z / chunkSizeCells;

  int chunkxmod = 0;
  int chunkymod = 0;

  int xmod = cell_index.x % chunkSizeCells;
  int zmod = cell_index.z % chunkSizeCells;

  if (xmod == 0) chunkxmod = -1;

  if ((xmod == chunkSizeCells - 1) || (xmod == -1)) chunkxmod = +1;

  if (zmod == 0) chunkymod = -1;

  if ((zmod == chunkSizeCells - 1) || (zmod == -1)) chunkymod = +1;

  if (chunkxmod) result.push_back(Glade::Vector2i(ichunk + chunkxmod, jchunk));

  if (chunkymod) result.push_back(Glade::Vector2i(ichunk, jchunk + chunkymod));

  if (chunkxmod && chunkymod)
    result.push_back(Glade::Vector2i(ichunk + chunkxmod, jchunk + chunkymod));
}

int Grid::coordToCellCoord(float coord) const { return std::floorf(coord / cellSize); }

int Grid::cellCoordToChunkCoord(int cell_coord) const {
  if (cell_coord >= 0) {
    return cell_coord / chunkSizeCells;
  } else {
    return ((cell_coord + 1) / chunkSizeCells) - 1;
  }
}

Glade::Vector2i Grid::cellIndexToChunkIndex(const Glade::Vector3i &cell_index) const {
  return Glade::Vector2i(cellCoordToChunkCoord(cell_index.x), cellCoordToChunkCoord(cell_index.z));
}

Glade::Vector3i Grid::pointToCellIndex(const Glade::Vector3f &point) const {
  return Glade::Vector3i(coordToCellCoord(point.x), coordToCellCoord(point.y),
                         coordToCellCoord(point.z));
}

Glade::Vector3f Grid::chunkPoint(const Glade::Vector2i &chunk_index) const {
  Glade::Vector3f result;
  result.x = chunk_index.x * chunkSizeCells * cellSize;
  result.y = 0;
  result.z = chunk_index.y * chunkSizeCells * cellSize;

  return result;
}

int Grid::relativeCellCoord(int absolute_cell_coord) const {
  if (absolute_cell_coord >= 0) return absolute_cell_coord % chunkSizeCells;
  else return chunkSizeCells + absolute_cell_coord % chunkSizeCells;
}

Glade::Vector3i Grid::absoluteCellIndex(const Glade::Vector3i &relative_cell_index,
                                        const Glade::Vector2i &chunk_index) const {
  Glade::Vector3i result;

  result.x = chunk_index.x * chunkSizeCells + relative_cell_index.x;
  result.y = relative_cell_index.y;
  result.z = chunk_index.y * chunkSizeCells + relative_cell_index.z;

  return result;
}

Glade::Vector3f Grid::relativeCellPoint(const Glade::Vector3i &relative_cell_index) const {
  Glade::Vector3f result;
  result.x = relative_cell_index.x * cellSize;
  result.y = relative_cell_index.y * cellSize;
  result.z = relative_cell_index.z * cellSize;

  return result;
}

Glade::Vector3i Grid::chunkCenterCellIndex(const Glade::Vector2i &chunk_index) const {
  Glade::Vector3i result;
  result.x = chunkSizeCells / 2 + chunk_index.x * chunkSizeCells;
  result.y = CHUNK_HEIGHT / 2;
  result.z = chunkSizeCells / 2 + chunk_index.y * chunkSizeCells;

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

bool Grid::doesCubeVertBelongInTheCell(float x, float y, float z, Glade::Vector3i &cell_coord) {
  std::vector<float> shift = {-cellSize / 2, cellSize / 2};

  for (float xshift : shift) {
    for (float yshift : shift) {
      for (float zshift : shift) {
        Glade::Vector3f checking_point(x + xshift, y + yshift, z + zshift);
        Glade::Vector3i checking_cell_coord = pointToCellIndex(checking_point);

        if (checking_cell_coord == cell_coord) {
          return true;
        }
      }
    }
  }

  return false;
}
