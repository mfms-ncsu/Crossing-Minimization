/**
 * @file dfs.h
 * @brief interface for function that assigns weights based on depth-first
 * search
 * @author Matthias Stallmann
 * @date 2008/01/03
 * $Id: dfs.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef DFS_H
#define DFS_H

/**
 * Assigns weights to nodes based on their preorder number in a depth-first
 * search that starts at the first node on the lowest layer.
 */
void assignDfsWeights( void );

#endif

/*  [Last modified: 2009 01 03 at 20:44:57 GMT] */
