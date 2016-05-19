/**
 * @file crossings.h
 * @brief Definition of functions that keep track of and update the number of
 * crossings for each node, each layer, and for the whole graph.
 * @author Matt Stallmann
 * @date 2008/12/23
 * $Id: crossings.h 2 2011-06-07 19:50:41Z mfms $
 */

#include<stdbool.h>

#ifndef CROSSINGS_H
#define CROSSINGS_H

#include"defs.h"
#include"graph.h"

/**
 * Initializes all crossing counts and allocates data structures used for
 * counting crossings.
 */
void initCrossings( void );

/**
 * @return the total number of crossings in the graph
 */
int numberOfCrossings( void );

/**
 * @return the maximum number of crossings for any edge
 */
int maxEdgeCrossings( void );

/**
 * @return the number of crossings for the given layer
 */
int numberOfCrossingsLayer( int layer );

/**
 * @return the number of crossings for the given node
 */
int numberOfCrossingsNode( Nodeptr node );

/**
 * Updates all crossings based on current ordering of nodes on each layer.
 * The position pointers for all nodes are made consistent as well, using
 * updateAllPositions() in the sorting module
 */
void updateAllCrossings( void );

/**
 * Updates crossings after the given layer has been permuted. The position
 * pointers for all nodes on the layer are made consistent as well, using
 * updateNodePositions() in the sorting module
 */
void updateCrossingsForLayer( int layer );

/**
 * Updates crossings between two adjacent layers. Also updates the relevant
 * crossing fields and position fields of nodes for the two layers.
 * @param upper_layer the higher of the two layers; crossings between layers
 * upper_layer - 1 and upper_layer are counted
 */
void updateCrossingsBetweenLayers( int upper_layer );

/**
 * @return The number of an unfixed layer whose incident edges have the
 * largest number of total crossings, or -1 if all layers are fixed.
 */
int maxCrossingsLayer( void );

/**
 * @return A pointer to an unfixed node whose incident edges have the most
 * crossings, or NULL if all nodes are fixed. The layer and position of the
 * node are stored with it. When there are ties, bias is in favor of a node
 * not on the same layer as the most recent node chosen; this is to avoid a
 * lot of repeated sifting on the same layer.
 */
Nodeptr maxCrossingsNode( void );

/**
 * @return A pointer to an unfixed edge with the most crossings, or NULL if
 * all edges are fixed.
 */
Edgeptr maxCrossingsEdge( void );

/**
 * @return A pointer to an edge with the most crossings; ignores the current
 * status of the edge (fixed or not) and has no impact on the state of any
 * heuristic.
 */
Edgeptr maxCrossingsEdgeStatic( void );

/**
 * Prints information about the crossings, mostly for debugging
 */
void printCrossings( void );

#endif

/*  [Last modified: 2016 05 19 at 20:23:37 GMT] */
