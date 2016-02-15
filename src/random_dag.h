/**
 * @file random_dag.h
 * @brief Module for creating a random dag with a given number of nodes and
 * layers.
 *
 * @author Matt Stallmann
 * @date 2011/06/01
 * $Id: random_dag.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef RANDOM_DAG_H
#define RANDOM_DAG_H

/**
 * Creates a random dag with the given number of nodes and layers. Assumes
 * that the nodes and layers of the dag have already been created, and that
 * the master_edge_list contains the edges of the backbone tree
 *
 * @param branching_factor the number of chidren of a tree node is a random
 * number in the range [1 .. branching_factor]; a large branching factor
 * means that the variance in degree will be larger. Since the tree forms the
 * backbone of the random dag, the same observation applies to the dag
 */
void create_random_dag( int num_nodes,
                        int num_edges,
                        int num_layers,
                        int branching_factor
                        );

#endif

/*  [Last modified: 2011 06 01 at 18:58:36 GMT] */
