#pragma once

#include <stack>
#include "glade/math/Graph.h"

class DijkstraPathFinder {
private:
  std::stack<GraphNode *> route;
  GraphNode::GraphEdges frontierEdges;
  Graph::GraphNodes checkedNodes;
  std::map<GraphNode *, GraphNode *> routeWaypoints;
  std::map<GraphNode *, float> shortestPaths;

public:
  std::stack<GraphNode *> *findPath(Graph *graph, GraphNode *source, GraphNode *destination) {
    if (destination == NULL || source == NULL) {
      return &route;
    }

    frontierEdges.clear();
    checkedNodes.clear();
    shortestPaths.clear();
    routeWaypoints.clear();
    route = std::stack<GraphNode *>();
    shortestPaths.emplace(source, 0.0f);

    if (dijkstra(source, destination)) {
      GraphNode *current = destination;

      while (current != NULL) {
        route.push(current);
        current = routeWaypoints.find(current)->second;
      }

      route.pop();
    }

    return &route;
  }

private:
  bool dijkstra(GraphNode *source, GraphNode *destination) {
    GraphNode *current = source;

    if (current == NULL) {
      return false;
    }

    while (current != destination) {
      checkedNodes.insert(current);

      GraphNode::GraphEdges *ways_in = current->getWaysIn();

      for (GraphNode::GraphEdgesI edge = ways_in->begin(); edge != ways_in->end(); ++edge) {
        if (checkedNodes.find((*edge)->getSource()) != checkedNodes.end()) {
          frontierEdges.erase(*edge);
        }
      }

      GraphNode::GraphEdges *ways_out = current->getWaysOut();

      for (GraphNode::GraphEdgesI edge = ways_out->begin(); edge != ways_out->end(); ++edge) {
        if (checkedNodes.find((*edge)->getDestination()) != checkedNodes.end()) {
          frontierEdges.insert(*edge);
        }
      }

      current = pickNext();

      if (current == NULL) {
        return false;
      }
    }

    return true;
  }

  GraphNode *pickNext() {
    float greedy_number;
    float min_greedy_number, *min_greedy_number_p = NULL;
    GraphNode *result = NULL;

    GraphNode::GraphEdgesI selected_edge, edge = frontierEdges.begin();

    while (edge != frontierEdges.end()) {
      greedy_number = shortestPaths.find((*edge)->getSource())->second + (*edge)->getWeight();

      if (min_greedy_number_p == NULL || greedy_number < min_greedy_number) {
        min_greedy_number = greedy_number;
        min_greedy_number_p = &greedy_number;
        result = (*edge)->getDestination();
        selected_edge = edge;
      }
    }

    if (min_greedy_number_p != NULL) {
      shortestPaths.emplace(result, min_greedy_number);
      routeWaypoints.emplace(result, (*selected_edge)->getSource());
    }

    return result;
  }
};
