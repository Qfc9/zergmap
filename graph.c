#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
#include "zergPrint.h"
#include "util.h"

#define INITWEIGHT 1000000

// Initializing Structs
struct _graph
{
    struct _node   *nodes;
} _graph;

struct _data
{
    union zergH zHead;
    struct statusH status;
    struct gpsH gpsInfo;
    struct GPS gps;
} _data;

struct _node
{
    size_t edgeCount;
    struct _data data;
    bool visited;
    double weight;
    struct _node   *parent;
    struct _edge   *edges;
    struct _node   *next;
} _node;

struct _edge
{
    double          weight;
    struct _node   *node;
    struct _edge   *next;
} _edge;

struct _stack
{
    struct _node   *node;
    struct _stack  *next;
} _stack;

// Initializing Static Functions
static void     _graphDestoryNodes(
    struct _node *n);
static void     _graphDestoryEdges(
    struct _edge *e);
// static struct _node *_graphFindNode(
//     struct _node *n,
//     char value);
// static bool     _graphDFS(
//     struct _stack *stack,
//     struct _stack *path,
//     struct _edge *edge,
//     char end);
// static void     _graphFastPath(
//     struct _stack *stack,
//     struct _edge *edge);
// static void     _graphResetNodes(
//     struct _node *n);
// static void     _freeStack(
//     struct _stack *s);
// static void     _graphResetNodes(
//     struct _node *n);
// static struct _node *_graphFind(struct _node *n, struct GPS *gps);
// static void     _graphMakePath(
//     struct _stack *s,
//     struct _node *n);
// static void     _graphMakeEdges(
//     graph g,
//     struct _node *curN,
//     struct _node *topN);
// static void     _graphEditMap(
//     struct _stack *s,
//     char **map);
static void printNodes(struct _node *n);
static void printEdges(struct _edge *e);

// Creating Graph
graph
graphCreate(
    void)
{
    graph           g = calloc(1, sizeof(*g));

    return g;
}

void graphPrintNodes(graph g)
{
    printNodes(g->nodes);
}

static void printNodes(struct _node *n)
{
    if(!n)
    {
        return;
    }

    //printf("%u\t", n->data.zHead.details.source);
    printf("(%.4lf, %.4lf)\t", n->data.gpsInfo.latitude, n->data.gpsInfo.longitude);

    printEdges(n->edges);

    printNodes(n->next);
}

static void printEdges(struct _edge *e)
{
    if(!e)
    {
        printf("\n");
        return;
    }

    printf("E[%.1lf] (%.4lf, %.4lf)\t", e->weight, e->node->data.gpsInfo.latitude, e->node->data.gpsInfo.longitude);

    printEdges(e->next);
}

// // Printing the graph
// void
// graphPrint(
//     graph g,
//     char **map,
//     size_t mapSz,
//     char end)
// {
//     if (!g || !g->nodes)
//     {
//         return;
//     }

//     // Making a path stack
//     struct _stack  *path = calloc(1, sizeof(*path));

//     if (!path)
//     {
//         return;
//     }

//     // Setting the first stack item to the end result node
//     path->node = _graphFindNode(g->nodes, end);
//     if (path->node)
//     {
//         // Filling the stack with the result
//         _graphMakePath(path, path->node->parent);
//     }

//     // Editing the map in memory based off the path
//     _graphEditMap(path, map);

//     // Printing the map
//     for (size_t y = 0; y < mapSz; y++)
//     {
//         printf("%s\n", map[y]);
//     }

//     _freeStack(path);
// }

// // CHANGE ERRORS TO PRINT TO STDERR
// bool
// graphPrintPath(
//     graph g,
//     char **data,
//     size_t sz,
//     char start,
//     char end,
//     size_t search)
// {
//     // Initializing Variables
//     bool            exists = false;

//     struct _stack  *s = calloc(1, sizeof(*s));

//     if (!s)
//     {
//         return exists;
//     }

//     struct _stack  *thePath = calloc(1, sizeof(*thePath));

//     if (!thePath)
//     {
//         free(s);
//         return exists;
//     }

//     // Making sure there is a start and end node
//     if (!(s->node = _graphFindNode(g->nodes, start)) ||
//         !_graphFindNode(g->nodes, end))
//     {
//         fprintf(stderr, "The starting or ending position is missing!\n");
//         free(thePath);
//         free(s);
//         return exists;
//     }

//     // Doing DFS
//     if (search == 0)
//     {
//         s->node->visited = true;
//         thePath->node = s->node;

