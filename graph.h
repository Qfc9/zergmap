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
int graphAddNode(graph g, union zergH zHead, struct gpsH gps);
void graphAddStatus(graph g, union zergH zHead, struct statusH status);
void graphPrintNodes(graph g);
void graphDestroy(graph g);
void graphResetNodes(graph g, bool full);

#endif
