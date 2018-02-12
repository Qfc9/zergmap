#ifndef graph_h
#define graph_h
#define DFS 0
#define FAST 1
#define BOTH 2

#include <stdlib.h>
#include <stdbool.h>

#include "zergHeaders.h"

typedef struct _graph *graph;

graph graphCreate(void);
int graphAddNode(graph g, union zergH zHead, struct gpsH *gps);
int graphAddStatus(graph g, union zergH zHead, struct statusH status);
void graphAnalyzeMap(graph g);
void graphPrint(graph g);
void graphPrintLowHP(graph g, int limit);
void graphDestroy(graph g);
void graphRemoveBadNodes(graph g);
void graphResetNodes(graph g, bool full);

#endif
