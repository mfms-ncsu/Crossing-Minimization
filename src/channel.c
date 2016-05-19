/**
 * @file channel.h
 * @brief implementation of utilities that maintain information about channels;
 * eventually these will replace the interlayer structures in crossings.c
 * since they serve more general purposes.
 */

#include<stdio.h>
#include <stdlib.h>
#include "graph.h"
#include "min_crossings.h"
#include "heuristics.h"
#include "channel.h"
#include "stretch.h"
#include "random.h"

Channelptr * channels;

/**
 * @return the number of edges between layers i-1 and i
 */
static int count_down_edges(int i)
{
  Layerptr layer = layers[i];
  int count = 0;
  for( int j = 0; j < layer->number_of_nodes; j++ ) {
    count += layer->nodes[j]->down_degree; 
  }
  return count;
}

/**
 * @return a pointer to the list of edges for channel i and fills in
 * appropriate data: number of edges and the actual edges; note: channel i is
 * between layers i-1 and i
 */
static Channelptr initChannel(int i) {
  Channelptr new_channel
    = (Channelptr) calloc(1, sizeof(struct channel_struct));
  new_channel->number_of_edges = count_down_edges(i);
  new_channel->edges
    = (Edgeptr *) calloc(new_channel->number_of_edges,
                         sizeof(Edgeptr));
  int edge_position = 0;
  for (int j = 0; j < layers[i]->number_of_nodes; j++) {
    Nodeptr current_node = layers[i]->nodes[j];
    for (int k = 0; k < current_node->down_degree; k++) {
      new_channel->edges[edge_position++] = current_node->down_edges[k];
    }
  }
  return new_channel;
}

/**
 * initializes data structures relevant to channels
 */
void initChannels(void) {
  channels
    = (Channelptr *) calloc( number_of_layers, sizeof(Channelptr) );
  for( int i = 1; i < number_of_layers; i++ ) {
    channels[i] = initChannel(i);
  }
}

/**
 * @return the total strech of edges in channel i; assumes the positions of
 * nodes on the two layers have been updated correctly
 */
double totalChannelStretch(int i) {
  double total_stretch = 0.0;
  for ( int j = 0; j < channels[i]->number_of_edges; j++ ) {
    total_stretch += stretch(channels[i]->edges[j]);
  }
  return total_stretch;
}

/**
 * @return the total strech of edges in channel i; assumes the positions of
 * nodes on the two layers have been updated correctly
 */
double maxEdgeStretchInChannel(int i) {
  double max_stretch = 0.0;
  for ( int j = 0; j < channels[i]->number_of_edges; j++ ) {
    double current_stretch = stretch(channels[i]->edges[j]);
    if ( current_stretch > max_stretch ) {
      max_stretch = current_stretch;
    }
  }
  return max_stretch;
}

/**
 * @return the total stretch of all edges
 */
double totalStretch() {
#ifdef DEBUG
  printf("-> totalStretch, iteration = %d\n", iteration);
#endif
  double total_stretch = 0.0;
  for ( int i = 1; i < number_of_layers; i++ ) {
    total_stretch += totalChannelStretch(i);
  }
#ifdef DEBUG
  printf("<- totalStretch, total_stretch = %7.2f\n", total_stretch);
#endif
  return total_stretch;
}

/**
 * @return the maximum stretch among all edges
 */
double maxEdgeStretch() {
#ifdef DEBUG
  printf("-> maxEdgeStretch, iteration = %d\n", iteration);
#endif
  double max_stretch = 0.0;
  for ( int i = 1; i < number_of_layers; i++ ) {
    double channel_stretch = maxEdgeStretchInChannel(i);
    if ( channel_stretch > max_stretch ) {
      max_stretch = channel_stretch;
    }
  }
#ifdef DEBUG
  printf("<- maxEdgeStretch, max_stretch = %7.2f\n", max_stretch);
#endif
  return max_stretch;
}

/**
 * @return the edge with maximum stretch among edges that have not been fixed
 */
Edgeptr maxStretchEdge() {
  Edgeptr max_stretch_edge = NULL;
  double max_stretch = -1.0;
  if ( randomize_order ) {
    genrand_permute(master_edge_list, number_of_edges, sizeof(Edgeptr));
  }
  for ( int i = 0; i < number_of_edges; i++ ) {
    Edgeptr edge = master_edge_list[i];
    double current_stretch = stretch(edge);
    if( current_stretch > max_stretch && ! isFixedEdge( edge ) ) {
      max_stretch = current_stretch;
      max_stretch_edge = edge;
    }
  }
  return max_stretch_edge;
}

/*  [Last modified: 2016 05 19 at 20:49:19 GMT] */
