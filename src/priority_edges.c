/**
 * @file priority_edges.c
 *
 * @brief Implementation of unctions for setting up priority edges and
 * retrieving the number of crossings involving them.
 *
 * @author Matthias Stallmann
 * @date April, 2011
 * $Id: priority_edges.c 2 2011-06-07 19:50:41Z mfms $
 */

#include"graph.h"
#include"defs.h"
#include"crossings.h"
#include"heuristics.h"          /* clearFixedNodes() */
#include"priority_edges.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<string.h>

static Edgeptr * priority_edge_list;
static int number_of_priority_edges;
static int priority_edge_list_capacity;

void initPriorityEdges( void )
{
  priority_edge_list = (Edgeptr *) malloc( CAPACITY_INCREMENT * sizeof(Edgeptr) );
  number_of_priority_edges = 0;
  priority_edge_list_capacity = CAPACITY_INCREMENT;
}

void freePriorityEdges( void )
{
  free( priority_edge_list );
  priority_edge_list = NULL;
  number_of_priority_edges = 0;
}

void addToPriorityEdges( Edgeptr edge )
{
  if ( number_of_priority_edges >= priority_edge_list_capacity ) {
    priority_edge_list_capacity += CAPACITY_INCREMENT;
    priority_edge_list = realloc( priority_edge_list,
                                  priority_edge_list_capacity * sizeof(Edgeptr) );
  }
  priority_edge_list[ number_of_priority_edges++ ] = edge;
} 

int numberOfFavoredEdges( void )
{
  return number_of_priority_edges;
}

const Edgeptr * favoredEdges( void )
{
  return priority_edge_list;
}

int priorityEdgeCrossings( void )
{
  int total_crossings = 0;
  for ( int i = 0; i < number_of_priority_edges; i++ )
    total_crossings += priority_edge_list[ i ]->crossings;
  return total_crossings;
}

/**
 * @brief Adds edges along all paths going upward from the node to the
 * priority list.
 */
static void upDFS( Nodeptr node ) {
  for ( int i = 0; i < node->up_degree; i++ ) {
    Edgeptr current_edge = node->up_edges[i];
    Nodeptr current_node = current_edge->up_node;
    if ( ! current_node->fixed ) {
      current_node->fixed = true;
      addToPriorityEdges( current_edge );
      upDFS( current_node );
    }
  }
}

/**
 * @brief Adds edges along all paths going downward from the node to the
 * priority list.
 */
static void downDFS( Nodeptr node ) {
  for ( int i = 0; i < node->down_degree; i++ ) {
    Edgeptr current_edge = node->down_edges[i];
    Nodeptr current_node = current_edge->down_node;
    if ( ! current_node->fixed ) {
      current_node->fixed = true;
      addToPriorityEdges( current_edge );
      downDFS( current_node );
    }
  }
}

/* Caution: uses the 'fixed' field of nodes that are encountered during the
 * search as markers so should not be used in the middle of an mcn or mce
 * heuristic */
void createFanoutList( Nodeptr node )
{
  clearFixedNodes();
  upDFS( node );
  downDFS( node );
  clearFixedNodes();
}

void createFavoredEdgeInfo( 
                            char * file_name_buffer,
                            char * graph_name_buffer,
                            char * comment_buffer
                            )
{
  createDotFileName( file_name_buffer, "favored_edges" );
  strcpy( graph_name_buffer, graph_name );
  strcat( graph_name_buffer, "_" );
  /**
   * @todo put more information into the graph name and/or comments; also,
   * most graph names have characters such as - and + in them; these must be
   * converted to _'s
   */
  strcat( graph_name_buffer, "favored_edges" );
  strcpy( comment_buffer,
          "Favored edges created as "
          "ancestors and descendants of a central node" );
}

/*  [Last modified: 2011 04 29 at 19:39:15 GMT] */
