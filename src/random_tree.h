/**
 * @file random_tree.h
 * @brief Module for creating a random tree with a given number of nodes and
 * layers.
 *
 * @author Matt Stallmann
 * @date 2011/05/30
 * $Id: random_tree.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef RANDOM_TREE_H
#define RANDOM_TREE_H

/**
 * Creates a random tree with the given number of nodes and layers.
 *
 * @param branching_factor the number of chidren of a node is a random number
 * in the range [1 .. branching_factor]; a large branching factor means that
 * the variance in degree will be larger. 
 */
void create_random_tree( int num_nodes,
                         int num_layers,
                         int branching_factor
                         );

#endif

/*  [Last modified: 2011 06 01 at 18:59:33 GMT] */
