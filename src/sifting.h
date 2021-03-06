/**
 * @file sifting.h
 * @brief Definition of infrastructure functions that are used to implement
 * sifting-based heuristics.
 * @author Matt Stallmann
 * @date 2009/01/08
 * $Id: sifting.h 2 2011-06-07 19:50:41Z mfms $
 */

#include<stdbool.h>

#ifndef SIFTING_H
#define SIFTING_H

#include"defs.h"
#include"graph.h"

/**
 * @param node This node is placed on its layer in a position that minimizes
 * the total number of crossings
 */
void sift( Nodeptr node );

/**
 * @param edge An edge that has the current maximum number of crossings; for
 * convience so that this does not need to be recalculated, it is assumed
 * that the node is one of its enpoints
 * @param node This node is placed on its layer in a position that minimizes
 * the maximum number of crossings for any edge incident on the layer.
 */
void sift_node_for_edge_crossings( Edgeptr edge, Nodeptr node );

/**
 * @param node This node is placed into a position that minimizes the total
 * stretch of edges incident on its layer; ties are broken by moving the node
 * as far away as possible from its initial position
 */
void sift_node_for_total_stretch(Nodeptr node);

#endif

/*  [Last modified: 2016 05 19 at 20:15:25 GMT] */
