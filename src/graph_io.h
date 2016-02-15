/**
 * @file graph.h
 * @brief Definition of data structures and access functions for a layered
 * graph.
 * @author Matt Stallmann, based on Saurabh Gupta's crossing heuristics
 * implementation.
 * @date 2008/12/19
 * $Id: graph_io.h 90 2014-08-13 20:31:25Z mfms $
 */

#include<stdbool.h>

#ifndef GRAPH_IO_H
#define GRAPH_IO_H

#include"graph.h"

/**
 * Reads the graph from the given files, specified by their names. Each file
 * is read twice so that arrays can be allocated to the correct size on the
 * first pass. Also initializes all graph-related data structures and global
 * variables.
 */
void readGraph( const char * dot_file, const char * ord_file );

/**
 * Prints the graph in a verbose format on standard output for debugging
 * purposes. May also be used for piping to a graphical trace later.
 */
void printGraph();

/**
 * Writes the current layer orderings to an ord file with the given name.
 */
void writeOrd( const char * ord_file );

/**
 * Writes a dot file with the given name.
 * @param dot_file_name the output file name (including .dot extension)
 * @param header_information what will go into the file before the '{' that
 * starts the list of edges (including the graph name)
 * @param edge_list array of (indices of) the edges to be written
 * @param edge_list_length length of the edge list
 */
void writeDot( const char * dot_file_name,
               const char * graph_name,
               const char * header_information,
               const Edgeptr * edge_list,
               int edge_list_length
               );

/**
 * Renumbers nodes, i.e., assigns id's, based on their order of appearance in
 * a dot file. This is useful if converting to sgf format -- see
 * dot_and_ord_to_sgf.c; the node number will not depend on the order in the
 * ord file and different orderings will have the same node numbers so that
 * they can be compared.
 */
void renumberNodesUsingDotFile( const char * dot_file );

#endif

/*  [Last modified: 2014 08 13 at 19:19:40 GMT] */
