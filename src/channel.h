/**
 * @file channel.h
 * @brief header for utilities that maintain information about channels
 *
 * @todo eventually channels should replace the interlayer structures in
 * crossings.c since can they serve more general purposes; note that the
 * current discipline in crossings.[ch] is that, when an iteration sorts a
 * layer, the crossings for the adjoining channels are updated, including the
 * max crossings node and edge; here, we do this with stretch and leave the
 * crossings updates alone for now.
 */

#include "graph.h"

/**
 * Information about edges in the channel between two layers. Channels are
 * numbered starting at 1 (for the channel between layers 0 and 1).
 */
typedef struct channel_struct {
  int number_of_edges;
  /**
   * Positions on the lower layer of endpoints of the edges; these are sorted
   * lexicographically by the positions of the upper endpoints. Crossings can
   * be determined by counting inversions in a sort by positions of lower endpoints.
   */
  Edgeptr * edges;
} * Channelptr;

/**
 * initializes data structures relevant to channels
 */
void initChannels(void);

/**
 * channels[i] is information about edges between
 * layers i - 1 and i; the entry for i = 0 is not used
 */
extern Channelptr * channels;

/**
 * @return the total strech of edges in channel i; assumes the positions of
 * nodes on the two layers have been updated correctly
 */
double totalChannelStretch(int i);

/**
 * @return the total stretch of edges incident on layer i
 */
double totalLayerStretch(int i);

/**
 * @return the maximum stretch of any edge in channel i; assumes positions of
 * nodes in the two layers have been updated correctly
 */
double maxEdgeStretchInChannel(int i);

/**
 * @return the total stretch of all edges
 */
double totalStretch();

/**
 * @return the maximu stretch of any edge overall
 */
double maxEdgeStretch();

/**
 * @return the edge with maximum stretch among edges that have not been fixed
 */
Edgeptr maxStretchEdge();

/*  [Last modified: 2016 05 20 at 18:39:03 GMT] */
