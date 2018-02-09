#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "graph.h"
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
    struct statusH *status;
    struct gpsH *gps;
} _data;

struct _node
{
    size_t edgeCount;
    struct _data data;
    bool visited;
    double weight;
    struct _node   *parent;
    struct _stack *invalid;
    struct _edge   *edges;
    struct _node   *next;
} _node;

struct _edge
{
    double weight;
    bool visited;
    struct _node   *node;
    struct _edge   *next;
} _edge;

struct _stack
{
    struct _node   *node;
    struct _stack  *next;
} _stack;

// Initializing Static Functions
static void _dijktra(struct _stack *stack, struct _edge *edge, size_t *totalNodes);
static void _validEdge(struct _node *a, struct _node *b);
static bool _notAdjacent(struct _edge *e, struct _node *n);
static void _printNodes(struct _node *n);
static void _printEdges(struct _edge *e);
static void _printBadZerg(struct _stack *s);
static void _printLowHP(struct _node *n, int limit, bool isLow);
static void _addEdge(struct _node *a, struct _node *b, double weight);
static void _addInvalid(struct _stack *s, struct _node *n);
static struct _node *_findNode(struct _node *n, unsigned int id);
static void _setHeavyEdges(struct _edge *e);
static bool _setGPS(struct _node *n,  struct gpsH *gps);
static void _setEdgeVisited(struct _edge *e, struct _node *n);
static bool _setNodeData(struct _node *n, union zergH *zHead, struct gpsH *gps);
static void _resetNodes(struct _node *n, bool full);
static void _resetEdges(struct _edge *e);
static void _removeBadNodes(struct _node *n);
static void _freeStack(struct _stack *s);
static void _disableRoute(struct _node *n);
static void _destroyNodes(struct _node *n);
static void _destroyEdges(struct _edge *e);

// Can be removed
static void _printStack(struct _stack *s);
static void _printFastest(struct _node *n);
static bool _DFS(struct _stack *stack, struct _stack *path, struct _edge *edge, unsigned int id);


// Creating Graph
graph graphCreate(void)
{
    graph g = calloc(1, sizeof(*g));
    if (!g)
    {
        return NULL;
    }

    return g;
}

// Settings all nodes to false
void graphResetNodes(graph g, bool full)
{
    if (!g)
    {
        return;
    }

    _resetNodes(g->nodes, full);
}

int graphAddStatus(graph g, union zergH zHead, struct statusH status)
{
    int err = 0;
    if (!g)
    {
        return err;
    }

    struct _node *found = NULL;

    if(!(found = _findNode(g->nodes, zHead.details.source)))
    {
        graphAddNode(g, zHead, NULL);
        return graphAddStatus(g, zHead, status);
    }

    if (!(found->data.status))
    {
        found->data.status = calloc(1, sizeof(*found->data.status));
        if (!found->data.status)
        {
            return err;
        }
    }
    else
    {
        err = 2;
    }

    *found->data.status = status;

    return err;
}

// Adding a node to the graph
int graphAddNode(graph g, union zergH zHead, struct gpsH *gps)
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
        g->nodes = calloc(1, sizeof(*g->nodes));
        if (!g->nodes)
        {
            return err;
        }
        if(_setNodeData(g->nodes, &zHead, gps))
        {
            printf("Skipping node, out of bounds payload!\n");
            free(g->nodes);
            g->nodes = NULL;
        }
        g->nodes->next = NULL;
        return err;
    }

    // Adding a new node on the chain
    struct _node *curNode = g->nodes;
    struct _node *newNode = _findNode(g->nodes, zHead.details.source);
    bool new = true;

    if(!newNode)
    {
        newNode = calloc(1, sizeof(*newNode));
        if (!newNode)
        {
            return err;
        }  
        if(_setNodeData(newNode, &zHead, gps))
        {
            printf("Skipping node, out of bounds payload!\n");
            free(newNode);
            return err;
        }
        newNode->next = NULL;
    }
    else
    {
        if (newNode->data.gps)
        {
            return 2;
        }

        if(_setGPS(newNode, gps))
        {
            free(newNode);
            return 2;
        }
        new = false;
    }

   _validEdge(newNode, curNode);

    if (newNode->data.zHead.details.source == curNode->data.zHead.details.source && new)
    {
        err = 2;
        return err;
    }

    // Making sure we are at the last node on the chain
    while (curNode->next)
    {   
        if (newNode->data.zHead.details.source == curNode->data.zHead.details.source && new)
        {
            err = 2;
        }
        curNode = curNode->next;
        _validEdge(newNode, curNode);
    }

    if (new)
    {
        curNode->next = newNode;
    }

    return err;
}

