/**
 * @file LayeredGraph.h
 * @brief Functions that allow creation and manipulation of a layered graph.
 * @author Matt Stallmann
 * @date 2008/07/17
 * $Id: LayeredGraph.h 9 2011-06-13 01:29:18Z mfms $
 */

#ifndef LAYEREDGRAPH_H
#define LAYEREDGRAPH_H

typedef struct node * Node;

typedef struct edge
{
  Node from;
  Node to;
} Edge;

typedef struct graph * Graph;

/**
 * Creates the graph.
 * @param number_of_layers The number of layers.
 * @param name Name used when printing this graph or for other
 * identification.
 * @param how_created Command used to create the graph. Used when comments
 * are embedded in the .dot and .ord files to allow reproducibility.
 * @return the graph
 */
Graph createGraph( int number_of_layers,
                   const char * name, const char * how_created );

/**
 * Deallocates whatever may have been allocated during creation and
 * modification of the graph.
 */
void destroyGraph( Graph G );

/**
 * @return number of layers in G
 */
int getNumberOfLayers( Graph G );

/**
 * @return name of the graph
 */
const char * getGraphName( Graph G );

/**
 * @return how the graph was created
 */
const char * getHowCreated( Graph G );

/**
 * Creates a new node and adds it to the graph
 * @param G the graph
 * @param name Name used to externally identify the node in files
 * @param layer The layer to which the node will be assigned
 * @return A unique pointer for the new node
 */
Node addNode( Graph G, const char * name, int layer );

/**
 * @return (a pointer to) the name of the node
 */
const char * getName( Graph G, Node node );

/**
 * @return the layer of the node
 */
int getLayer( Graph G, Node node );

/**
 * @return the number of nodes on the given layer
 */
int getNumberOfNodesOnLayer( Graph G, int layer );

/**
 * @return (a pointer to) an array of all nodes on a given layer
 */
const Node * getNodesOnLayer( Graph G, int layer );

/**
 * Adds an edge to the graph
 * Assumes that the layers of the nodes have been assigned and that they are
 * contiguous.
 */
void addEdge( Graph G, Node v, Node w );

/**
 * @return the number of edges in the graph
 */
int getNumberOfEdges( Graph G );

/**
 * @return (a pointer to) an array containing all the edges of the graph
 */
const Edge * getAllEdges( Graph G );

#endif

/*  [Last modified: 2008 07 28 at 16:50:44 GMT] */
