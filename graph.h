#ifndef graph_h
#define graph_h
#define DFS 0
#define FAST 1
#define BOTH 2

#include <stdlib.h>
#include <stdbool.h>

#include "zergHeaders.h"

typedef struct _graph *graph;

// Creating and returning a graph
graph graphCreate(void);

// Adding a node to the graph
int graphAddNode(graph g, union zergH zHead, struct gpsH *gps);

// Adding a status to a node
int graphAddStatus(graph g, union zergH zHead, struct statusH status);

// Analyzing the map for bad nodes
void graphAnalyzeMap(graph g);

// Printing bad nodes
void graphPrint(graph g);

// Printing low hp nodes
void graphPrintLowHP(graph g, int limit);

// Removing incomplete nodes
void graphRemoveBadNodes(graph g);

// Freeing the graph
void graphDestroy(graph g);

#endif
