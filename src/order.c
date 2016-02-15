/**
 * @file order.c
 * @brief implementation of functions for saving/restoring order
 * information.
 * @author Matt Stallmann
 * @date 2009/07/27
 * $Id: order.c 2 2011-06-07 19:50:41Z mfms $
 */

#include"order.h"
#include"graph.h"

#ifdef DEBUG
#include"crossings.h"
#endif

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

void init_order( Orderptr ord_info )
{
  ord_info->num_layers = number_of_layers;
  ord_info->num_nodes_on_layer
    = (int *) calloc( number_of_layers, sizeof(int) );
  ord_info->node_ptr_on_layer
    = (Nodeptr * *) calloc( number_of_layers, sizeof(Nodeptr *) );
  for ( int i = 0; i < number_of_layers; i++ )
    {
      ord_info->num_nodes_on_layer[i] = layers[i]->number_of_nodes;
      ord_info->node_ptr_on_layer[i]
        = (Nodeptr *) calloc( layers[i]->number_of_nodes, sizeof(Nodeptr) );
    }
  save_order( ord_info );
}

void cleanup_order( Orderptr ord_info )
{
  if ( ord_info->num_layers == 0 ) return;
  for ( int i = 0; i < ord_info->num_layers; i++ )
    {
      free( ord_info->node_ptr_on_layer[i] );
    }
  free( ord_info->node_ptr_on_layer );
  free( ord_info->num_nodes_on_layer );
}

void save_order( Orderptr ord_info )
{
  ord_info->num_layers = number_of_layers;
  for ( int i = 0; i < number_of_layers; i++ )
    {
      for( int j = 0; j < layers[i]->number_of_nodes; j++ )
        {
          Nodeptr node = layers[i]->nodes[j];
          ord_info->node_ptr_on_layer[i][j] = node; 
        }
    }
}

void restore_order( Orderptr ord_info )
{
#ifdef DEBUG
  updateAllCrossings();
  printf( "-> restore_order, num_layers = %d, crossings = %d\n", ord_info->num_layers, numberOfCrossings() );
#endif
  // reorder each layer according to the information stored in ord_info
  for ( int i = 0; i < ord_info->num_layers; i++ )
    {
      for( int j = 0; j < ord_info->num_nodes_on_layer[i]; j++ )
        {
          Nodeptr node = ord_info->node_ptr_on_layer[i][j];
          layers[i]->nodes[j] = node;
          node->position = j;
        }
#ifdef DEBUG
      updateAllCrossings();
      printf( " - restore_order, i = %d, num_nodes = %d, crossings = %d\n",
              i, ord_info->num_nodes_on_layer[i], numberOfCrossings() );
#endif
    }
#ifdef DEBUG
  updateAllCrossings();
  printf( "<- restore_order, crossings = %d\n", numberOfCrossings() );
#endif
}

/*  [Last modified: 2011 05 23 at 21:09:34 GMT] */
