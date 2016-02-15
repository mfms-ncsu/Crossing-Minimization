/**
 * @file barycenter.c
 * @brief implements various functions related to barycenter heuristics
 * @author Matthias Stallmann
 * @date 2008/12/29
 * $Id: barycenter.c 96 2014-09-09 16:37:16Z mfms $
 */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"defs.h"
#include"min_crossings.h"
#include"barycenter.h"
#include"graph.h"
#include"sorting.h"
#include"crossings.h"
#include"graph_io.h"
#include"heuristics.h"

/**
 * Computes the weight of a node based on the average position of its
 * neighbors according to the given orientation.
 * If the node has no neighbors in the direction specified by the
 * orientation, its neighbors in the other direction are taken into
 * account. Isolated nodes have 0 weight.
 */
static void node_weight( Nodeptr node, Orientation orientation )
{
  int total_degree = 0;
  int total_of_positions = 0;
  int adj_index;
  if( orientation != UPWARD )
    {
      total_degree += node->down_degree;
      for( adj_index = 0; adj_index < node->down_degree; adj_index++ )
        {
          total_of_positions
            += node->down_edges[adj_index]->down_node->position;
        }
    }
  if( orientation != DOWNWARD )
    {
      total_degree += node->up_degree;
      for( adj_index = 0; adj_index < node->up_degree; adj_index++ )
        {
          total_of_positions
            += node->up_edges[adj_index]->up_node->position;
        }
    }
  if( total_degree > 0 )
    node->weight = (double) total_of_positions / total_degree;
  else if( adjust_weights == NONE
           // put isolated nodes to the far left
           || node->up_degree + node->down_degree == 0 ) node->weight = 0;
  else
    // indicate that this is a special case - no edges in the given
    // orientation - and needs to be fixed 
    node->weight = -1;
#ifdef DEBUG
  printf("  node_weight, node = %d, weight = %f\n", node->id, node->weight );
#endif  
}

/**
 * computes weight based on two neighboring layers, but does it as
 *   1/2 * (upper_average + lower_average)
 * instead of
 *   sum_of_positions / total_degree
 */
static void balanced_node_weight( Nodeptr node ) {
#ifdef DEBUG
  printf( "-> balanced_node_weight, node = %d\n", node->id );
#endif
  int adj_index;
  int degree;
  int total_of_positions;

  // compute average position in the downward direction
  total_of_positions = 0;
  degree = node->down_degree;
  for( adj_index = 0; adj_index < degree; adj_index++ ) {
      total_of_positions
        += node->down_edges[adj_index]->down_node->position;
  }
  double downward_average;
  if ( degree > 0 ) downward_average = (double) total_of_positions / degree;
  else downward_average = 0;

  // compute average position in the upward direction
  total_of_positions = 0;
  degree = node->up_degree;
  for( adj_index = 0; adj_index < degree; adj_index++ ) {
      total_of_positions
        += node->up_edges[adj_index]->up_node->position;
  }
  double upward_average;
  if ( degree > 0 ) upward_average = (double) total_of_positions / degree;
  else upward_average = 0;

  node->weight = (downward_average + upward_average) / 2;
#ifdef DEBUG
  printf( "<- balanced_node_weight, node = %d, weight = %4.1f,"
          " down_avg = %4.1f, up_avg = %4.1f\n",
          node->id, node->weight, downward_average, upward_average );
#endif
}

