/**
 * @file channel.h
 * @brief implementation of utilities that maintain information about channels;
 * eventually these will replace the interlayer structures in crossings.c
 * since they serve more general purposes.
 */

#include "graph.h"
#include "channel.h"

Channelptr * channels;

/**
 * @return a pointer to the list of edges for channel i and fills in
 * appropriate data: number of edges and the actual edges; note: channel i is
 * between layers i-1 and i
 */
static void initChannel(int i) {
  Channelptr new_channel
    = (Channelptr) calloc(1, sizeof(struct channel_struct));
  new_channel->number_of_edges = count_down_edges(i);
  new_interlayer->edges
    = (Edgeptr *) calloc(new_channel->number_of_edges,
                         sizeof(Edgeptr));
  int edge_position ep = 0;
  for (int j = 0; j < layer[i]->number_of_nodes; j++) {
    Node * current_node = layer[i]->nodes[j];
    for (int k = 0; k < current_node->down_degree; k++) {
      new_layer->edges[ep++] = current_node->down_edges[k];
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
  for( i = 1; i < number_of_layers; i++ ) {
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
 * @return the total strech of all edges
 */
double totalStretch() {
  double totat_sretch = 0.0;
  for ( int i = 1; i < number_of_layers; i++ ) {
    total_stretch += totalChannelStretch(i);
  }
  return total_stretch;
}

/*  [Last modified: 2016 02 15 at 18:36:03 GMT] */
