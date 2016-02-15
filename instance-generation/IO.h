/**
 * @file IO.h
 * @brief Functions that deal with layered graph input and output (output
 * only for now)
 * @author Matt Stallmann
 * @date 2008/07/23
 * $Id: IO.h 9 2011-06-13 01:29:18Z mfms $
 */

#ifndef IO_H
#define IO_H

#include "LayeredGraph.h"
#include <stdio.h>

/**
 * @todo Need a way to add header comments to these files and give the graph
 * a name.  The latter can be part of the LayeredGraph module.  Even the
 * former could be with some thought.
 */

/**
 * Writes a dot file
 * @param dot_file_stream the stream for the file
 * @param G the graph to be encoded by the dot file
 */
void writeDot( FILE * dot_file_stream, Graph G );

/**
 * Writes an ord file
 * @param ord_file_stream the stream for the file
 * @param G the graph whose layers are to be listed in the ord file
 */
void writeOrd( FILE * ord_file_stream, Graph G );

#endif

/*  [Last modified: 2008 07 23 at 21:19:16 GMT] */
