#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "graph.h"
#include "util.h"

#define INITWEIGHT 1000000
#define HEAVYEDGE 1000

// Initializing Structs
struct _graph
{
    struct _node   *nodes;
    struct _stack *badNodes;
    size_t totalBad;
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

// Dijkstra Algorithm
static void _dijktra(struct _stack *stack, struct _edge *edge, size_t *totalNodes);

// Returning the smallest stack of bad nodes after analyzing a given stack
static struct _stack *_smallestBadStack(graph g, struct _node *n);

// Analyzing the graph
static void _analyzeGraph(graph g, struct _node *start, struct _node *end, struct _stack *badZerg, size_t *badZergSz);

// Verifying if an edge can be made
static void _validEdge(struct _node *a, struct _node *b);

// Checking if the edge and node are adjacent
static bool _notAdjacent(struct _edge *e, struct _node *n);

// Printing bad nodes
static void _printBadNodes(struct _stack *s);

// Printing nodes with low HP
static void _printLowHP(struct _node *n, int limit, bool isLow);

// Adding an edge between nodes
static void _addEdge(struct _node *a, struct _node *b, double weight);



static void _addFromInvalid(struct _node *n, struct _stack **s, size_t *sz);


// Adding a node to the invalid stack
static void _addInvalid(struct _stack *s, struct _node *n);

// Finding and returning a node with the matching id
static struct _node *_findNode(struct _node *n, unsigned int id);


static bool _findOnStack(struct _stack *s, unsigned int id);


// Setting a heavy edge for nodes with 3+ edges
static void _setHeavyEdges(struct _edge *e);

// Setting GPS info
static bool _setGPS(struct _node *n,  struct gpsH *gps);

// Marking the edge as visited
static void _setEdgeVisited(struct _edge *e, struct _node *n);

// Setting the node data
static bool _setNodeData(struct _node *n, union zergH *zHead, struct gpsH *gps);

// Reseting the node data
static void _resetNodes(struct _node *n, bool full);

// Reseting the edge data
static void _resetEdges(struct _edge *e);

// Removing bad nodes
static void _removeBadNodes(struct _node *n);

// Freeing a stack
static void _freeStack(struct _stack *s);

// Disabling a route for Dijkstra
static void _disableRoute(struct _node *n);

// Freeing nodes and all their data
static void _destroyNodes(struct _node *n);

// Freeing Edges
static void _destroyEdges(struct _edge *e);

// Creating and returning a graph
graph graphCreate(void)
{
    graph g = calloc(1, sizeof(*g));
    g->badNodes = NULL;
    g->totalBad = 0;
    g->totalNodes = 0;
    g->totalEdges = 0;
    if (!g)
    {
        return NULL;
    }

    return g;
}

void printEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    printf("%d[%f]\t", e->node->data.zHead.details.source, e->weight);

    printEdges(e->next);
}

void printNodes(struct _node *n)
{
    if (!n)
    {
        return;
    }

    printf("\n");
    printf("%d [%zu]\t", n->data.zHead.details.source, n->edgeCount);
    printEdges(n->edges);

    printNodes(n->next);
}


