#ifndef graph_h
#define graph_h
#define DFS 0
#define FAST 1
#define BOTH 2

#include <stdlib.h>
#include <stdbool.h>

#include "zergHeaders.h"

typedef struct _graph *graph;

struct _node;


void graphPrintNodes(graph g);
graph graphCreate(void);
void graphAddNode(graph g, union zergH zHead,struct gpsH gps);
void graphPrint(graph g,char **map,size_t mapSz,char end);
void graphDestroy(graph g);
void graphResetNodes(graph g);

#endif
