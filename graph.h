#ifndef graph_h
#define graph_h
#define DFS 0
#define FAST 1
#define BOTH 2

#include <stdlib.h>
#include <stdbool.h>

typedef struct _graph *graph;

struct _node;

graph           graphCreate(
    void);
void            graphAddNode(
    graph g,
    size_t x,
    size_t y,
    char value);
void            graphPrint(
    graph g,
    char **map,
    size_t mapSz,
    char end);
void            graphAddEdge(
    graph g,
    size_t sX,
    size_t sY,
    size_t dX,
    size_t dY);
bool            graphPrintPath(
    graph g,
    char **data,
    size_t sz,
    char start,
    char end,
    size_t search);
void            graphDestroy(
    graph g);
void            graphResetNodes(
    graph g);

void            graphAutoEdges(
    graph g);

#endif
