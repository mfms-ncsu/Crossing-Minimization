/**
 * @file barycenter.h
 * @brief interface for various functions related to barycenter heuristics
 * @author Matthias Stallmann
 * @date 2008/12/29
 * $Id: barycenter.h 12 2011-06-17 22:00:01Z mfms $
 */

#ifndef BARYCENTER_H
#define BARYCENTER_H

#include"defs.h"

/**
 * Assigns weights to nodes on the given layer based on positions of their
 * edges above, below, or both, as specified by the orientation.
 */
void barycenterWeights( int layer, Orientation orientation );

/**
 * Repeats barycenter heuristic moving upward from the starting layer to the
 * uppermost layer. Orientation of each heuristic application is downward.
 *
 * @return true if max iterations was reached in the process
 */
bool barycenterUpSweep( int starting_layer );

/**
 * Repeats barycenter heuristic moving downward from the starting layer to the
 * bottom layer, layer 0. Orientation of each heuristic application is upward.
 *
 * @return true if max iterations was reached in the process
 */
bool barycenterDownSweep( int starting_layer );

#endif

/*  [Last modified: 2011 06 17 at 21:16:42 GMT] */
