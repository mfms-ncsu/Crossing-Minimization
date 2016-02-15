/**
 * @file dot_and_ord_to_sgf.c
 * @brief Program to convert a dot and ord file to an equivalent sgf file
 * @author Matt Stallmann
 * @date 2011/06/16
 *
 * sgf format is as follows (blank lines are ignored):
 *    c comment line 1
 *    ...
 *    c comment line k
 *
 *    t graph_name nodes edges layers
 *
 *    n id_1 layer_1 position_1
 *    n id_2 layer_2 position_2
 *    ...
 *    n id_n layer_n position_n
 *
 *    e source_1 target_1
 *    ...
 *    e source_m target_m
 *
 * edges are directed so that the nodes appearing earlier in the input are
 * sources
 *
 * @todo comments from the original dot file are not preserved and should be
 * added by an external script for now.
 *
 * Deviations (not relevant here):
 * - edge direction may not be correct yet, but can be fixed using
 * sgfDirect.awk
 * - nodes, edges, and layers may be missing in the title (t) line, but can
 * be deduced
 * - the 'n' lines may be missing, but can be deduced from the title
 * - layers and positions may be missing in the 'n' lines but can be filled
 * in by existing programs and/or scripts
 * - the 'e' lines may have additional information, e.g., to identify edges
 * as 'favored' (details not worked out yet)
 *
 * $Id: dot_and_ord_to_sgf.c 73 2014-07-17 20:36:15Z mfms $
 */


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<assert.h>

#include"defs.h"
#include"graph_io.h"
#include"graph.h"

/**
 * prints usage message
 */
static void printUsage( void ) {
  printf( "Usage: dot_and_ord_to_sgf DOT_FILE_NAME ORD_FILE_NAME\n" );
  printf( " reads files DOT_FILE_NAME and ORD_FILE_NAME to produce an sgf file\n" );
  printf( " printing it on standard output\n");
}

/**
 * Writes an sgf file based on the current graph to standard output
 */
static void write_sgf( void ) {
  printf( "t %s %d %d %d\n",
           graph_name,
           number_of_nodes,
           number_of_edges,
           number_of_layers
           );
  
  // add lines for the nodes
  for( int layer = 0; layer < number_of_layers; layer++ ) {
    for( int position = 0;
         position < layers[ layer ]->number_of_nodes;
         position++ ) {
      Nodeptr node = layers[ layer ]->nodes[ position ];
      printf( "n %d %d %d\n", node->id, layer, position );
    }
  }

  // add lines for the edges
  for( int layer = 0; layer < number_of_layers - 1; layer++ ) {
    for(
        int node_position = 0;
        node_position < layers[ layer ]->number_of_nodes;
        node_position++ ) {
      Nodeptr node = layers[ layer ]->nodes[ node_position ];
      for( int edge_position = 0;
           edge_position < node->up_degree;
           edge_position++ ) {
        Edgeptr edge = node->up_edges[ edge_position ];
        printf( "e %d %d\n", node->id, edge->up_node->id );
      }
    }
  }
}

int main( int argc, char * argv[] )
{
  if( argc != 3 ) {
      printUsage();
      return EXIT_FAILURE;
  }
  const char * dot_file_name = argv[1];
  const char * ord_file_name = argv[2];

  readGraph( dot_file_name, ord_file_name );
  write_sgf();

  return EXIT_SUCCESS;
}

/*  [Last modified: 2014 07 17 at 20:14:41 GMT] */
