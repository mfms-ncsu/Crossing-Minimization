/**
 * @file dfs.h
 * @brief interface for function that assigns weights based on depth-first
 * search
 * @author Matthias Stallmann
 * @date 2008/01/03
 * $Id: dfs.c 2 2011-06-07 19:50:41Z mfms $
 */

#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "dfs.h"

static int preorder_number = 0;

/**
 * Visits the given node, assigns it the next preorder number, and
 * recursively visits all unvisited adjacent nodes; edges to higher-numbered
 * layers are given precedence.
 */
static void dfs_visit( Nodeptr node );

/**
 * Sets all preorder numbers (weights) to -1 (not visited)
 */
static void initialize_dfs_weights( void )
{
  int layer = 0;
  for( ; layer < number_of_layers; layer++ )
    {
      int position = 0;
      for( ; position < layers[ layer ]->number_of_nodes; position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ position ];
          node->weight = -1;
        }
    }
}

/**
 * Visits (does a dfs_visit) the nodes on the next higher layer to which the
 * given node is adjacent.
 */
static void visit_upper_edges( Nodeptr node )
{
  int edge_pos = node->up_degree - 1;
  for( ; edge_pos >= 0; edge_pos-- )
    {
      Nodeptr adjacent_node = node->up_edges[ edge_pos ]->up_node;
      if( adjacent_node->weight == -1 ) dfs_visit( adjacent_node );
    }
}

/**
 * Visits (does a dfs_visit) the nodes on the next lower layer to which the
 * given node is adjacent.
 */
static void visit_lower_edges( Nodeptr node )
{
  int edge_pos = 0;
  for( ; edge_pos < node->down_degree; edge_pos++ )
    {
      Nodeptr adjacent_node = node->down_edges[ edge_pos ]->down_node;
      if( adjacent_node->weight == -1 ) dfs_visit( adjacent_node );
    }
}

/**
 * Visits the given node, assigns it the next preorder number, and
 * recursively visits all unvisited adjacent nodes; edges to higher-numbered
 * layers are given precedence.
 */
static void dfs_visit( Nodeptr node )
{
#ifdef DEBUG
  printf( "| ----> dfs_visit, node = %s\n", node->name );
#endif
  node->weight = preorder_number++;
  visit_upper_edges( node );
  visit_lower_edges( node );
#ifdef DEBUG
  printf( "<- | dfs_visit, node = %s, weight = %3.1f\n",
          node->name, node->weight );
#endif
}

/**
 * Initiates a depth-first search in each connected component using the
 * leftmost node in the lowest layer as the first node of the component.
 * This is the standard outer loop of dfs, and would not be needed if the
 * graph were connected. 
 */
static void dfs( void )
{
  preorder_number = 0;
  int number_of_components = 0;
  int size_of_largest_component = 0;
  // traverse all nodes, starting a new dfs_visit for any node not yet
  // visited
  for( int layer = 0; layer < number_of_layers; layer++ )
    {
      for( int position = 0; position < layers[ layer ]->number_of_nodes; position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ position ];
          if( node->weight == -1 )
            {
              number_of_components++;
              int start_preorder_number = preorder_number;
              dfs_visit( node );
              int end_preorder_number = preorder_number;
              int size_of_current_component
                = end_preorder_number - start_preorder_number;
              if ( size_of_current_component > size_of_largest_component )
                size_of_largest_component = size_of_current_component;
            }
        }
    }
  printf( "dfs done, number_of_components = %d, size_of_largest_component = %d\n",
          number_of_components, size_of_largest_component );
}

void assignDfsWeights( void )
{
  initialize_dfs_weights();
  dfs();
}

/*  [Last modified: 2011 06 04 at 20:19:23 GMT] */
