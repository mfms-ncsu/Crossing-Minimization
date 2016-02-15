/**
 * @file median.c
 * @brief implements various functions related to median heuristics
 * @author Matthias Stallmann
 * @date 2008/12/29
 * $Id: median.c 96 2014-09-09 16:37:16Z mfms $
 */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"defs.h"
#include"min_crossings.h"
#include"median.h"
#include"graph.h"
#include"sorting.h"
#include"crossings.h"
#include"graph_io.h"
#include"heuristics.h"

/**
 * @return the median position of the nodes adjacent to 'node' on the layer
 * above 'node' 
 */
static double upper_median( Nodeptr node )
{
  // -1 indicates no up edges -- see the adjust_weights functions below
  if ( node->up_degree == 0 ) return -1;
  sortByUpNodePosition( node->up_edges, node->up_degree );
  int median_position = (node->up_degree - 1) / 2;
  Edgeptr median_edge = node->up_edges[ median_position ];
  return median_edge->up_node->position; 
}

/**
 * @return the median position of the nodes adjacent to 'node' on the layer
 * below 'node' 
 */
static double lower_median( Nodeptr node )
{
  // -1 indicates no down edges -- see the adjust_weights functions below
  if ( node->down_degree == 0 ) return -1;
  sortByDownNodePosition( node->down_edges, node->down_degree );
  int median_position = (node->down_degree - 1) / 2;
  Edgeptr median_edge = node->down_edges[ median_position ];
  return median_edge->down_node->position; 
}

/**
 * Computes the weight of a node based on the median position of its
 * neighbors according to the given orientation.
 */
static void node_weight( Nodeptr node, Orientation orientation )
{
#ifdef DEBUG
  printf("-> (median) node_weight, node = %s, orientation = %d\n", node->name, orientation );
#endif  
  assert( orientation != BOTH );
  if( orientation == UPWARD )
    {
      node->weight = upper_median( node );
    }
  else if ( orientation == DOWNWARD )
    {
      node->weight = lower_median( node );
    }
#ifdef DEBUG
  printf("<- (median) node_weight, node = %s, weight = %f\n", node->name, node->weight );
#endif  
}

/**
 * computes weight based on two neighboring layers as
 *   1/2 * (upper_median + lower_median)
 */
static void two_layer_node_weight( Nodeptr node )
{
  node->weight = ( upper_median(node) + lower_median(node) ) / 2;
}

/**
 * Some nodes may have weight -1 because they have no edges in the
 * desired direction.  This function gives them a weight identical to that of
 * their left neighbor on the layer, or 0 if all nodes to the left have
 * weight -1.
 * Note: If the node to the left has weight -1 originally, its weight will
 * have been changed by the time the node is processed.
 */
static void adjust_weights_left( int layer )
{
  Layerptr layerptr = layers[ layer ];
  int i = 0;
  for( ; i < layerptr->number_of_nodes; i++ )
    {
      Nodeptr node = layerptr->nodes[i];
      if( node->weight == -1 )
        {
          if( i == 0 ) node->weight = 0;
          else node->weight = layerptr->nodes[i-1]->weight;
#ifdef DEBUG
          printf("  adjust_weight (left), node = %s, weight = %f\n",
                 node->name, node->weight );
#endif  
        }
    }
}

/**
 * Some nodes may have weight -1 because they have no edges in the
 * desired direction.  This function gives them a weight that is the average
 * of that of the two neighbors on the layer - or just the weight of one of
 * the neighbors if the other is absent or also has weight -1.
 */
static void adjust_weights_avg( int layer )
{
  Layerptr layerptr = layers[ layer ];
  int i = 0;
  for( ; i < layerptr->number_of_nodes; i++ )
    {
      Nodeptr node = layerptr->nodes[i];
      if( node->weight == -1 )
        {
          int number_of_weights = 0;
          double total_weight = 0;
          if( i > 0 )
            {
              number_of_weights++;
              total_weight += layerptr->nodes[i-1]->weight;
            }
          if( i < layerptr->number_of_nodes - 1
              && layerptr->nodes[i+1]->weight >= 0 )
            {
              number_of_weights++;
              total_weight += layerptr->nodes[i+1]->weight;
            }
          if( number_of_weights > 0 )
            node->weight = total_weight / number_of_weights;
          else
            // this happens if the leftmost node has a right neighbor with
            // weight of -1
            node->weight = 0;
#ifdef DEBUG
          printf("  adjust_weight (avg), node = %s, weight = %f\n",
                 node->name, node->weight );
#endif  
        }
    }
}

/**
 * Assigns weights to nodes on the given layer based on positions of their
 * edges above, below, or both, as specified by the orientation.
 */
void medianWeights( int layer, Orientation orientation )
{
#ifdef DEBUG
  printf("-> medianWeights, layer = %d, orientation = %d\n",
         layer, orientation );
#endif  
  Layerptr layerptr = layers[ layer ];
  int i = 0;
  int num_nodes = layerptr->number_of_nodes;
/*
#ifdef _OPENMP
#pragma omp parallel for default(none) private(i) \
  shared(num_nodes, orientation, balanced_weight, \
  layerptr, trace_freq, number_of_processors)
#endif
*/
  for(i = 0 ; i < num_nodes; i++ )
    {
      if ( orientation == BOTH )
        two_layer_node_weight( layerptr->nodes[i] );
      else
        node_weight( layerptr->nodes[i], orientation );
    }
  if( adjust_weights == LEFT )
    adjust_weights_left( layer );
  else if( adjust_weights == AVG )
    adjust_weights_avg( layer );
#ifdef DEBUG
  printf( "<- medianWeights\n" );
#endif  
}

bool medianUpSweep( int starting_layer )
{
  int layer = starting_layer;
  for( ; layer < number_of_layers; layer++ )
    {
      medianWeights( layer, DOWNWARD );
      layerSort( layer );
      updateCrossingsForLayer( layer );
      tracePrint( layer, "median upsweep" );
      if ( end_of_iteration() )
        return true;
    }
  return false;
}

/**
 * Repeats median heuristic moving downward from the starting layer to the
 * bottom layer, layer 0. Orientation of each heuristic application is upward.
 */
bool medianDownSweep( int starting_layer )
{
  int layer = starting_layer;
  for( ; layer >= 0; layer-- )
    {
      medianWeights( layer, UPWARD );
      layerSort( layer );
      updateCrossingsForLayer( layer );
      tracePrint( layer, "median downsweep" );
      if ( end_of_iteration() )
        return true;
    }
  return false;
}

/*  [Last modified: 2014 09 09 at 15:54:14 GMT] */
