/**
 * @file median.h
 * @brief interface for various functions related to median heuristics
 * @author Matthias Stallmann
 * @date 2011/06/20
 * $Id: median.h 16 2011-06-20 20:51:06Z mfms $
 */

#ifndef MEDIAN_H
#define MEDIAN_H

#include"defs.h"

/**
 * Assigns weights to nodes on the given layer based on positions of their
 * edges above, below, or both, as specified by the orientation.
 */
void medianWeights( int layer, Orientation orientation );

/**
 * Repeats median heuristic moving upward from the starting layer to the
 * uppermost layer. Orientation of each heuristic application is downward.
 *
 * @return true if max iterations was reached in the process
 */
bool medianUpSweep( int starting_layer );

/**
 * Repeats median heuristic moving downward from the starting layer to the
 * bottom layer, layer 0. Orientation of each heuristic application is upward.
 *
 * @return true if max iterations was reached in the process
 */
bool medianDownSweep( int starting_layer );

#endif

/*  [Last modified: 2011 06 20 at 19:39:19 GMT] */
