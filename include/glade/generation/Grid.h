#pragma once

#include <glade/math/Vector.h>

#include <unordered_map>
#include <vector>

class GladeObject;

struct Grid
{
  static const int CHUNK_HEIGHT = 60;

  struct Cell
  {
    Cell(const Glade::Vector3i& relativeCellIndex, float cellSize, GladeObject *chunkEntity):
      startingVertexIndex(-1),
      numVertices(-1),
      chunkEntity(chunkEntity)
    {
      float x = relativeCellIndex.x * cellSize;
      float y = relativeCellIndex.y * cellSize;
      float z = relativeCellIndex.z * cellSize;

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

  typedef std::unordered_map<Glade::Vector3i, Cell> Cells;
  typedef Cells::iterator CellsI;
  typedef std::unordered_map<Glade::Vector3i, std::vector<int>*> AdjacencyMap;
  typedef AdjacencyMap::iterator AdjacencyMapI;

  typedef std::unordered_map<Glade::Vector2i, GladeObject*> Chunks;
  typedef Chunks::iterator ChunksI;

  Grid(int chunkSizeCells, float cellSize);

  Cell& getOrCreateCell(const Glade::Vector3i& cellIndex, bool forceCreate = false);
  void addValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float adder);
  void setValueAtCubeVertex(const Glade::Vector3i &cellIndex, int cubeVertIndex, float val);
  void addValueAtCell(const Glade::Vector3i &centralCellIndex, float adder, int cube_vert_index = -1);
  void setValueAtCell(const Glade::Vector3i &centralCellIndex, float value, int cube_vert_index = -1);

  // conversion functions
  int coordToCellCoord(float coord) const;
  int cellCoordToChunkCoord(int cellCoord) const;
  Glade::Vector3i pointToCellIndex(const Glade::Vector3f &point) const;
  Glade::Vector2i cellIndexToChunkIndex(const Glade::Vector3i &cellIndex) const;
  Glade::Vector3f chunkPoint(const Glade::Vector2i &chunkIndex) const;
  int relativeCellCoord(int absoluteCellCoord) const;
  Glade::Vector3i absoluteCellIndex(const Glade::Vector3i &relativeCellIndex, const Glade::Vector2i &chunkIndex) const;
  Glade::Vector3f relativeCellPoint(const Glade::Vector3i &relativeCellIndex) const;
  Glade::Vector3i chunkCenterCellIndex(const Glade::Vector2i &chunkIndex) const;

  void getAdjacentChunks(const Glade::Vector3i &cellIndex, std::vector<Glade::Vector2i> &result);
  void getCubeVertexWorldPositions(const Glade::Vector3i &cellIndex, Glade::Vector3f p[]);
  bool doesCubeVertBelongInTheCell(float x, float y, float z, Glade::Vector3i& cellCoord);

  void addChunk(int i, int j, GladeObject* obj) {
    return addChunk(Glade::Vector2i(i, j), obj);
  }

  void addChunk(const Glade::Vector2i &chunkIndex, GladeObject* obj) {
    chunks.emplace(chunkIndex, obj);
  }

  GladeObject* getChunk(int i, int j) {
    return getChunk(Glade::Vector2i(i, j));
  }

  GladeObject* getChunk(const Glade::Vector2i &chunkIndex) {
    auto chunki = chunks.find(chunkIndex);

    if (chunki == chunks.end())
      return nullptr;

    return chunki->second;
  }

  void clear() {
    cells.clear();
    chunks.clear();
  }

  void walkChunks(const std::function<void(ChunksI &chunki)> &callback) {
    for (ChunksI chunki = chunks.begin(); chunki != chunks.end(); chunki++)
      callback(chunki);
  }

  float chunkSizeCoords;
  float cellSize;
  int chunkSizeCells;
  Cells cells;
  Chunks chunks;

  AdjacencyMap cubeAdjacencyMap;
};

