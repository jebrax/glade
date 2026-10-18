#pragma once

#include <set>
#include "../exception/GladeException.h"
#include "GraphNode.h"
#include "GraphEdge.h"

class Graph {
public:
  typedef std::set<GraphNode *> GraphNodes;
  typedef std::set<GraphNode *>::iterator GraphNodesI;

private:
  GraphNodes nodes;
  GraphNode::GraphEdges edges;

public:
  ~Graph() {
    for (GraphNodesI inode = nodes.begin(); inode != nodes.end(); ++inode) {
      delete *inode;
    }

    for (GraphNode::GraphEdgesI iedge = edges.begin(); iedge != edges.end(); ++iedge) {
      delete *iedge;
    }
  }

  GraphNodes *getNodes(void) { return &nodes; }
  GraphNode::GraphEdges *getEdges(void) { return &edges; }

  GraphNode *newNode(void) {
    GraphNode *node = new GraphNode();
    nodes.insert(node);

    return node;
  }

  GraphEdge *newEdge(GraphNode *source, GraphNode *destination, float weight) {
    if (!source || !destination || nodes.find(source) == nodes.end() ||
        nodes.find(destination) == nodes.end())
      throw GladeException("There are no such nodes in this graph. Call newNode() first");

    GraphEdge *edge = new GraphEdge(source, destination, weight);
    edges.insert(edge);

    return edge;
  }
};
