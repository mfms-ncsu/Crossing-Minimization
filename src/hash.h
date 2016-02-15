/**
 * @file hash.h
 * @brief A simple map based on a hash table, maps names to node pointers
 * @author Matthias Stallmann
 * @date 2008/12/21
 * $Id: hash.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef HASH_H
#define HASH_H

#include"defs.h"
#include"graph.h"

/**
 * Initializes the hash table so that it can "comfortably" accommodate the
 * given number of items
 */
void initHashTable( int number_of_items );

/**
 * Inserts A node into the hash table.
 * Assumes that the node is not already present (fatal error otherwise)
 */
void insertInHashTable( const char * name, Nodeptr node );

/**
 * Retrieves a node from the table, given its name
 * @return A pointer to a node with the given name if found, NULL otherwise.
 */
Nodeptr getFromHashTable( const char * name );

/**
 * Deallocates the memory used by the table
 */
void removeHashTable();

/**
 * @return The average number of probes per call to insert or get.
 */
double getAverageNumberOfProbes();

#endif

/*  [Last modified: 2008 12 22 at 15:56:44 GMT] */
