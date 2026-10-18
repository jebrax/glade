#pragma once

#include "GraphNode.h"

class GraphEdge {
private:
  float weight;
  GraphNode *destination;
  GraphNode *source;

public:
  GraphEdge(GraphNode *source_, GraphNode *destination_, float weight_)
      : weight(weight_), destination(destination_), source(source_) {
    destination_->getWaysIn()->insert(this);
    source_->getWaysOut()->insert(this);
  }

  float getWeight(void) { return weight; }
  GraphNode *getDestination(void) { return destination; }
  GraphNode *getSource(void) { return source; }
};
