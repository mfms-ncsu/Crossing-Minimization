/**
 * @file IO.c
 * @brief Functions that deal with layered graph input and output (output
 * only for now)
 * @author Matt Stallmann
 * @date 2008/07/23
 * $Id: IO.c 9 2011-06-13 01:29:18Z mfms $
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "IO.h"
#include "LayeredGraph.h"

/** Maximum number of nodes per line in printing the .ord files */
#define MAX_NODES_PER_LINE 8

void writeDot( FILE * dot_file_stream, Graph G )
{
  fprintf( dot_file_stream, "/* Created by: %s */\n", getHowCreated(G) );
  fprintf( dot_file_stream, "digraph %s {\n", getGraphName(G) );
  const Edge * edge_array = getAllEdges( G );
  int number_of_edges = getNumberOfEdges( G );
  for( int i = 0; i < number_of_edges; i++ )
    {
      const char * from_name
        = getName( G, edge_array[i].from );
      const char * to_name
        = getName( G, edge_array[i].to );
      fprintf( dot_file_stream, "  %s -> %s;\n", from_name, to_name );
    }
  fprintf( dot_file_stream, "}\n" );
}

void writeOrd( FILE * ord_file_stream, Graph G )
{
  fprintf( ord_file_stream, "# Natural ordering for graph %s\n",
           getGraphName(G) );
  fprintf( ord_file_stream, "# Created by %s\n", getHowCreated(G) );
  for( int current_layer = 0;
       current_layer < getNumberOfLayers( G );
       current_layer++ )
    {
      fprintf( ord_file_stream, "\n# Ordering for layer %d\n", current_layer );
      fprintf( ord_file_stream, "%d {\n", current_layer );

      // print names of all nodes on the current layer with at most
      // MAX_NODES_PER_LINE as given
      const Node * node_list = getNodesOnLayer( G, current_layer );
      for( int j = 0;
           j < getNumberOfNodesOnLayer( G, current_layer );
           j++ )
        {
          if( j > 1 && j % MAX_NODES_PER_LINE == 1 )
            fprintf( ord_file_stream, "\n" );
          fprintf( ord_file_stream, " %s", getName( G, node_list[j] ) );
        }
      fprintf( ord_file_stream, "\n} # end of layer %d\n", current_layer );
    }
}

/*  [Last modified: 2008 09 18 at 20:47:41 GMT] */