// Adding a node to the graph
int graphAddNode(graph g, union zergH zHead, struct gpsH *gps)
{
    int err = 0;
    if (!g)
    {
        return err;
    }

    // Adding to the node counter
    g->totalNodes++;

    //If nodes exist make the first one
    if (!g->nodes)
    {
        g->nodes = calloc(1, sizeof(*g->nodes));
        if (!g->nodes)
        {
            return err;
        }

        // Setting node data
        g->nodes->next = NULL;
        if(_setNodeData(g->nodes, &zHead, gps))
        {
            printf("Skipping node, out of bounds payload!\n");
            free(g->nodes);
            g->nodes = NULL;
        }
        return err;
    }

    // Adding a new node on the chain
    struct _node *curNode = g->nodes;
    struct _node *newNode = _findNode(g->nodes, zHead.details.source);
    bool new = true;

    // If the node was not found make a new one
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
    // If the node was found
    else
    {
        // If the node already has gps data, error out
        if (newNode->data.gps)
        {
            return 2;
        }

        // Setting gps data
        if(_setGPS(newNode, gps))
        {
            free(newNode);
            return 2;
        }
        new = false;
    }

    // Adding edges
    _validEdge(newNode, curNode);

    // Making sure the current node doesn't have the same id as the new node
    if (newNode->data.zHead.details.source == curNode->data.zHead.details.source && new)
    {
        err = 2;
        return err;
    }

    // Making sure we are at the last node on the chain
    while (curNode->next)
    {   
        // Making sure the current node doesn't have the same id as the new node
        if (newNode->data.zHead.details.source == curNode->data.zHead.details.source && new)
        {
            err = 2;
        }
        curNode = curNode->next;

        // Adding edges
        _validEdge(newNode, curNode);
    }

    // If it was a new node, add it to the end of the node chain
    if (new)
    {
        curNode->next = newNode;
    }

    return err;
}

// Adding a status to a node
int graphAddStatus(graph g, union zergH zHead, struct statusH status)
{
    int err = 0;
    if (!g)
    {
        return err;
    }

    struct _node *found = NULL;

    // If the node wasn't found
    if(!(found = _findNode(g->nodes, zHead.details.source)))
    {
        // Make a new node
        graphAddNode(g, zHead, NULL);
        return graphAddStatus(g, zHead, status);
    }

    // If a node was found, but there is no status
    if (!(found->data.status))
    {
        // Making the status variable
        found->data.status = calloc(1, sizeof(*found->data.status));
        if (!found->data.status)
        {
            return err;
        }
    }
    // If there already is a status, error out
    else
    {
        err = 2;
    }

    // Adding the status
    *found->data.status = status;

    return err;
}

// Analyzing the map for bad nodes
void graphAnalyzeGraph(graph g)
{
    if (!g || !g->nodes)
    {
        return;
    }

    //printNodes(g->nodes);

    // Making a stack for bad nodes
    // struct _stack  *badNodes = calloc(1, sizeof(*badNodes));
    // if (!badNodes)
    // {
    //     return;
    // }
    // badNodes->node = NULL;
    // badNodes->next = NULL;

    // Size of the stack
    // size_t badSz = 0;

    // Running every invalid item on the stack to get analyzed
    g->badNodes = _smallestBadStack(g, g->nodes);
    // struct _stack *newBadNodes = _smallestBadStack(g, g->nodes);
    // if (newBadNodes)
    // {
    //     _freeStack(badNodes);
    //     g->badNodes = newBadNodes;
    // }

    // // Analyzing the map
    // _addFromInvalid(g->nodes, &badNodes, &badSz);
    // _analyzeGraph(g, g->nodes, g->nodes->next, badNodes, &badSz);

    // printNodes(g->nodes);

    // // Setting sz and stack to the graph
    // g->totalBad = badSz;
    // g->badNodes = badNodes;

    // // If their are items on the stack
    // if (g->totalBad > 0)
    // {
    //     // Running every invalid item on the stack to get analyzed
    //     struct _stack *newBadNodes = _smallestBadStack(g, g->nodes);
    //     if (newBadNodes)
    //     {
    //         _freeStack(badNodes);
    //         g->badNodes = newBadNodes;
    //     }
    // }
}

// Printing bad nodes
void graphPrint(graph g)
{
    if (!g || !g->nodes)
    {
        return;
    } 

    printf("\n");

    // More than half the nodes are bad
    if (g->totalBad > (g->totalNodes/2))
    {
        printf("TOO MANY CHANGES REQUIRED\n");
    }
    // If there is any bad nodes
    else if(g->totalBad > 0)
    {
        printf("Network Alterations:\n");
        _printBadNodes(g->badNodes);
    }
    // All good
    else
    {
        printf("ALL ZERG ARE IN POSITION\n");
    }
}

// Printing low hp nodes
void graphPrintLowHP(graph g, int limit)
{
    if (!g || !g->nodes)
    {
        return;
    }
    printf("\n");
    // Printing low HP Items
    _printLowHP(g->nodes, limit, false);
}