/**
 * Some nodes may have weight -1 because they have no edges in the
 * desired direction.  This function gives them a weight identical to that of
 * their left neighbor on the layer, or 0 if all nodes to the left have
 * weight -1.
 * Note: If the node to the left has weight -1 originally, its weight will
 * have been changed by the time the node is processed.
 *
 * @todo adjust_weights_left() and adjust_weights_avg() have identical
 * counterparts in median.c
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
static void adjust_weights_avg( int layer ) {
  Layerptr layerptr = layers[ layer ];
  int num_nodes = layerptr->number_of_nodes;
  // this method of adjusting weights is used in parallel barycenter
  // versions, so it is important that the adjusted weight of a node is not
  // influenced by the already adjusted weight of its left neighbor;
  // temp_weights holds the unadjusted weights
  double * temp_weights;
  bool parallel = (number_of_processors != 1);
  if ( parallel ) {
    temp_weights = (double *) calloc( num_nodes, sizeof(double) );
    for ( int i = 0; i < num_nodes; i++ ) {
      temp_weights[i] = layerptr->nodes[i]->weight;
    }
  }
  for ( int i = 0; i < num_nodes; i++ ) {
    Nodeptr node = layerptr->nodes[i];
    double weight = parallel ? temp_weights[i] : layerptr->nodes[i]->weight; 
    // Do nothing if node already has a weight
    if ( weight != -1 ) continue;

    double left_weight = -1;
    double right_weight = -1;
    if ( i > 0 ) {
      left_weight = parallel ? temp_weights[i-1] : layerptr->nodes[i-1]->weight;
    }
    if ( i < num_nodes - 1 ) {
      right_weight = parallel ? temp_weights[i+1] : layerptr->nodes[i+1]->weight;
    }

    // if both neighbors are present and have weights, take the
    // average of their weights
    if ( left_weight != -1 && right_weight != -1 ) {
      node->weight = (left_weight + right_weight) / 2;
    }
    else if ( left_weight != -1 ) {
      // only the left neighbor has a weight
      node->weight = left_weight;
    }
    else if ( right_weight != -1 ) {
      // only the right neighbor has a weight
      node->weight = right_weight;
    }
    else {
      // neither neighbor has a weight: can propagate from left if not parallel
      node->weight = parallel ? 0 : left_weight;
    }
#ifdef DEBUG
    printf("  adjust_weight (avg), node = %s, weight = %f\n",
           node->name, node->weight );
#endif  
  } // for nodes on this layer
  if ( parallel )
    free( temp_weights );
} // end, adjust_weights_avg

/**
 * Assigns weights to nodes on the given layer based on positions of their
 * edges above, below, or both, as specified by the orientation.
 */
void barycenterWeights( int layer, Orientation orientation )
{
#ifdef DEBUG
  printf("-> barycenterWeights, layer = %d, orientation = %d"
         ", balanced_weight = %d\n",
         layer, orientation, balanced_weight );
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
      if ( orientation == BOTH && balanced_weight )
        balanced_node_weight( layerptr->nodes[i] );
      else
        node_weight( layerptr->nodes[i], orientation );
    }
  if( adjust_weights == LEFT )
    adjust_weights_left( layer );
  else if( adjust_weights == AVG )
    adjust_weights_avg( layer );
#ifdef DEBUG
  printf( "<- barycenterWeights\n" );
#endif  
}      

bool barycenterUpSweep( int starting_layer )
{
  int layer = starting_layer;
  for( ; layer < number_of_layers; layer++ )
    {
      barycenterWeights( layer, DOWNWARD );
      layerSort( layer );
      //      layerQuicksort( layer );
      //      layerUnstableSort( layer );
      updateCrossingsForLayer( layer );
      tracePrint( layer, "barycenter upsweep" );
      if ( end_of_iteration() )
        return true;
    }
  return false;
}

/**
 * Repeats barycenter heuristic moving downward from the starting layer to the
 * bottom layer, layer 0. Orientation of each heuristic application is upward.
 */
bool barycenterDownSweep( int starting_layer )
{
  int layer = starting_layer;
  for( ; layer >= 0; layer-- )
    {
      barycenterWeights( layer, UPWARD );
      layerSort( layer );
      //      layerQuicksort( layer );
      //      layerUnstableSort( layer );
      updateCrossingsForLayer( layer );
      tracePrint( layer, "barycenter downsweep" );
      if ( end_of_iteration() )
        return true;
    }
  return false;
}

/*  [Last modified: 2014 09 09 at 15:52:37 GMT] */
