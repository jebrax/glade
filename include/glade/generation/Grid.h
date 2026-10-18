#pragma once

#include <glade/math/Vector.h>
#include <glade/io/Stream.h>

#include <unordered_map>
#include <vector>
#include <pthread.h>

class GladeObject;

struct Grid {
  static const int CHUNK_HEIGHT = 60;

  struct Cell {
    Cell(const Glade::Vector3i &relative_cell_index, float cell_size, GladeObject *chunk_entity)
        : startingVertexIndex(-1), numVertices(-1), chunkEntity(chunk_entity),
          relativeCellIndex(relative_cell_index), cellSize(cell_size) {
      for (int i = 0; i < 8; ++i) {
        val[i] = 0.6;
      }

      fillRelativePointCoords(cell_size);
    }

    int getNumberOfPointsWithValueLessThan(float threshold) {
      int result = 0;

      for (int i = 0; i < 8; i++) {
        if (val[i] < threshold) result++;
      }

      return result;
    }

    int getNumberOfPointsWithValueMoreThan(float threshold) {
      int result = 0;

      for (int i = 0; i < 8; i++) {
        if (val[i] > threshold) result++;
      }

      return result;
    }

    float sumValues() {
      float result = 0;

      for (int i = 0; i < 8; i++)
        result += val[i];

      return result;
    }

    GladeObject *getChunkEntity() { return chunkEntity; }
    void setChunkEntity(GladeObject *chunk_entity) { this->chunkEntity = chunk_entity; }

    void serialize(Glade::MemoryOutputStream &stream) {
      stream.write(relativeCellIndex);

      for (int i = 0; i < 8; i++)
        stream.write(val[i]);
    }

    void unserialize(Glade::MemoryInputStream &stream) {
      stream.read(relativeCellIndex);

      for (int i = 0; i < 8; i++)
        stream.read(val[i]);

      fillRelativePointCoords(cellSize);
    }

    Glade::Vector3f p[8];
    float val[8];
    int startingVertexIndex, numVertices;

  private:
    void fillRelativePointCoords(float cell_size) {
      float x = relativeCellIndex.x * cell_size;
      float y = relativeCellIndex.y * cell_size;
      float z = relativeCellIndex.z * cell_size;

      p[0] = Glade::Vector3f(x, y, z);
      p[1] = Glade::Vector3f(x + cell_size, y, z),
      p[2] = Glade::Vector3f(x + cell_size, y, z + cell_size),
      p[3] = Glade::Vector3f(x, y, z + cell_size), p[4] = Glade::Vector3f(x, y + cell_size, z),
      p[5] = Glade::Vector3f(x + cell_size, y + cell_size, z),
      p[6] = Glade::Vector3f(x + cell_size, y + cell_size, z + cell_size),
      p[7] = Glade::Vector3f(x, y + cell_size, z + cell_size);
    }

    GladeObject *chunkEntity;
    float cellSize;
    Glade::Vector3i relativeCellIndex;
  };

  typedef std::unordered_map<Glade::Vector3i, Cell> Cells;
  typedef Cells::iterator CellsI;
  typedef std::unordered_map<Glade::Vector3i, std::vector<int> *> AdjacencyMap;
  typedef AdjacencyMap::iterator AdjacencyMapI;

  typedef std::unordered_map<Glade::Vector2i, GladeObject *> Chunks;
  typedef Chunks::iterator ChunksI;

  Grid(int chunk_size_cells, float cell_size);
  ~Grid();
  Cell &getOrCreateCell(const Glade::Vector3i &cell_index, bool force_create = false);
  Cell *getCell(const Glade::Vector3i &cell_index);
  void setCell(const Glade::Vector3i &cell_index, const Grid::Cell &cell);
  void addValueAtCubeVertex(const Glade::Vector3i &cell_index, int cube_vert_index, float adder);
  void setValueAtCubeVertex(const Glade::Vector3i &cell_index, int cube_vert_index, float val);

  void addValueAtCell(const Glade::Vector3i &central_cell_index, float adder,
                      int cube_vert_index = -1);

  void setValueAtCell(const Glade::Vector3i &central_cell_index, float value,
                      int cube_vert_index = -1);

  // conversion functions
  int coordToCellCoord(float coord) const;
  int cellCoordToChunkCoord(int cell_coord) const;
  Glade::Vector3i pointToCellIndex(const Glade::Vector3f &point) const;
  Glade::Vector2i cellIndexToChunkIndex(const Glade::Vector3i &cell_index) const;
  Glade::Vector3f chunkPoint(const Glade::Vector2i &chunk_index) const;
  int relativeCellCoord(int absolute_cell_coord) const;

  Glade::Vector3i absoluteCellIndex(const Glade::Vector3i &relative_cell_index,
                                    const Glade::Vector2i &chunk_index) const;

  Glade::Vector3f relativeCellPoint(const Glade::Vector3i &relative_cell_index) const;
  Glade::Vector3i chunkCenterCellIndex(const Glade::Vector2i &chunk_index) const;
  void getAdjacentChunks(const Glade::Vector3i &cell_index, std::vector<Glade::Vector2i> &result);
  void getCubeVertexWorldPositions(const Glade::Vector3i &cell_index, Glade::Vector3f p[]);
  bool doesCubeVertBelongInTheCell(float x, float y, float z, Glade::Vector3i &cell_coord);
  void addChunk(int i, int j, GladeObject *obj) { return addChunk(Glade::Vector2i(i, j), obj); }

  void addChunk(const Glade::Vector2i &chunk_index, GladeObject *obj) {
    chunks.emplace(chunk_index, obj);
  }

  GladeObject *getChunk(int i, int j) { return getChunk(Glade::Vector2i(i, j)); }

  GladeObject *getChunk(const Glade::Vector2i &chunk_index) {
    auto chunki = chunks.find(chunk_index);

    if (chunki == chunks.end()) return nullptr;

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

private:
  Cells cells;
  Chunks chunks;
  AdjacencyMap cubeAdjacencyMap;

  pthread_mutex_t mCellsLock;
};
