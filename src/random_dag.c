/**
 * @file random_tree.c
 * @brief Module for creating a random dag with a given number of nodes and
 * layers. First a tree is created to form the backbone of the dag.
 *
 * @author Matt Stallmann
 * @date 2011/06/01
 * $Id: random_dag.c 27 2011-07-09 21:22:42Z mfms $
 */

#include "graph.h"
#include "graph_io.h"
#include "random_tree.h"
#include "random_dag.h"
#include "check_edge_duplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/**
 * Adds an edge between two nodes whose layers are determined and adjacent to
 * one another
 *
 * @todo Avoid duplication with the same function in create_random_tree and
 * add_edges
 */
static void add_edge( Nodeptr upper_node, Nodeptr lower_node )
{
#ifdef DEBUG
  printf( "-> add_edge: upper_node = (%s,%d,%d), lower_node = (%s,%d,%d)\n",
          upper_node->name, upper_node->layer, upper_node->position,
          lower_node->name, lower_node->layer, lower_node->position );
#endif
  assert( upper_node->layer == lower_node->layer + 1 );
  Edgeptr new_edge = (Edgeptr) calloc( 1, sizeof(struct edge_struct) );
  new_edge->up_node = upper_node;
  new_edge->down_node = lower_node;
  new_edge->fixed = false;

  // add new edge to master edge list, making room if necessary
  if ( number_of_edges % CAPACITY_INCREMENT == 0 )
    {
      master_edge_list
        = (Edgeptr *) realloc( master_edge_list,
                               (number_of_edges + CAPACITY_INCREMENT) * sizeof(Edgeptr) );
    }
  master_edge_list[ number_of_edges++ ] = new_edge;

  // add new edge to lower edge list of upper node, making room if necessary
  if ( upper_node->down_degree % CAPACITY_INCREMENT == 0 )
    {
      upper_node->down_edges
        = (Edgeptr *) realloc( upper_node->down_edges,
                               (upper_node->down_degree + CAPACITY_INCREMENT) * sizeof(Edgeptr) );
    }
  upper_node->down_edges[ upper_node->down_degree++ ] = new_edge;
  // add new edge to upper edge list of lower node, making room if necessary
  if ( lower_node->up_degree % CAPACITY_INCREMENT == 0 )
    {
      lower_node->up_edges
        = (Edgeptr *) realloc( lower_node->up_edges,
                               lower_node->up_degree + CAPACITY_INCREMENT * sizeof(Edgeptr) );
    }
  lower_node->up_edges[ lower_node->up_degree++ ] = new_edge;

#ifdef DEBUG
  printf( "<- add_edge: edge = %s -> %s\n",
          new_edge->up_node->name, new_edge->down_node->name );
#endif
}

/**
 * Make it so that when the current edges are checked for existence in the
 * future, the correct answer will be given; for use after tree edges are
 * created.
 */
static void make_all_current_edges_exist( void )
{
  for ( int i = 0; i < number_of_edges; i++ )
    {
      Edgeptr edge = master_edge_list[i];
      int up_node_id = edge->up_node->id;
      int down_node_id = edge->down_node->id;
      pair_already_exists( up_node_id, down_node_id );
    }
}

void create_random_dag( int num_nodes,
                        int desired_num_edges,
                        int num_layers,
                        int branching_factor )
{
#ifdef DEBUG
  printf( "-> create_random_dag: nodes = %d, edges = %d, layers = %d, branching = %d\n"
          "  number_of_nodes = %d, number_of_edges = %d\n",
          num_nodes, desired_num_edges, num_layers, branching_factor, number_of_nodes, number_of_edges );
#endif

  assert( num_nodes > 0
          && desired_num_edges > 0
          && num_layers > 1
          && branching_factor > 0 );

  create_random_tree( num_nodes, num_layers, branching_factor );
#ifdef DEBUG
  printf( " After create_random_tree: number_of_nodes = %d, number_of_edges = %d, desired = %d\n",
          number_of_nodes, number_of_edges, desired_num_edges );
#endif

  create_hash_table_for_pairs( desired_num_edges );

  make_all_current_edges_exist();

  while ( desired_num_edges > number_of_edges )
    {
      // pick two random nodes that are on adjacent layers
      // if there's not already an edge between them, add one

      // pick a node that's not on layer 0
      int first_node_index = random() % number_of_nodes;
      Nodeptr first_node = master_node_list[ first_node_index ];
      int first_node_layer_number = first_node->layer;
#ifdef DEBUG
      printf( " Loop iteration: number_of_edges = %d, desired = %d\n"
              "   first_node = %d [%d]\n",
              number_of_edges, desired_num_edges,
              first_node_index, first_node_layer_number );
#endif
      if ( first_node_layer_number == 0 ) continue;

      // pick another node on the layer below that of the first node
      int second_node_layer_number = first_node_layer_number - 1;
      Layerptr second_node_layer = layers[ second_node_layer_number ];
      int second_node_layer_position = random() % second_node_layer->number_of_nodes;
      Nodeptr second_node = second_node_layer->nodes[ second_node_layer_position ];
      int second_node_index = second_node->id;

#ifdef DEBUG
      printf( " Attempting to add edge: %d [%d] -> %d [%d]\n",
              first_node_index, first_node_layer_number,
              second_node_index, second_node_layer_number );
#endif

      // add the edge if it doesn't already exist
      if ( ! pair_already_exists( first_node_index, second_node_index ) )
        {
          add_edge( first_node, second_node );
        }
    }
  destroy_hash_table_for_pairs();
}

/*  [Last modified: 2011 07 07 at 16:12:28 GMT] */