static bool _findOnStack(struct _stack *s, unsigned int id)
{
    if (!s || !s->node)
    {
        return false;
    }

    if(s->node->data.zHead.details.source == id)
    {
        return true;
    }

    return _findOnStack(s->next, id);
}

static void _addFromInvalid(struct _node *n, struct _stack *s, size_t *sz)
{
    if (!n || !s || !sz)
    {
        return;
    }

    if (n->invalid && !_findOnStack(s, n->data.zHead.details.source))
    {
        if (!s->node)
        {
            struct _stack *temp = n->invalid->next;
            s->node = n->invalid->node;
            free(n->invalid);
            n->invalid = temp;
            (*sz)++;
        }
        while(n->invalid)
        {
            struct _stack *temp = n->invalid->next;
            _addInvalid(s, n->invalid->node);
            free(n->invalid);
            n->invalid = temp;
            (*sz)++;
        }
    }

    _addFromInvalid( n->next, s, sz);
}

void analyzeMap(graph g, struct _node *n, struct _stack *badZerg, size_t *badZergSz)
{
    if (!g || !n || !badZerg || !badZergSz)
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
    
    graphResetNodes(g, true);
    for (int i = 0; i < 2; i++)
    {
        graphResetNodes(g, false);
        s->node->weight = 0;
        s->node->parent = NULL;
        _dijktra(s, s->node->edges, &totalNodes);
        _disableRoute(n);
        //_printFastest(n);

        if (!n->parent && (_notAdjacent(g->nodes->edges, n) || (totalNodes > 2 && n->edgeCount < 2)))
        {
            badZerg->next = calloc(1, sizeof(*badZerg));
            if (!badZerg->next)
            {
                return;
            }
            (*badZergSz)++;
            badZerg->node = n;
            badZerg->next->next = NULL;
            badZerg->next->node = NULL;

            break;
        }
    }

    free(s);

    if (!n->parent && _notAdjacent(g->nodes->edges, n))
    {
        analyzeMap(g, n->next, badZerg->next, badZergSz);
        return;
    }

    analyzeMap(g, n->next, badZerg, badZergSz);
}

void graphPrintBadZerg(graph g)
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
    badZerg->node = NULL;
    badZerg->next = NULL;

    size_t badZergSz = 0;

    // _printNodes(g->nodes);
    printf("\n");

    analyzeMap(g, g->nodes->next, badZerg, &badZergSz);
    _addFromInvalid(g->nodes, badZerg, &badZergSz);
    
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

void graphPrintLowHP(graph g, int limit)
{
    printf("\n");
    _printLowHP(g->nodes, limit, false);
}

void graphRemoveBadNodes(graph g)
{
    if (!g || !g->nodes)
    {
        return;
    }

    _removeBadNodes(g->nodes);
    if (!g->nodes->data.gps)
    {
        struct _node *freeMe = g->nodes;
        g->nodes = g->nodes->next;

        _destroyNodes(freeMe);
    }
}

// Destroying the graph
void graphDestroy(graph g)
{
    if (!g)
    {
        return;
    }

    _destroyNodes(g->nodes);
    free(g);
}

static void _removeBadNodes(struct _node *n)
{
    if (!n || !n->next)
    {
        return;
    }

    _removeBadNodes(n->next);

    if (!n->next->data.gps)
    {
        struct _node *freeMe = n->next;
        n->next = n->next->next;

        freeMe->next = NULL;
        _destroyNodes(freeMe);
    }
}

static void _printLowHP(struct _node *n, int limit, bool isLow)
{
    if (!n)
    {
        return;
    }

    if (!n->data.status || ((((float) n->data.status->hp / n->data.status->maxHp) * 100) <= limit))
    {
        if (!isLow)
        {
            isLow = true;
            printf("LOW HEALTH (%%%d):\n", limit);
        }
        printf("Zerg #%u\n", n->data.zHead.details.source);
    }

    _printLowHP(n->next, limit, isLow);
}

