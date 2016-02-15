/**
 * @file priority_edges.h
 * @brief
 * Functions for setting up priority edges and retrieving the number of
 * crossings involving them.
 *
 * @author Matthias Stallmann
 * @date April, 2011
 * $Id: priority_edges.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef PRIORITY_EDGES_H
#define PRIORITY_EDGES_H

/**
 * Initializes list of priority edges
 */
void initPriorityEdges( void );

/**
 * deallocates priority edge list
 */
void freePriorityEdges( void );

/**
 * @param edge the edge to be added to the list of priority edges
 */
void addToPriorityEdges( Edgeptr edge );

/**
 * @return the number of priority (favored) edges
 */
int numberOfFavoredEdges( void );

/**
 * @return (a pointer to) the array of favored edges
 */
const Edgeptr * favoredEdges( void );

/**
 * @return the number of crossings involving priority edges
 */
int priorityEdgeCrossings( void );

/**
 * @brief Takes all the edges that are accessible via a path from the node
 * and adds them to the priority list.
 */
void createFanoutList( Nodeptr node );

/**
 * @param file_name_buffer storage for the file name for the favored edges
 * @param graph_name_buffer storage for the graph name for the favored edges
 * @param comments in the favored edge file, if any
 */
void createFavoredEdgeInfo( 
                            char * file_name_buffer,
                            char * graph_name_buffer,
                            char * comment_buffer
                            );

#endif

/*  [Last modified: 2011 04 27 at 20:44:09 GMT] */