// Removing incomplete nodes
void graphRemoveBadNodes(graph g)
{
    if (!g || !g->nodes)
    {
        return;
    }

    _removeBadNodes(g->nodes);

    // If node has no GPS data, remove it
    if (!g->nodes->data.gps)
    {
        struct _node *freeMe = g->nodes;
        g->nodes = g->nodes->next;
        g->totalNodes--;

        _destroyNodes(freeMe);
    }
}

// Freeing the graph
void graphDestroy(graph g)
{
    if (!g)
    {
        return;
    }

    _destroyNodes(g->nodes);
    _freeStack(g->badNodes);
    free(g);
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

static void _addFromInvalid(struct _node *n, struct _stack **s, size_t *sz)
{
    if (!n || !(*s) || !sz)
    {
        return;
    }

    if (n->invalid && !_findOnStack((*s), n->data.zHead.details.source))
    {
        struct _stack *temp = n->invalid;
        if (!(*s)->node)
        {
            (*s)->node = temp->node;
            temp = temp->next;
            (*sz)++;
        }
        while(temp)
        {
            if (temp->node->invalid && !_findOnStack((*s), temp->node->data.zHead.details.source))
            {
                _addInvalid((*s), temp->node);
                (*sz)++;
            }
            temp = temp->next;
        }
    }

    _addFromInvalid( n->next, s, sz);
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


// Analyzing the graph
static void _analyzeGraph(graph g, struct _node *start, struct _node *end, struct _stack *badZerg, size_t *badZergSz)
{
    if (!g || !start || !end || !badZerg || !badZergSz)
    {
        return;
    }

    // Skipping nodes that are scanning for itself but have invalid items
    if (start->data.zHead.details.source == end->data.zHead.details.source)
    {
        _analyzeGraph(g, start, end->next, badZerg, badZergSz);
        return;
    }

    struct _stack  *s = calloc(1, sizeof(*s));
    if (!s)
    {
        return;
    }

    size_t totalNodes = 1;
    s->node = start;
    
    // Reseting all stats on nodes
    _resetNodes(g->nodes, true);
    for (int i = 0; i < 2; i++)
    {
        // Setting starting node info
        s->node->weight = 0;
        s->node->parent = NULL;

        _dijktra(s, s->node->edges, &totalNodes);

        // Disabling a known fastest path
        _disableRoute(end);
        //_printFastest(end);

        // Adding bad items to the stack
        // if ((!end->parent) && ((_notAdjacent(start->edges, end)) || (totalNodes > 2 && end->edgeCount < 2)) && !_findOnStack(badZerg, end->data.zHead.details.source))
        //((_notAdjacent(s->node->edges, end) && totalNodes != 2) || (totalNodes > 2 && end->edgeCount < 2))
        if ((!end->parent) 
            && ((_notAdjacent(s->node->edges, end) && (totalNodes - *badZergSz > 2))
            || (!_notAdjacent(s->node->edges, end) && (totalNodes - *badZergSz > 2)))
            && !_findOnStack(badZerg, end->data.zHead.details.source))
        {   
            if (!badZerg->node)
            {
                badZerg->next = calloc(1, sizeof(*badZerg));
                if (!badZerg->next)
                {
                    return;
                }
                badZerg->node = end;
                badZerg->next->next = NULL;
                badZerg->next->node = NULL;
            }
            else
            {
                _addInvalid(badZerg, end);
            }
            (*badZergSz)++;

            break;
        }

        // Light Reset on nodes
        _resetNodes(g->nodes, false);
    }

    free(s);

    // Going to the next stack place if an item was added
    if (badZerg->next)
    {
        _analyzeGraph(g, start, end->next, badZerg->next, badZergSz);
        return;
    }

    _analyzeGraph(g, start, end->next, badZerg, badZergSz);
}

// Returning the smallest stack of bad nodes after analyzing a given stack
static struct _stack *_smallestBadStack(graph g, struct _node *n)
{
    if (!g || !n)
    {
        return NULL;
    }

    // Making a stack and size counter
    size_t sz = 0;
    struct _stack  *badS = calloc(1, sizeof(*badS));
    if (!badS)
    {
        return NULL;
    }
    badS->node = NULL;
    badS->next = NULL;

    // Analyzing for bad nodes
    // _printBadNodes(badS);
    _analyzeGraph(g, n, g->nodes, badS, &sz);
    // printf("%zu\n", sz);

    // Saving if the new stack is smaller
    if ((sz < g->totalBad) || (g->totalBad == 0))
    {
        g->totalBad = sz;
    }


    // Recursive
    struct _stack *returnS = _smallestBadStack(g, n->next);

    // Returning current bad stack if it's the smallest
    if (sz == g->totalBad)
    {
        if (returnS)
        {
            _freeStack(returnS);
        }
        return badS;
    }
    // Returning the new stack if that is smaller
    else
    {
        _freeStack(badS);
        return returnS;
    }

    return NULL;
}

// Removing bad nodes
static void _removeBadNodes(struct _node *n)
{
    if (!n || !n->next)
    {
        return;
    }

    _removeBadNodes(n->next);

    // Removing nodes without gps data
    if (!n->next->data.gps)
    {
        struct _node *freeMe = n->next;
        n->next = n->next->next;

        freeMe->next = NULL;
        _destroyNodes(freeMe);
    }
}

// Printing nodes with low HP
static void _printLowHP(struct _node *n, int limit, bool isLow)
{
    if (!n)
    {
        return;
    }

    // If the HP percentage is below or equal to the limit
    if (!n->data.status || ((((float) n->data.status->hp / n->data.status->maxHp) * 100) <= limit))
    {
        // If this is the first low HP item
        if (!isLow)
        {
            isLow = true;
            printf("LOW HEALTH (%%%d):\n", limit);
        }
        printf("Zerg #%u\n", n->data.zHead.details.source);
    }

    _printLowHP(n->next, limit, isLow);
}

// Setting GPS info
static bool _setGPS(struct _node *n,  struct gpsH *gps)
{
    if (!n)
    {
        return true;
    }

    gps->altitude = gps->altitude * 1.8288;

    // GPS bounds checks
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

    // Making the gps variable
    n->data.gps = calloc(1, sizeof(*n->data.gps));
    if (!n->data.gps)
    {
        return true;
    }

    // Adding the gps data
    *n->data.gps = *gps;

    return false;
}

// Setting the node data
static bool _setNodeData(struct _node *n, union zergH *zHead, struct gpsH *gps)
{
    if (!n)
    {
        return false;
    }

    n->data.gps = NULL;
    n->data.status = NULL;

    // If gps data exists, set it
    if (gps)
    {

        if(_setGPS(n, gps))
        {
            return true;
        }
    }

    // Setting base values
    n->data.zHead = *zHead;
    n->edgeCount = 0;
    n->visited = false;
    n->weight = INITWEIGHT;
    n->parent = NULL;
    n->invalid = NULL;

    return false;
}

// Finding and returning a node with the matching id
static struct _node *_findNode(struct _node *n, unsigned int id)
{
    if (!n)
    {
        return NULL;
    }

    // Returning if node has the same id
    if (n->data.zHead.details.source == id)
    {
        return n;
    }

    // Going to the next node on the chain
    return _findNode(n->next, id);
}

// Disabling a route for Dijkstra
static void _disableRoute(struct _node *n)
{
    if(!n)
    {
        return;
    }

    // If the node has a parent
    if(n->parent)
    {

        _setEdgeVisited(n->parent->edges, n);
    }

    // If the node has a parent and grand parent
    if(n->parent && n->parent->parent)
    {
       n->parent->visited = true;
    }

    _disableRoute(n->parent);
}

// Checking if the edge and node are adjacent
static bool _notAdjacent(struct _edge *e, struct _node *n)
{
    if (!e || !n)
    {
        return true;
    }

    // If the edge is connected to the node
    if (e->node->data.zHead.details.source == n->data.zHead.details.source)
    {
        return false;
    }

    return _notAdjacent(e->next, n);
}

// Printing bad nodes
static void _printBadNodes(struct _stack *s)
{
    if (!s || !s->node)
    {
        return;
    }

    printf("Remove zerg #%u\n", s->node->data.zHead.details.source);

    _printBadNodes(s->next);
}

// Marking the edge as visited
static void _setEdgeVisited(struct _edge *e, struct _node *n)
{
    if(!e || !n)
    {
        return;
    }

    // If the edge is connected to the node
    if (e->node->data.zHead.details.source == n->data.zHead.details.source)
    {
        e->visited = true;
        return;
    }

    _setEdgeVisited(e->next, n);
}

// Adding a node to the invalid stack
static void _addInvalid(struct _stack *s, struct _node *n)
{
    if (!s || !n)
    {
        return;
    }

    // Adding the node to the end of the stack
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

// Verifying if an edge can be made
static void _validEdge(struct _node *a, struct _node *b)
{
    if (!a || !b || !a->data.gps || !b->data.gps)
    {
        return;
    }

    // Checking the Altitude Difference
    double altDiff = a->data.gps->altitude - b->data.gps->altitude;

    if (altDiff > 15.0000)
    {
        return;
    }

    // Checking the true distance using Pythagorean theorem
    double trueDist = sqrt(pow(dist(a->data.gps, b->data.gps), 2) + pow(altDiff, 2));

    // If the distance is to long
    if (trueDist > 15.0000)
    {
        return;
    }
    // If the distance is to short add it to invalid
    else if(trueDist <= 1.1430)
    {
        // Setting the invalid item on A
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

        // Setting the invalid item on B
        if(!b->invalid)
        {
            b->invalid = calloc(1, sizeof(*b->invalid));
            if (!b->invalid)
            {
                return;
            }
            b->invalid->node = a;
            b->invalid->next = NULL;
        }
        else
        {
            _addInvalid(b->invalid, a);
        }
        a->visited = true;
        b->visited = true;
        return;
    }

    // Adding edges
    _addEdge(a, b, trueDist);
    _addEdge(b, a, trueDist);
}

// Setting a heavy edge for nodes with 3+ edges
static void _setHeavyEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    // Making the edge wait heavy if it has 3+ edges
    if (e->node->edgeCount > 2)
    {
        e->weight = HEAVYEDGE;
    }

    _setHeavyEdges(e->next);
}

// Adding an edge between nodes
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

    // Tracking edges
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
        // If node a and b have 3+ edges, make them heavy
        if (a->edgeCount > 2 && curEdge->node->edgeCount > 2)
        {
            curEdge->weight = HEAVYEDGE;
            _setHeavyEdges(curEdge->node->edges);
        }
        curEdge = curEdge->next;
    }

    // If node a and b have 3+ edges, make them heavy
    if (a->edgeCount > 2 && curEdge->node->edgeCount > 2)
    {
        curEdge->weight = HEAVYEDGE;
        _setHeavyEdges(curEdge->node->edges);
    }

    curEdge->next = newEdge;
}

// Freeing a stack
static void _freeStack(struct _stack *s)
{
    if (!s)
    {
        return;
    }
    _freeStack(s->next);
    free(s);
}

// Dijkstra Algorithm
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

    // Going to the next item if the edge or node is disabled
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

// Reseting the node data
static void _resetNodes(struct _node *n, bool full)
{
    if (!n)
    {
        return;
    }

    // For a full reset
    if (full)
    {
        if (!n->invalid)
        {
            n->visited = false;
        }
        _resetEdges(n->edges);
    }

    n->weight = INITWEIGHT;
    n->parent = NULL;
    _resetNodes(n->next, full);
}

// Reseting the edge data
static void _resetEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    e->visited = false;

    _resetEdges(e->next);
}

// Freeing Edges
static void _destroyEdges(struct _edge *e)
{
    if (!e)
    {
        return;
    }

    _destroyEdges(e->next);
    free(e);
}

// Freeing nodes and all their data
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