static bool _setGPS(struct _node *n,  struct gpsH *gps)
{
    if (!n)
    {
        return true;
    }

    gps->altitude = gps->altitude * 1.8288;

    if (gps->latitude > 90.0 || gps->latitude < -90.0)
    {
        return true;
    }
    else if (gps->longitude > 180.0 || gps->longitude < -180.0)
    {
        return true;
    }
    else if (gps->altitude > 11265.4 ||  gps->altitude < -11265.4)
    {
        return true;
    }

    n->data.gps = calloc(1, sizeof(*n->data.gps));
    if (!n->data.gps)
    {
        return true;
    }

    *n->data.gps = *gps;

    return false;
}

static bool _setNodeData(struct _node *n, union zergH *zHead, struct gpsH *gps)
{
    if (!n)
    {
        return false;
    }

    n->data.gps = NULL;
    n->data.status = NULL;

    if (gps)
    {

        if(_setGPS(n, gps))
        {
            return true;
        }
    }

    n->data.zHead = *zHead;
    n->edgeCount = 0;
    n->visited = false;
    n->weight = INITWEIGHT;
    n->parent = NULL;
    n->invalid = NULL;

    return false;
}

// Find a node based of it's x and y
static struct _node *_findNode(struct _node *n, unsigned int id)
{
    if (!n)
    {
        return NULL;
    }

    if (n->data.zHead.details.source == id)
    {
        return n;
    }

    return _findNode(n->next, id);
}

static void _printStack(struct _stack *s)
{
    if (!s)
    {
        return;
    }

    printf("(%u)\n", s->node->data.zHead.details.source);

    _printStack(s->next);
}

static void _disableRoute(struct _node *n)
{
    if(!n)
    {
        return;
    }

    if(n->parent)
    {

        _setEdgeVisited(n->parent->edges, n);
    }

    if(n->parent && n->parent->parent)
    {
       n->parent->visited = true;
    }

    _disableRoute(n->parent);
}

static bool _notAdjacent(struct _edge *e, struct _node *n)
{
    if (!e || !n)
    {
        return true;
    }

    if (e->node->data.zHead.details.source == n->data.zHead.details.source)
    {
        return false;
    }

    return _notAdjacent(e->next, n);
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

static void _setEdgeVisited(struct _edge *e, struct _node *n)
{
    if(!e || !n)
    {
        return;
    }

    if (e->node->data.zHead.details.source == n->data.zHead.details.source)
    {
        e->visited = true;
        return;
    }

    _setEdgeVisited(e->next, n);
}

static void _printFastest(struct _node *n)
{
    if(!n)
    {
        return;
    }

    printf("%u[%.4lf]", n->data.zHead.details.source, n->weight);
    if(n->parent)
    {
       printf(" -> "); 
    }
    else
    {
        printf("\n");
    }

    _printFastest(n->parent);
}

static void _printNodes(struct _node *n)
{
    if(!n)
    {
        return;
    }

    printf("%u[%zu]   \t", n->data.zHead.details.source,  n->edgeCount);

    _printEdges(n->edges);

    _printNodes(n->next);
}

static void _printEdges(struct _edge *e)
{
    if(!e)
    {
        printf("\n");
        return;
    }

    printf("%u[%.2lf]\t", e->node->data.zHead.details.source, e->weight);

    _printEdges(e->next);
}

static void _addInvalid(struct _stack *s, struct _node *n)
{
    if (!s || !n)
    {
        return;
    }

    if (!s->next)
    {
        s->next = calloc(1, sizeof(*s->next));
        if (!s->next)
        {
            return;
        }
        s->next->node = n;
        s->next->next = NULL;
        return;
    }

    _addInvalid(s->next, n);
}

static void _validEdge(struct _node *a, struct _node *b)
{
    if (!a || !b || !a->data.gps || !b->data.gps)
    {
        return;
    }

    double altDiff = a->data.gps->altitude - b->data.gps->altitude;

    if (altDiff > 15.0000)
    {
        return;
    }

    double trueDist = sqrt(pow(dist(a->data.gps, b->data.gps), 2) + pow(altDiff, 2));

    if (trueDist > 15.0000)
    {
        return;
    }
    else if(trueDist <= 1.1430)
    {
        if (!a->invalid)
        {
            a->invalid = calloc(1, sizeof(*a->invalid));
            if (!a->invalid)
            {
                return;
            }
            a->invalid->node = b;
            a->invalid->next = NULL;
        }
        else
        {
            _addInvalid(a->invalid, b);
        }
        return;
    }
    _addEdge(a, b, trueDist);
    _addEdge(b, a, trueDist);
}

static void _setHeavyEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    if (e->node->edgeCount > 2)
    {
        e->weight = INITWEIGHT/100;
    }

    _setHeavyEdges(e->next);
}

