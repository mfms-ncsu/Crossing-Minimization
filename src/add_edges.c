/**
 * @file add_edges.c
 * @brief Main program for adding random edges to a given dag (usually a
 * tree) in order to achieve a given number of edges.
 *
 * @author Matt Stallmann, 2011/07/07
 *
 * $Id: add_edges.c 27 2011-07-09 21:22:42Z mfms $
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<libgen.h>              /* basename() */
#include<assert.h>
#include"graph.h"
#include"graph_io.h"
#include"Statistics.h"
#include"heuristics.h"
#include"random_tree.h"
#include"check_edge_duplication.h"

static void usage_message( char * prog_name )
{
  char * truncated_prog_name = basename( prog_name );
  printf(
         "Usage: %s input_basename output_basename edges seed\n"
         " where input_basename.dot and input_basename.ord are the input files\n"
         "           representing the dag to which edges are to be added\n"
         "       output_basename.dot and output_basename.ord are the output files\n"
         "   representing the dag with the added edges\n"
         "       edges is the *total* number of  edges desired\n"
         "       seed is a single integer seed for the random number stream\n"
         ,
         truncated_prog_name
         );
}

static void print_stats( void )
{
  Statistics degree_info = init_statistics( number_of_nodes );
  Statistics layer_info = init_statistics( number_of_layers );
  for( int layer = 0; layer < number_of_layers; layer++ )
    {
      add_data( layer_info, layers[ layer ]->number_of_nodes );
      for( int position = 0; position < layers[ layer ]->number_of_nodes; position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ position ];
          add_data( degree_info, DEGREE( node ) );
        }
    }
  printf( "NumberOfNodes,%d\n", number_of_nodes );
  printf( "NumberOfEdges,%d\n", number_of_edges );
  printf( "EdgeDensity,%2.2f\n", ((double) number_of_edges) / number_of_nodes );
  printf( "DegreeStats\t" );
  print_statistics( degree_info, stdout, "%2.1f" );
  printf( "\n" );
  printf( "LayerSize\t" );
  print_statistics( layer_info, stdout, "%2.1f" );
  printf( "\n" );
  free_statistics( layer_info );
  free_statistics( degree_info );
}

/**
 * Adds an edge between two nodes whose layers are determined and adjacent to
 * one another
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
 * future, the correct answer will be given
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

static void add_edges( int desired_num_edges )
{
#ifdef DEBUG
  printf( "-> add_edges: number_of_nodes = %d, current_number_of_edges = %d,"
          " desired_number_of_edges = %d\n",
          number_of_nodes, number_of_edges, desired_num_edges );
#endif

  assert(
         number_of_nodes > 1
         && number_of_layers > 1
         );

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

int main( int argc, char * argv[] )
{
  if ( argc != 5 )
    {
      usage_message( argv[0] );
      return EXIT_FAILURE;
    }

  const char * input_base_name = argv[1];
  const char * output_base_name = argv[2];
  int edges = atoi( argv[3] );
  long seed = atoi( argv[4] );

  srandom( seed );

  // read the input graph
  char dot_name_buffer[MAX_NAME_LENGTH];
  char ord_name_buffer[MAX_NAME_LENGTH];
  strcpy( dot_name_buffer, input_base_name );
  strcat( dot_name_buffer, ".dot" );
  strcpy( ord_name_buffer, input_base_name );
  strcat( ord_name_buffer, ".ord" );
  readGraph( dot_name_buffer, ord_name_buffer );
  int original_num_edges = number_of_edges;

  // check whether the desired number of edges is reasonable
  // choice of maximum density is arbitrary, based on the number obtained if
  // all edges between two adjacent layers were present; the division by 4
  // prevents the program from spending too long to avoid duplicate edges
  double max_edges = (double) number_of_nodes * number_of_nodes / 4;
  if ( edges > max_edges )
    {
      printf( "Desired graph is too dense to be constructed, desired edges = %d, max edges = %2.0f\n",
              edges, max_edges );
      return EXIT_FAILURE;
    }

  // add edges to it
  add_edges( edges );

  print_stats();

  strcpy( graph_name, output_base_name );

  strcpy( dot_name_buffer, output_base_name );
  strcpy( ord_name_buffer, output_base_name );
  strcat( dot_name_buffer, ".dot" );
  strcat( ord_name_buffer, ".ord" );
  char header_info_buffer[MAX_NAME_LENGTH];
  sprintf( header_info_buffer,
           " random dag, created by: add_edges %s %s %d %d %d %ld\n",
           input_base_name, output_base_name,
           number_of_nodes, original_num_edges, number_of_edges, seed
           );

  writeDot(
           dot_name_buffer,
           graph_name,
           header_info_buffer,
           master_edge_list,
           number_of_edges
           );
  
  writeOrd( ord_name_buffer );

  return EXIT_SUCCESS;
}

/*  [Last modified: 2011 07 07 at 16:57:39 GMT] */