//         exists = _graphDFS(s, thePath, s->node->edges, end);
//     }
//     // Doing Dijkstra
//     else if (search == 1)
//     {
//         s->node->weight = 0;
//         s->node->parent = NULL;

//         _graphFastPath(s, s->node->edges);

//         // Printing the graph
//         graphPrint(g, data, sz, end);
//     }
//     // Doing Both
//     else
//     {
//         if (graphPrintPath(g, data, sz, start, end, 0))
//         {
//             s->node->weight = 0;
//             s->node->parent = NULL;

//             _graphFastPath(s, s->node->edges);
//         }

//         // Printing the graph
//         graphPrint(g, data, sz, end);
//     }

//     _freeStack(thePath);
//     _freeStack(s);

//     graphResetNodes(g);

//     return exists;
// }

// Adding a node to the graph
void graphAddNode(graph g, union zergH zHead, struct gpsH gps)
{
    if (!g)
    {
        return;
    }

    //If nodes exist make the first one
    if (!g->nodes)
    {
        g->nodes = calloc(1, sizeof(_node));
        if (!g->nodes)
        {
            return;
        }

        setGPSDMS(&gps.latitude, &g->nodes->data.gps.lat);
        setGPSDMS(&gps.longitude, &g->nodes->data.gps.lon);

        g->nodes->data.gpsInfo = gps;
        g->nodes->data.zHead = zHead;
        g->nodes->visited = false;
        g->nodes->weight = INITWEIGHT;
        g->nodes->parent = NULL;
        return;
    }

    // Adding a new node on the chain
    struct _node   *newNode = calloc(1, sizeof(_node));

    setGPSDMS(&gps.latitude, &newNode->data.gps.lat);
    setGPSDMS(&gps.longitude, &newNode->data.gps.lon);

    newNode->data.gpsInfo = gps;
    newNode->data.zHead = zHead;
    newNode->visited = false;
    newNode->weight = INITWEIGHT;
    newNode->parent = NULL;

    struct _node   *curNode = g->nodes;

    // MODEIFY TO INCLUDE ALTITUDE!!!!!!!!!
    // CHECK ZERG ID
    if (15 >= dist(&newNode->data.gpsInfo, &curNode->data.gpsInfo))
    {
        graphAddEdge(newNode, curNode);
        graphAddEdge(curNode, newNode);
    }

    // Making sure we are at the last node on the chain
    while (curNode->next)
    {
        curNode = curNode->next;
        // MODEIFY TO INCLUDE ALTITUDE!!!!!!!!!
        if (15 >= dist(&newNode->data.gpsInfo, &curNode->data.gpsInfo))
        {
            graphAddEdge(newNode, curNode);
            graphAddEdge(curNode, newNode);
        }
    }

    curNode->next = newNode;
}
    
// Adding an edge to the graph
void graphAddEdge(struct _node *a, struct _node *b)
{
    if (!a || !b)
    {
        return;
    }

    struct _edge   *newEdge = calloc(1, sizeof(*newEdge));

    // Setting weight based off the node value
    newEdge->node = b;
    newEdge->weight = dist(&a->data.gpsInfo, &b->data.gpsInfo);

    if (!a->edges)
    {
        a->edges = newEdge;
        return;
    }

    struct _edge   *curEdge = a->edges;

    // Making sure the edge it set at the end of the edges
    while (curEdge->next)
    {
        curEdge = curEdge->next;
    }

    curEdge->next = newEdge;

}

// Destroying the graph
void
graphDestroy(
    graph g)
{
    _graphDestoryNodes(g->nodes);

    free(g);
}

// Find a node based of it's x and y
// static struct _node *_graphFind(struct _node *n, struct GPS *gps)
// {
//     if (!n)
//     {
//         return NULL;
//     }

//     if (n->data.gps.lon.degrees == gps->lon.degrees && 
//         n->data.gps.lon.minutes == gps->lon.minutes && 
//         n->data.gps.lon.seconds == gps->lon.seconds && 
//         n->data.gps.lat.degrees == gps->lat.degrees &&
//         n->data.gps.lat.minutes == gps->lat.minutes &&
//         n->data.gps.lat.seconds == gps->lat.seconds)
//     {
//         return n;
//     }

//     return _graphFind(n->next, gps);
// }


// // Adding nodes onto a stack if it has a parent
// static void
// _graphMakePath(
//     struct _stack *s,
//     struct _node *n)
// {
//     if (!n)
//     {
//         return;
//     }

//     s->next = calloc(1, sizeof(*s));
//     if (!s->next)
//     {
//         return;
//     }

//     s->next->node = n;

//     _graphMakePath(s->next, n->parent);
// }

