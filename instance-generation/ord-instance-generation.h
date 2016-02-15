/// @file ord-instance-generation.h
/// @brief header for utility functions that read and write .ord files (node
///          ordering on layers of a graph)
/// @author Matt Stallmann
/// @date 29 Dec 1998
/// $Id: ord-instance-generation.h 19 2011-06-23 01:46:27Z mfms $

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

// 20 May 2002 - added get_graph_name()
// 09 Sep 2008 - modified as a C implementation

// Standard format for an ord file is (comments begin with # and end with
// newline):
//
// # Ordering for graph graph_name
// # method of generation ('natural' order, treatment, or random seed)
//
// 0 {
//   node_0_0 node_0_1 ... node_0_m
// }
// ...
// k {
//   node_k_0 ... node_k_n
// }
//
// where there are k+1 layers numbered 0 through k
//
// To read a .ord file via the stream 'in', do the following:
//     int layer;
//     while ( nextLayer( in, & layer ) ) {
//       // initialize the layer
//       char * node = 0;
//       while ( nextNode( in, & node ) ) {
//         // do something with this node
//       }
//     }

#ifndef ORD_H
#define ORD_H

#include<stdio.h>
#include<stdbool.h>

// sizes of important buffers (need to be #define so that they can be used
// for global variable declarations
#define NAME_LENGTH  511    // maximum length of a node name
#define LINE_LENGTH  75     // maximum length of a line in a .ord file when
                            // producing output

bool getGraphName( FILE * in, char * buffer );
// PRE: 'in' is a valid istream for a .ord file
//      'buffer' is large enough to hold a graph name
// POST: 'in' is beyond the initial comments and white space
//       'buffer' contains the name of the graph, if any (last word of the
//       first comment line in the stream)
//       retval == true iff there was a nonblank comment before there was
//                 any meaningful input 

bool nextLayer( FILE * in, int * layer );
// PRE: 'in' is a valid istream for a .ord file
// POST: 'in' is at the first node of the next layer (if any)
//       'layer' == the number of the next layer (if any)
//       retval == true iff there is another layer

bool nextNode( FILE * in, char * * node );
// PRE: 'in' is a valid istream for a .ord file and is positioned somewhere
//      inside the list of nodes for a layer
// POST: 'in' is beyond the next node (if any)
//       'node' is an allocated string, the name of the next node (if any)
//       retval == true iff there is another node on the current layer

// output functions

void ordPreamble( FILE * out, const char * graph_name,
                   const char * generation_method );
// PRE: 'out' is a valid ostream
// POST: comments identifying the graph and the way it was generated have
// been written on 'out'

void beginLayer( FILE * out, int layer );
// PRE: 'out' is a valid ostream
// POST: the encoding for the start of 'layer' has been written on 'out' 

void endLayer( FILE * out );
// PRE: 'out' is a valid ostream
// POST: the encoding for the end of the current layer has been written on
//       'out' 

void outputNode( FILE * out, const char * node );
// PRE: 'out' is a valid ostream
// POST: the 'node' is listed next for the current layer on 'out' 

#endif

/*  [Last modified: 2011 06 22 at 19:58:14 GMT] */
