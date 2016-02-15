/**
 * @file sifting.c
 * @brief Implementation of functions that place a node in a position on its
 * layer that minimizes the number of crossings or minimizes the maximum
 * number of crossings among edges incident on the node.
 *
 * @author Matt Stallmann
 * @date 2009/01/08
 * $Id: sifting.c 64 2014-03-25 20:36:19Z mfms $
 */

#include"graph.h"
#include"defs.h"
#include"crossings.h"
#include"crossing_utilities.h"
#include"sifting.h"
#include"swap.h"
#include"sorting.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>
#include<limits.h>

/**
 * Puts a node into a different position in an array of nodes.
 * @param node The node to be repositioned
 * @param nodes The array of nodes
 * @param after_position The position of the node that 'node' must come
 * after. If this is -1, then the new position is before all of the other
 * nodes.
 */
static void reposition_node( Nodeptr node, Nodeptr * nodes,
                             int after_position );

/**
 * @brief puts the node in a position that minimizes the number of crossings.
 *
 * Basic algorithm is as follows:
 *
 * -# let x be the node to be sifted
 * -# for each node y != x, calculate cr(x,y) and cr(y,x), where cr(a,b) is
 * the number of crossings among edges incident to a and b if a and b are in
 * the given order
 * -# use the cr values to compute diff(x,y) = cr(x,y) - cr(y,x) for each y
 * -# let y_0, ..., y_L be the nodes other than x on this layer
 * -# let prefix(-1) = 0, prefix(i>=0) = prefix(i-1) + diff(x,y_i)
 * -# if prefix(i) is minimum over all i, then x belongs between y_i and
 * y_{i+1}; however, if the minimum is > 0, then x belongs before y_0
 *
 * Note that prefix(i) represents the crossings(i) - crossings(-1), where
 * crossings(i) = the number of crossings that arise when the node is
 * inserted between y_i and y_{i+1}
 */
void sift( Nodeptr node )
{
#ifdef DEBUG
  printf( "-> sift, node = %s, layer = %d, position = %d\n",
          node->name, node->layer, node->position );
#endif
  // create an array containing diff( node, y_i ) for each y_i on the same
  // layer as 'node', assuming y_i is the node in position i of the layer
  int layer_size =  layers[node->layer]->number_of_nodes;
  Nodeptr * nodes = layers[node->layer]->nodes;
  int * diff = (int *) calloc( layer_size, sizeof(int) );
  int i = 0;
  for( i = 0; i < layer_size; i++ ) {
      if ( nodes[i] != node ) {
          diff[i] = node_crossings( nodes[ i ], node )
            - node_crossings( node, nodes[ i ] );
      }
      else {
          diff[i] = 0;
      }
#ifdef DEBUG
      printf( "  sift loop: diff[%d] = %d\n", i, diff[i] );
#endif
   }

  // compute the minimum prefix sum and its position in the diff array
  // bias the decision in favor of maximum distance from the current
  // position; this does consistently better in preliminary experiments,
  // possibly because it's good to cycle through a lot of possible
  // configurations
  int prefix_sum = 0;
  int min_prefix_sum = 0;
  int min_position = -1;
  int max_distance = 0;
  for( i = 0; i < layer_size; i++ ) {
      prefix_sum += diff[i];
      if( prefix_sum < min_prefix_sum 
          || ( prefix_sum == min_prefix_sum
               && abs( i - node->position ) > max_distance ) ) {
          min_prefix_sum = prefix_sum;
          min_position = i;
          max_distance = abs( i - node->position );
      }
  }
  free( diff );

  // if min_position is i, then the node belongs between nodes[i] and
  // nodes[i+1];

#ifdef DEBUG
  printf( "   sift, reposition: min_prefix_sum = %d, old = %d, new = %d\n",
          min_prefix_sum, node->position, min_position );
#endif

  reposition_node( node, nodes, min_position ); 

  // recompute crossings with respect to this layer
  updateCrossingsForLayer( node->layer );
#ifdef DEBUG
  printf( "<- sift, node = %s, layer = %d, position = %d\n",
          node->name, node->layer, node->position );
#endif
}

