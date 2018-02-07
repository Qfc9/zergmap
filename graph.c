#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "graph.h"
#include "zergPrint.h"
#include "util.h"

#define INITWEIGHT 1000000

// Initializing Structs
struct _graph
{
    struct _node   *nodes;
    size_t totalNodes;
    size_t totalEdges;
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
    bool active;
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
static struct _node *_graphFind(struct _node *n, unsigned int id);
static bool _graphDFS(struct _stack *stack, struct _stack *path, struct _edge *edge, double endLat, double endLon);
static void _graphFastPath(struct _stack *stack, struct _edge *edge, size_t *totalNodes);
static void _graphResetNodes(struct _node *n);
static void _freeStack(struct _stack *s);
// static void     _graphResetNodes(
//     struct _node *n);
// static struct _node *_graphFind(struct _node *n, struct GPS *gps);

static void printNodes(struct _node *n);
static void printEdges(struct _edge *e);
static void _graphAddEdge(struct _node *a, struct _node *b, double weight);
static void _graphValidEdge(struct _node *a, struct _node *b);
static void printFastest(struct _node *n);
static void setEdgeInactive(struct _edge *e, struct _node *n);

// Creating Graph
graph graphCreate(void)
{
    graph g = calloc(1, sizeof(*g));

    return g;
}

void printStack(struct _stack *s)
{
    if (!s)
    {
        return;
    }
    printf("(%.4lf, %.4lf)\n", s->node->data.gpsInfo.latitude, s->node->data.gpsInfo.longitude);

    printStack(s->next);
}

void removeSingle(struct _node *n)
{
    if (!n)
    {
        return;
    }

    if (n->edgeCount < 2)
    {
        printf("Remove zerg #%u\n", n->data.zHead.details.source);
    }

    removeSingle(n->next);
}

static void setEdges(struct _node *n)
{
    if(!n)
    {
        return;
    }

    if(n->parent)
    {
       setEdgeInactive(n->parent->edges, n);
    }

    setEdges(n->parent);
}

void analyzeMap(graph g, struct _node *n, struct _stack *badZerg, size_t *badZergSz)
{
    if (!n)
    {
        return;
    }

    struct _stack  *s = calloc(1, sizeof(*s));
    if (!s)
    {
        return;
    }

    size_t totalNodes = 1;
    s->node = g->nodes;

    for (int i = 0; i < 2; i++)
    {
        graphResetNodes(g);
        s->node->weight = 0;
        s->node->parent = NULL;
        _graphFastPath(s, n->edges, &totalNodes);
        setEdges(n);

        if (!n->parent)
        {
            badZerg->next = calloc(1, sizeof(*badZerg));
            if (!badZerg->next)
            {
                return;
            }
            (*badZergSz)++;
            badZerg->node = n;

            break;
        }
    }

    free(s);

    if (!n->parent)
    {
        analyzeMap(g, n->next, badZerg->next, badZergSz);
        return;
    }

    analyzeMap(g, n->next, badZerg, badZergSz);
}

static void _printBadZerg(struct _stack *s)
{
    if (!s)
    {
        return;
    }
    else if(!s->node)
    {
        free(s);
        return;
    }

    printf("Remove zerg #%u\n", s->node->data.zHead.details.source);

    _printBadZerg(s->next);
    free(s);
}



void graphPrintNodes(graph g)
{
    if (!g || !g->nodes)
    {
        return;
    }

    struct _stack  *badZerg = calloc(1, sizeof(*badZerg));
    if (!badZerg)
    {
        return;
    }

    size_t badZergSz = 0;

    printNodes(g->nodes);
    printf("\n");

    analyzeMap(g, g->nodes->next, badZerg, &badZergSz);
    
    if (badZergSz > (g->totalNodes/2))
    {
        printf("TOO MANY CHANGES REQUIRED\n");
    }
    else if(badZergSz > 0)
    {
        printf("Network Alterations:\n");
        _printBadZerg(badZerg);
        badZerg = NULL;
    }
    else
    {
        printf("ALL ZERG ARE IN POSITION\n");
    }

    _freeStack(badZerg);
}

static void setEdgeInactive(struct _edge *e, struct _node *n)
{
    if(!e || !n)
    {
        return;
    }

    if ((e->node->data.gpsInfo.latitude - n->data.gpsInfo.latitude) < 0.00001 && 
        (e->node->data.gpsInfo.longitude - n->data.gpsInfo.longitude) < 0.00001)
    {
        e->active = false;
        return;
    }

    setEdgeInactive(e->next, n);
}

static void printFastest(struct _node *n)
{
    if(!n)
    {
        return;
    }

    printf("%u[%.4lf]", n->data.zHead.details.source, n->weight);
    if(n->parent)
    {
       printf(" -> "); 
       setEdgeInactive(n->parent->edges, n);
    }
    else
    {
        printf("\n");
    }

    printFastest(n->parent);
}

static void printNodes(struct _node *n)
{
    if(!n)
    {
        return;
    }

    printf("%u[%f]\t", n->data.zHead.details.source,  n->data.gpsInfo.altitude);
    // printf("(%.4lf, %.4lf)\t", n->data.gpsInfo.latitude, n->data.gpsInfo.longitude);

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

    printf("%u[%.2lf]\t", e->node->data.zHead.details.source, e->weight);

    printEdges(e->next);
}

// Adding a node to the graph
int graphAddNode(graph g, union zergH zHead, struct gpsH gps)
{
    int err = 0;
    if (!g)
    {
        return err;
    }

    g->totalNodes++;

    //If nodes exist make the first one
    if (!g->nodes)
    {
        g->nodes = calloc(1, sizeof(_node));
        if (!g->nodes)
        {
            return err;
        }

        setGPSDMS(&gps.latitude, &g->nodes->data.gps.lat);
        setGPSDMS(&gps.longitude, &g->nodes->data.gps.lon);

        g->nodes->data.gpsInfo = gps;
        g->nodes->data.gpsInfo.altitude = g->nodes->data.gpsInfo.altitude * 1.8288;
        g->nodes->data.zHead = zHead;
        g->nodes->edgeCount = 0;
        g->nodes->visited = false;
        g->nodes->weight = INITWEIGHT;
        g->nodes->parent = NULL;
        return err;
    }

    // Adding a new node on the chain
    struct _node   *newNode = calloc(1, sizeof(_node));

    setGPSDMS(&gps.latitude, &newNode->data.gps.lat);
    setGPSDMS(&gps.longitude, &newNode->data.gps.lon);

    newNode->data.gpsInfo = gps;
    newNode->data.gpsInfo.altitude = newNode->data.gpsInfo.altitude * 1.8288;
    newNode->data.zHead = zHead;
    newNode->edgeCount = 0;
    newNode->visited = false;
    newNode->weight = INITWEIGHT;
    newNode->parent = NULL;

    struct _node   *curNode = g->nodes;

   _graphValidEdge(newNode, curNode);
    if (newNode->data.zHead.details.source == curNode->data.zHead.details.source)
    {
        err = 2;
    }

    // Making sure we are at the last node on the chain
    while (curNode->next)
    {
        if (newNode->data.zHead.details.source == curNode->data.zHead.details.source)
        {
            err = 2;
        }
        curNode = curNode->next;
        _graphValidEdge(newNode, curNode);
    }

    curNode->next = newNode;

    return err;
}

static void _graphValidEdge(struct _node *a, struct _node *b)
{
    if (!a || !b)
    {
        return;
    }

    double altDiff = a->data.gpsInfo.altitude - b->data.gpsInfo.altitude;

    if (altDiff > 15.0000)
    {
        return;
    }

    double trueDist = sqrt(pow(dist(&a->data.gpsInfo, &b->data.gpsInfo), 2) + pow(altDiff, 2));

    if (trueDist > 15.0000)
    {
        return;
    }
    _graphAddEdge(a, b, trueDist);
    _graphAddEdge(b, a, trueDist);
}

// Adding an edge to the graph
static void _graphAddEdge(struct _node *a, struct _node *b, double weight)
{
    if (!a || !b)
    {
        return;
    }

    struct _edge   *newEdge = calloc(1, sizeof(*newEdge));
    if (!newEdge)
    {
        return;
    }

    a->edgeCount++;

    // Setting weight based off the node value
    newEdge->node = b;
    newEdge->weight = weight;
    newEdge->active = true;

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
static struct _node *_graphFind(struct _node *n, unsigned int id)
{
    if (!n)
    {
        return NULL;
    }

    if (n->data.zHead.details.source == id)
    {
        return n;
    }

    return _graphFind(n->next, id);
}


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

// Freeing a Stack
static void
_freeStack(
    struct _stack *s)
{
    if (!s)
    {
        return;
    }
    _freeStack(s->next);
    free(s);
}

// My Dijkstra algorithm
static void _graphFastPath(struct _stack *stack, struct _edge *edge, size_t *totalNodes)
{
    if (!edge || !stack)
    {
        return;
    }
    // Making next stack
    if (stack->next)
    {
        free(stack->next);
        stack->next = NULL;
    }

    if (!edge->active)
    {
        _graphFastPath(stack, edge->next, totalNodes);
        return;
    }

    // If I haven't visited this node
    if (edge->node->weight > (stack->node->weight + edge->weight))
    {
        stack->next = calloc(1, sizeof(_stack));
        if (!stack->next)
        {
            return;
        }

        if((edge->node->weight - INITWEIGHT) >= 0.00)
        {
            (*totalNodes)++;
        }

        // Adding stack data
        edge->node->weight = (stack->node->weight + edge->weight);
        edge->node->parent = stack->node;
        stack->next->node = edge->node;

        _graphFastPath(stack->next, stack->next->node->edges, totalNodes);

        _graphFastPath(stack, stack->node->edges, totalNodes);

    }

    // If there is another edge
    if (edge->next)
    {
        // Going to the next edge
        _graphFastPath(stack, edge->next, totalNodes);
    }
}

// My DFS algorithm
static bool _graphDFS(struct _stack *stack, struct _stack *path, struct _edge *edge, double endLat, double endLon)
{
    if (!edge || !stack)
    {
        return false;
    }

    // Free Path and Stack if there is a next
    if (path->next)
    {
        free(path->next);
        path->next = NULL;
    }
    if (stack->next)
    {
        free(stack->next);
        stack->next = NULL;
    }

    // If I haven't visited this node
    if (edge->node->visited == false)
    {
        // Making next stack
        stack->next = calloc(1, sizeof(_stack));
        if (!stack->next)
        {
            return false;
        }

        // Adding stack data
        stack->next->node = edge->node;
        stack->next->node->visited = true;

        // Making next path
        path->next = calloc(1, sizeof(_stack));
        if (!path->next)
        {
            return false;
        }

        // Adding and comparing path data
        path->next->node = stack->next->node;
        if ((path->next->node->data.gpsInfo.latitude - endLat) < 0.00001 &&
            (path->next->node->data.gpsInfo.longitude - endLon) < 0.000001 )
        {
            return true;
        }

        // If there is another edges
        if (_graphDFS(stack->next, path->next, stack->next->node->edges, endLat, endLon))
        {
            return true;
        }

        // If there isn't a next make one
        if (!path->next)
        {
            path->next = calloc(1, sizeof(_stack));
            if (!path->next)
            {
                free(stack->next);
                stack->next = NULL;
                return false;
            }
        }

        // Going to the next edge on the last stack item
        path->next->node = stack->node;
        if (_graphDFS(stack, path->next, stack->node->edges, endLat, endLon))
        {
            return true;
            if (stack->next)
            {
                free(stack->next);
                stack->next = NULL;
            }
        }

        // Free Path and Stack if there is a next
        if (path->next)
        {
            free(path->next);
            path->next = NULL;
        }
    }
    // If I have visited this node
    else
    {
        // If there is another edge
        if (edge->next)
        {
            // Going to the next edge
            if (_graphDFS(stack, path, edge->next, endLat, endLon))
            {
                return true;
            }
        }
    }

    return false;

}

// Settings all nodes to false
void
graphResetNodes(
    graph g)
{
    if (!g)
    {
        return;
    }

    _graphResetNodes(g->nodes);
}

// Settings all nodes to false
static void
_graphResetNodes(
    struct _node *n)
{
    if (!n)
    {
        return;
    }

    n->visited = false;
    n->weight = INITWEIGHT;
    n->parent = NULL;
    _graphResetNodes(n->next);
}

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
