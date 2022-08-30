#pragma once

#include <glade/math/Vector.h>

#include <unordered_map>
#include <vector>

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

  Grid(int chunkSizeCells, float cellSize);

  void addValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float adder);
  void setValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float val);
  void addValueAtCell(const Glade::Vector3i &centralCellIndex, float adder, int cube_vert_index = -1);
  void setValueAtCell(const Glade::Vector3i &centralCellIndex, float value, int cube_vert_index = -1);
  void getCubeVertexWorldPositions(const Glade::Vector3i &cellIndex, Glade::Vector3f p[]);
  std::pair<Glade::Vector2i, Glade::Vector3i> getCellIndexByCoords(const Glade::Vector3f &coords) const;
  void getAdjacentChunks(const Glade::Vector3i &cellIndex, std::vector<Glade::Vector2i> &result);
  bool doesCubeVertBelongInTheCell(float x, float y, float z, Glade::Vector3i& cellCoord);
  int coordToCellCoord(float coord) const;
  Glade::Vector3i getCellForPoint(const Glade::Vector3f &point) const;

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