static void reposition_node( Nodeptr node, Nodeptr * nodes,
                             int after_position )
{
  // There are three cases to consider: if the node should go immediately
  // after its predecessor or after itself, there is nothing to be done; if
  // it should go  immediately after an earlier node, it goes into
  // after_position + 1 and the intervening nodes are shifted right; if it
  // should go after a later node, it goes into after_position and the
  // intervening nodes, including the one it goes after, are shifted left.
  int i = node->position;
  if( after_position < node->position - 1 )
    {
      for( ; i > after_position + 1; i-- )
        {
          nodes[ i ] = nodes[ i - 1 ];
          nodes[ i ]->position = i;
        }
      nodes[ after_position + 1 ] = node;
      node->position = after_position + 1;
    }
  else if( after_position > node->position )
    {
      for( ; i < after_position; i++ )
        {
          nodes[ i ] = nodes[ i + 1 ];
          nodes[ i ]->position = i;
        }
      nodes[ after_position ] = node;
      node->position = after_position;
    }
}

/**
 * Algorithm for sifting (a node x) in order to minimize the maximum number of
 * crossings for any edge with one endpoint on its layer:
 *
 * Move to the left and then back to the right (in this case the prefix sum
 * approach doesn't work).  When x is moved to the right of y, you do
 *  - change_crossings( x, y, -1 ): sort with edges of x followed by edges of
 *    y and subtract 1 from the crossing number of an edge when it's involved
 *    in an inversion   
 *  - change_crossings( y, x, +1 ): sort with edges of y followed by edges of
 *    x and add 1 from the crossing number of an edge when it's involved
 *    in an inversion
 *  - among all the edges of x and y, find the one with maximum number of
 *    crossings and use that number as the 'value' of the current position of
 *    x
 * The calculation of inversions needs to be done both for the upward and the
 * downward edges.       
 */

void sift_node_for_edge_crossings( Edgeptr edge, Nodeptr node )
{
  assert( node == edge->up_node || node == edge->down_node );
#ifdef DEBUG
  printf( "-> sift_node_for_edge_crossings: %s -> %s, %s\n",
          edge->down_node->name, edge->up_node->name, node->name );
#endif
  int layer = node->layer;
  int layer_size = layers[ layer ]->number_of_nodes;
  Nodeptr * nodes_on_layer = layers[ layer ]->nodes;

  // find the position where the maximum edge crossing count achieves its
  // minimum; bias the decision in favor of maximum distance from the current
  // position; same observation applies as with sifting for minimizing
  // overall crossings
  int min_edge_crossing_count = edge->crossings;
  int min_position = node->position;
  int max_distance = 0;

  // begin with a sweep to the left of the current node position
  for ( int i = node->position - 1; i >= 0; i-- )
    {
      int current_edge_crossing_count
        = edge_crossings_after_swap( nodes_on_layer[i], node );
      if ( current_edge_crossing_count < min_edge_crossing_count
           || ( current_edge_crossing_count == min_edge_crossing_count
                && node->position - i > max_distance )
           )
        {
          min_edge_crossing_count = current_edge_crossing_count;
          min_position = i - 1;
          max_distance = node->position - i + 1;
        }
#ifdef DEBUG
      printf( " mce left sweep: pos = %2d, min_pos = %2d, edge xings = %d\n",
              i, min_position, current_edge_crossing_count );
#endif
    }

  // Undo the left sweep (no need to check for min)
  for ( int i = 0; i < node->position; i++ )
    {
       edge_crossings_after_swap( node, nodes_on_layer[i] );
#ifdef DEBUG
      printf( " mce undo sweep: pos = %2d, min_pos = %2d, edge xings = %d\n",
              i, min_position, current_edge_crossing_count );
#endif
    }

  // Then sweep all the way to the right
  for ( int i = node->position + 1; i < layer_size; i++ )
    {
      int current_edge_crossing_count
        = edge_crossings_after_swap( node, nodes_on_layer[i] );
      if ( current_edge_crossing_count < min_edge_crossing_count
           || ( current_edge_crossing_count == min_edge_crossing_count
                && abs(node->position - i) > max_distance )
           )
        {
          min_edge_crossing_count = current_edge_crossing_count;
          min_position = i;
          max_distance = abs(node->position - i);
        }
#ifdef DEBUG
      printf( " mce right sweep: pos = %2d, min_pos = %2d, edge xings = %d\n",
              i, min_position, current_edge_crossing_count );
#endif
    }

  reposition_node( node, nodes_on_layer, min_position ); 

  // recompute crossings with respect to this layer
  updateCrossingsForLayer( layer );
}

/*  [Last modified: 2014 03 17 at 11:38:49 GMT] */
