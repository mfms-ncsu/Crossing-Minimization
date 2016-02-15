/**
 * @file create_random_dag.c
 * @brief Main program for creating a random dag with a specified number of
 * vertices, edges, and layers.
 * @author Matt Stallmann, 2011/06/01
 *
 * $Id: create_random_dag.c 2 2011-06-07 19:50:41Z mfms $
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<libgen.h>              /* basename() */
#include"graph.h"
#include"graph_io.h"
#include"Statistics.h"
#include"heuristics.h"
#include"random_tree.h"
#include"random_dag.h"

void usage_message( char * prog_name )
{
  char * truncated_prog_name = basename( prog_name );
  printf(
         "Usage: %s basename nodes edges layers skew seed\n"
         " where basename.dot and basename.ord are the output files\n"
         "       nodes, edges, layers are the number of nodes, edges, and layers of the dag, respectively\n"
         "       skew is a factor that affects max degree and variance of layer size\n"
         "       it should be at least 3 for sparse graphs to be interesting\n"
         "          large skew => large max degree and large variance\n"
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

int main( int argc, char * argv[] )
{
  if ( argc != 7 )
    {
      usage_message( argv[0] );
      return EXIT_FAILURE;
    }

  const char * base_name = argv[1];
  int nodes = atoi( argv[2] );
  int edges = atoi( argv[3] );
  int layers = atoi( argv[4] );
  int branching = atoi( argv[5] );
  long seed = atoi( argv[6] );

  if ( edges < nodes - 1 )
    {
      printf( "WARNING: number of edges is %d, less than the %d required\n",
              edges, nodes - 1 );
    }

  // choice of maximum density is arbitrary, based on having all edges
  // between two adjacent layers; the division by 4 avoids spending too long
  // to avoid duplicate edges
  double max_edges = (double) nodes * nodes / 4;
  if ( edges > max_edges )
    {
      printf( "Desired graph is too dense to be constructed, desired edges = %d, max edges = %2.0f\n",
              edges, max_edges );
      return EXIT_FAILURE;
    }


  srandom( seed );

  create_random_dag( nodes, edges, layers, branching ); 
  //  create_random_tree( nodes, layers, branching ); 

  print_stats();

  strcpy( graph_name, base_name );

  char dot_file_buffer[MAX_NAME_LENGTH];
  char ord_file_buffer[MAX_NAME_LENGTH];
  char header_info_buffer[MAX_NAME_LENGTH];

  strcpy( dot_file_buffer, base_name );
  strcpy( ord_file_buffer, base_name );
  strcat( dot_file_buffer, ".dot" );
  strcat( ord_file_buffer, ".ord" );
  sprintf( header_info_buffer,
           " random dag, created by: create_random_dag %s %d %d %d %d %ld\n",
           graph_name, nodes, edges, layers, branching, seed
           );

  writeDot(
           dot_file_buffer,
           graph_name,
           header_info_buffer,
           master_edge_list,
           number_of_edges
           );
  
  writeOrd( ord_file_buffer );

  return EXIT_SUCCESS;
}

/*  [Last modified: 2011 06 03 at 17:57:06 GMT] */