// // Freeing a Stack
// static void
// _freeStack(
//     struct _stack *s)
// {
//     if (!s)
//     {
//         return;
//     }
//     _freeStack(s->next);
//     free(s);
// }

// // My Dijkstra algorithm
// static void
// _graphFastPath(
//     struct _stack *stack,
//     struct _edge *edge)
// {
//     if (!edge || !stack)
//     {
//         return;
//     }

//     // Making next stack
//     if (stack->next)
//     {
//         free(stack->next);
//         stack->next = NULL;
//     }

//     // If I haven't visited this node
//     if (edge->node->weight > (stack->node->weight + edge->weight))
//     {
//         stack->next = calloc(1, sizeof(_stack));
//         if (!stack->next)
//         {
//             return;
//         }

//         // Adding stack data
//         edge->node->weight = (stack->node->weight + edge->weight);
//         edge->node->parent = stack->node;
//         stack->next->node = edge->node;

//         _graphFastPath(stack->next, stack->next->node->edges);

//         _graphFastPath(stack, stack->node->edges);

//     }

//     // If there is another edge
//     if (edge->next)
//     {
//         // Going to the next edge
//         _graphFastPath(stack, edge->next);
//     }
// }

// // My DFS algorithm
// static bool
// _graphDFS(
//     struct _stack *stack,
//     struct _stack *path,
//     struct _edge *edge,
//     char end)
// {
//     if (!edge || !stack)
//     {
//         return false;
//     }

//     // Free Path and Stack if there is a next
//     if (path->next)
//     {
//         free(path->next);
//         path->next = NULL;
//     }
//     if (stack->next)
//     {
//         free(stack->next);
//         stack->next = NULL;
//     }

//     // If I haven't visited this node
//     if (edge->node->visited == false)
//     {
//         // Making next stack
//         stack->next = calloc(1, sizeof(_stack));
//         if (!stack->next)
//         {
//             return false;
//         }

//         // Adding stack data
//         stack->next->node = edge->node;
//         stack->next->node->visited = true;

//         // Making next path
//         path->next = calloc(1, sizeof(_stack));
//         if (!path->next)
//         {
//             return false;
//         }

//         // Adding and comparing path data
//         path->next->node = stack->next->node;
//         if (path->next->node->data.value == end)
//         {
//             return true;
//         }

//         // If there is another edges
//         if (_graphDFS(stack->next, path->next, stack->next->node->edges, end))
//         {
//             return true;
//         }

//         // If there isn't a next make one
//         if (!path->next)
//         {
//             path->next = calloc(1, sizeof(_stack));
//             if (!path->next)
//             {
//                 free(stack->next);
//                 stack->next = NULL;
//                 return false;
//             }
//         }

//         // Going to the next edge on the last stack item
//         path->next->node = stack->node;
//         if (_graphDFS(stack, path->next, stack->node->edges, end))
//         {
//             return true;
//             if (stack->next)
//             {
//                 free(stack->next);
//                 stack->next = NULL;
//             }
//         }

//         // Free Path and Stack if there is a next
//         if (path->next)
//         {
//             free(path->next);
//             path->next = NULL;
//         }
//     }
//     // If I have visited this node
//     else
//     {
//         // If there is another edge
//         if (edge->next)
//         {
//             // Going to the next edge
//             if (_graphDFS(stack, path, edge->next, end))
//             {
//                 return true;
//             }
//         }
//     }

//     return false;

// }

// // Settings all nodes to false
// void
// graphResetNodes(
//     graph g)
// {
//     if (!g)
//     {
//         return;
//     }

//     _graphResetNodes(g->nodes);
// }

// // Settings all nodes to false
// static void
// _graphResetNodes(
//     struct _node *n)
// {
//     if (!n)
//     {
//         return;
//     }

//     n->visited = false;
//     n->weight = INITWEIGHT;
//     n->parent = NULL;
//     _graphResetNodes(n->next);
// }

// // Find a certain node
// static struct _node *
// _graphFindNode(
//     struct _node *n,
//     char value)
// {
//     if (!n)
//     {
//         return NULL;
//     }

//     if (n->data.value == value)
//     {
//         return n;
//     }

//     return _graphFindNode(n->next, value);
// }

// Destroy edges
static void
_graphDestoryEdges(
    struct _edge *e)
{
    if (!e)
    {
        return;
    }

    _graphDestoryEdges(e->next);
    free(e);
}

// Destroy nodes and edges
static void
_graphDestoryNodes(
    struct _node *n)
{
    if (!n)
    {
        return;
    }

    _graphDestoryEdges(n->edges);
    _graphDestoryNodes(n->next);
    free(n);
}
