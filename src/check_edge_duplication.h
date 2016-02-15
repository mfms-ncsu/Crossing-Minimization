/**
 * @file check_edge_duplication.h
 * @brief utility that uses hashing to check whether an edge already exists
 *
 * $Id: check_edge_duplication.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef CHECK_EDGE_DUPLICATION_H
#define CHECK_EDGE_DUPLICATION_H

#include<stdbool.h>

/**
 * Allocates the hash table based on the expected number of pairs
 */
void create_hash_table_for_pairs( int expected_number_of_pairs );

/**
 * Deallocates the hash table
 */
void destroy_hash_table_for_pairs( void );

/**
 * @param first_int index of a node in the master_node_list
 * @param second_int index of another node in the master_node_list
 * @return true if an edge between the given nodes already exists
 * <em>This function is oblivious to the fact that we are dealing with a
 * random dag - it applies to any collection of pairs of integers</em>
 *
 * <strong>Side effect:</strong> If the pair does not already exist, it will
 * exist from henceforth
 */
bool pair_already_exists( int first_int, int second_int );

#endif

/*  [Last modified: 2011 06 01 at 21:32:28 GMT] */
