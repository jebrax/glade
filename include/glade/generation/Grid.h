#pragma once

#include <glade/math/Vector.h>

#include <unordered_map>
#include <vector>

class GladeObject;

struct Grid
{
  struct Cell
  {
    Cell(const Glade::Vector3i& cellIndexInsideChunk, float cellSize, GladeObject *chunkEntity):
      startingVertexIndex(-1),
      numVertices(-1),
      chunkEntity(chunkEntity)
    {
      float x = cellIndexInsideChunk.x * cellSize;
      float y = cellIndexInsideChunk.y * cellSize;
      float z = cellIndexInsideChunk.z * cellSize;

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

    int getNumberOfPointsWithValueLessThan(float threshold)
    {
      int result = 0;

      for (int i = 0; i < 8; i++) {
        if (val[i] < threshold)
          result++;
      }

      return result;
    }

    int getNumberOfPointsWithValueMoreThan(float threshold)
    {
      int result = 0;

      for (int i = 0; i < 8; i++) {
        if (val[i] > threshold)
          result++;
      }

      return result;
    }

    float sumValues()
    {
      float result = 0;

      for (int i = 0; i < 8; i++)
        result += val[i];

      return result;
    }

    GladeObject* getChunkEntity() {
      return chunkEntity;
    }

    Glade::Vector3f p[8];
    float val[8];
    int startingVertexIndex, numVertices;

    private:
      GladeObject *chunkEntity;
  };

  Grid(int chunkSizeCells, float cellSize, int gridSizeChunks);

  Cell& getOrCreateCell(const Glade::Vector3i& cellIndex, bool forceCreate = false);
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

  void addChunk(int i, int j, GladeObject* obj) {
    Glade::Vector2i chunkIndex(i, j);
    chunks.emplace(chunkIndex, obj);
  }

  int getGridSizeChunks() {
    return gridSizeChunks;
  }

  typedef std::unordered_map<Glade::Vector3i, Cell> Cells;
  typedef Cells::iterator CellsI;
  typedef std::unordered_map<Glade::Vector3i, std::vector<int>*> AdjacencyMap;
  typedef AdjacencyMap::iterator AdjacencyMapI;

  typedef std::unordered_map<Glade::Vector2i, GladeObject*> Chunks;
  typedef Chunks::iterator ChunksI;

  float chunkSizeCoords;
  float cellSize;
  int chunkSizeCells;
  Cells cells;
  Chunks chunks;

  AdjacencyMap cubeAdjacencyMap;

  private:
    int gridSizeChunks;
};

