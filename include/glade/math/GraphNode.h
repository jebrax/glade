#pragma once

#include <set>

class GraphEdge;

class GraphNode {
public:
  typedef std::set<GraphEdge *> GraphEdges;
  typedef std::set<GraphEdge *>::iterator GraphEdgesI;

private:
  GraphEdges waysOut;
  GraphEdges waysIn;

public:
  GraphEdges *getWaysOut(void) { return &waysOut; }
  GraphEdges *getWaysIn(void) { return &waysIn; }
};
