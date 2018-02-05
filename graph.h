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
void graphAddNode(graph g, struct gpsH gps);
void graphPrint(graph g,char **map,size_t mapSz,char end);
void graphAddEdge(struct _node *a, struct _node *b);
bool raphPrintPath(graph g,char **data,size_t sz,char start,char end,size_t search);
void graphDestroy(graph g);
void graphResetNodes(graph g);
void graphAutoEdges(graph g);

#endif
