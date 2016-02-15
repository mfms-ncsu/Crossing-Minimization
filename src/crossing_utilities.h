/**
 * @file crossing_utilities.h
 * @brief Definition of functions that are used to count and update crossings
 * locally. Used both by crossings.c and by sifting.c.
 *
 * @author Matt Stallmann
 * @date 2009/05/15
 * $Id: crossing_utilities.h 56 2014-03-13 21:12:01Z mfms $
 */

#include<stdbool.h>

#ifndef CROSSING_UTILITIES_H
#define CROSSING_UTILITIES_H

#include"defs.h"
#include"graph.h"

/**
 * Counts the inversions when an array of edges and updates crossings for the
 * edges and their nodes accordingly [*** this is a side effect ***].
 *
 * @param edge_array an array of edges sorted by their down nodes
 * @param number_of_edges number of edges in the array
 * @param diff indicates whether to increment the crossing counts (+1) or
 * decrement them (-1); the latter is used for updates during sifting.
 *
 * @return the total number of crossings (inversions)
 */
int count_inversions_up( Edgeptr * edge_array, int number_of_edges,
                         int diff );

/**
 * Inserts the edge at starting_index into the already sorted edges with
 * indices 0, ..., starting_index - 1.  Sort is by up_node.  Each inversion
 * either increments (diff=1) or decrements (diff=-1) the number of crossings
 * for the edges involved and their endpoints.
 *
 * @return the total number of crossings (inversions)
 */
int insert_and_count_inversions_up( Edgeptr * edge_array,
                                      int starting_index,
                                      int diff );

/**
 * Counts the inversions when an array of edges and updates crossings for the
 * edges and their nodes accordingly [*** this is a side effect ***].
 *
 * @param edge_array an array of edges sorted by their up nodes
 * @param number_of_edges number of edges in the array
 * @param diff indicates whether to increment the crossing counts (+1) or
 * decrement them (-1); the latter is used for updates during sifting.
 *
 * @return the total number of crossings (inversions)
 */
int count_inversions_down( Edgeptr * edge_array, int number_of_edges,
                           int diff );

/**
 * Inserts the edge at starting_index into the already sorted edges with
 * indices 0, ..., starting_index - 1.  Sort is by down_node.  Each inversion
 * either increments (diff=1) or decrements (diff=-1) the number of crossings
 * for the edges involved and their endpoints.
 *
 * @return the total number of crossings (inversions)
 */
int insert_and_count_inversions_down( Edgeptr * edge_array,
                                      int starting_index,
                                      int diff );

/**
 * Adds edges to an array of edges. Assumes that there is enough space in the
 * array. Similar to strcat()
 * @param edge_array the array to which edges are to be added
 * @param edges_to_add an array of the edges to be added
 * @param num_edges the number of edges to add
 * @param start_pos the starting position at which the edges are to be
 * added.
 */
void add_edges_to_array( Edgeptr * edge_array, Edgeptr * edges_to_add,
                         int num_edges, int start_pos );

#endif

/*  [Last modified: 2014 03 10 at 16:37:54 GMT] */
