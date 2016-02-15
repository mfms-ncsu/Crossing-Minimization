/**
 * @file swap.c
 * @brief Implementation of functions that compute the change in crossing
 * number (or max edge crossings if desired) when two neighboring nodes are swapped.
 * @author Matt Stallmann
 * @date 2011/05/21
 * $Id: swap.c 2 2011-06-07 19:50:41Z mfms $
 */

#include"graph.h"
#include"defs.h"
#include"crossings.h"
#include"crossing_utilities.h"
#include"swap.h"
#include"sorting.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<stdbool.h>
#include<limits.h>

/**
 * Fills edge_array with upward edges of the two nodes, each set of edges
 * sorted by their endpoint positions on the layer above and those of the
 * first node coming first. Used to compute number of swaps by counting
 * inversions [reference to paper by Jünger and Mutzel needed]
 */
void create_sorted_up_edge_array( Edgeptr * edge_array,
                                  Nodeptr first_node,
                                  Nodeptr second_node );

/**
 * Fills edge_array with downward edges of the two nodes, each set of edges
 * sorted by their endpoint positions on the layer above and those of the
 * first node coming first. Used to compute number of swaps by counting
 * inversions [reference to paper by Jünger and Mutzel needed]
 */
void create_sorted_down_edge_array( Edgeptr * edge_array,
                                    Nodeptr first_node,
                                    Nodeptr second_node );

int edge_crossings_for_node( Nodeptr node )
{
  int edge_crossings = 0;
  // update max_edge_crossings based on upward edges
  for ( int i = 0; i < node->up_degree; i++ )
    if ( node->up_edges[i]->crossings > edge_crossings )
      edge_crossings = node->up_edges[i]->crossings;
  // update max_edge_crossings based on downward edges
  for ( int i = 0; i < node->down_degree; i++ )
    if ( node->down_edges[i]->crossings > edge_crossings )
      edge_crossings = node->down_edges[i]->crossings;
  return edge_crossings;
}

int node_crossings( Nodeptr node_a, Nodeptr node_b )
{
  assert( node_a->layer == node_b->layer );
  int layer = node_a->layer;

  int total_crossings = 0;

  // count crossings among upward edges (if any)
  if ( layer < number_of_layers - 1 )
    {
      Edgeptr * edge_array
        = (Edgeptr *) calloc( node_a->up_degree + node_b->up_degree,
                              sizeof(Edgeptr) );
      create_sorted_up_edge_array( edge_array, node_a, node_b );
      total_crossings += count_inversions_up( edge_array,
                                              node_a->up_degree
                                              + node_b->up_degree, 1 );
      free( edge_array );
    }

  // count crossings among downward edges (if any)
  if ( layer > 0 )
    {
      Edgeptr * edge_array
        = (Edgeptr *) calloc( node_a->down_degree + node_b->down_degree,
                              sizeof(Edgeptr) );

      create_sorted_down_edge_array( edge_array, node_a, node_b );
      total_crossings += count_inversions_down( edge_array,
                                                node_a->down_degree
                                                + node_b->down_degree, 1 );
      free( edge_array );
    }
  return total_crossings;
}

void change_crossings( Nodeptr left_node, Nodeptr right_node, int diff )
{
  int layer = left_node->layer;

  // update crossings on upward edges (if any)
  if ( layer < number_of_layers - 1 )
    {
      Edgeptr * edge_array
        = (Edgeptr *) calloc( left_node->up_degree + right_node->up_degree,
                              sizeof(Edgeptr) );
      create_sorted_up_edge_array( edge_array, left_node, right_node );
      count_inversions_up( edge_array,
                           left_node->up_degree + right_node->up_degree,
                           diff );
      free( edge_array );
    }

  // update crossings on downward edges (if any)
  if ( layer > 0 )
    {
      Edgeptr * edge_array
        = (Edgeptr *) calloc( left_node->down_degree + right_node->down_degree,
                              sizeof(Edgeptr) );
      create_sorted_down_edge_array( edge_array, left_node, right_node );
      count_inversions_down( edge_array,
                             left_node->down_degree + right_node->down_degree,
                             diff );
      free( edge_array );
    }
}

int edge_crossings_after_swap( Nodeptr left_node, Nodeptr right_node )
{
  change_crossings( left_node, right_node, -1 );
  change_crossings( right_node, left_node, +1 );
  int left_node_edge_crossings = edge_crossings_for_node( left_node );
  int right_node_edge_crossings = edge_crossings_for_node( right_node );
  if( left_node_edge_crossings > right_node_edge_crossings )
    return left_node_edge_crossings;
  else
    return right_node_edge_crossings;
}

void create_sorted_up_edge_array( Edgeptr * edge_array,
                                  Nodeptr first_node,
                                  Nodeptr second_node )
{
  sortByUpNodePosition( first_node->up_edges, first_node->up_degree );
  sortByUpNodePosition( second_node->up_edges, second_node->up_degree );
  add_edges_to_array( edge_array,
                      first_node->up_edges, first_node->up_degree, 0 );
  add_edges_to_array( edge_array,
                      second_node->up_edges, second_node->up_degree,
                      first_node->up_degree );
}

void create_sorted_down_edge_array( Edgeptr * edge_array,
                                    Nodeptr first_node,
                                    Nodeptr second_node )
{
  sortByDownNodePosition( first_node->down_edges, first_node->down_degree );
  sortByDownNodePosition( second_node->down_edges, second_node->down_degree );
  add_edges_to_array( edge_array,
                      first_node->down_edges, first_node->down_degree, 0 );
  add_edges_to_array( edge_array,
                      second_node->down_edges, second_node->down_degree,
                      first_node->down_degree );
}

/*  [Last modified: 2011 05 24 at 15:59:47 GMT] */
