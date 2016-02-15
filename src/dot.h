/**
 * @file dot.h
 * @brief
 * Module for reading files in .dot format
 * @author Matt Stallmann
 * @date 1998/07/17, modified for new experiments 2008/12/17
 * $Id: dot.h 2 2011-06-07 19:50:41Z mfms $
 */

//     Copyright (C) 2001 Matthias Stallmann.
//     Contact: matt_stallmann@ncsu.edu
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program (file COPYING.txt); if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
//     USA.

#ifndef DOT_H
#define DOT_H

#include<stdio.h>
#include<stdbool.h>

/* -----------  INPUT FUNCTIONS -------------- */

/**
 * Stores information about the dot file and
 * advances the input to the point where edges can be read.
 * NOTE: information in an initial comment is lost
 */
void initDot( FILE * in );

/**
 * Stores the name of the graph in the given buffer. Assumes the buffer is
 * large enough.
 */
void getNameFromDotFile( char * buffer );

/**
 * Reads the next edge from the input, storing the names of the vertices in
 * buffers. Assumes the buffers are large enough.
 * @return true if another edge was found.
 */
bool nextEdge( FILE * in, char * source_buffer, char * destination_buffer );

// The typical way to read a dot file from stream 'in' is ...
//
//     initDot( in );
//     char name_buf[MAX_NAME_LENGTH];
//     get_name( name_buf );
//     // do something with the name
//     char src_buf[MAX_NAME_LENGTH];
//     char dst_buf[MAX_NAME_LENGTH];
//     while ( nextEdge( in, src_buf, dst_buf ) )
//       {
//         // do something with the edge
//       }

/* -----------  OUTPUT FUNCTIONS -------------- */

/**
 * Writes the first part of a dot file to the output stream: comments about
 * how the graph was created and seed information
 */
void dotPreamble( FILE * out,
                  const char * graph_name,
                  const char * initial_comment );

/**
 * Writes the final '}'
 */
void endDot( FILE * out );

/**
 * Writes the edge (src, dst) to the output
 */
void outputEdge( FILE * out,
                 const char * src, const char * dst );

#endif

/*  [Last modified: 2011 04 19 at 20:31:55 GMT] */