// Adding an edge to the graph
static void _addEdge(struct _node *a, struct _node *b, double weight)
{
    if (!a || !b)
    {
        return;
    }

    struct _edge *newEdge = calloc(1, sizeof(*newEdge));
    if (!newEdge)
    {
        return;
    }

    a->edgeCount++;

    // Setting weight based off the node value
    newEdge->node = b;
    newEdge->weight = weight;
    newEdge->visited = false;

    if (!a->edges)
    {
        a->edges = newEdge;
        return;
    }

    struct _edge   *curEdge = a->edges;

    // Making sure the edge it set at the end of the edges
    while (curEdge->next)
    {
        if (a->edgeCount > 2 && curEdge->node->edgeCount > 2 && newEdge->weight <= 1.1430)
        {
            curEdge->weight = INITWEIGHT/100;
            _setHeavyEdges(curEdge->node->edges);
        }
        curEdge = curEdge->next;
    }

    if (a->edgeCount > 2 && curEdge->node->edgeCount > 2 && newEdge->weight <= 1.1430)
    {
        curEdge->weight = INITWEIGHT/100;
        _setHeavyEdges(curEdge->node->edges);
    }

    curEdge->next = newEdge;
}

// Freeing a Stack
static void _freeStack(struct _stack *s)
{
    if (!s)
    {
        return;
    }
    _freeStack(s->next);
    free(s);
}

// My Dijkstra algorithm
static void _dijktra(struct _stack *stack, struct _edge *edge, size_t *totalNodes)
{
    if (!edge || !stack || !totalNodes)
    {
        return;
    }
    // Making next stack
    if (stack->next)
    {
        free(stack->next);
        stack->next = NULL;
    }

    if (edge->visited || edge->node->visited)
    {
        _dijktra(stack, edge->next, totalNodes);
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

        _dijktra(stack->next, stack->next->node->edges, totalNodes);

        _dijktra(stack, stack->node->edges, totalNodes);

    }

    // If there is another edge
    if (edge->next)
    {
        // Going to the next edge
        _dijktra(stack, edge->next, totalNodes);
    }
}

// My DFS algorithm
static bool _DFS(struct _stack *stack, struct _stack *path, struct _edge *edge, unsigned int id)
{
    if (!edge || !stack || !path)
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
        if (stack->next->node->data.zHead.details.source == id)
        {
            return true;
        }

        // If there is another edges
        if (_DFS(stack->next, path->next, stack->next->node->edges, id))
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
        if (_DFS(stack, path->next, stack->node->edges, id))
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
            if (_DFS(stack, path, edge->next, id))
            {
                return true;
            }
        }
    }

    return false;

}

// Settings all nodes to false
static void _resetNodes(struct _node *n, bool full)
{
    if (!n)
    {
        return;
    }

    if (full)
    {
        n->visited = false;
        _resetEdges(n->edges);
    }
    n->weight = INITWEIGHT;
    n->parent = NULL;
    _resetNodes(n->next, full);
}

// Settings all edges to false
static void _resetEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    e->visited = false;

    _resetEdges(e->next);
}

// Destroy edges
static void _destroyEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    _destroyEdges(e->next);
    free(e);
}

// Destroy nodes and edges
static void _destroyNodes(struct _node *n)
{
    if (!n)
    {
        return;
    }

    _destroyEdges(n->edges);
    _destroyNodes(n->next);

    if(n->data.status)
    {
        free(n->data.status);
    }
    if(n->data.gps)
    {
        free(n->data.gps);
    }
    if (n->invalid)
    {
        _freeStack(n->invalid);
    }
    free(n);
}
