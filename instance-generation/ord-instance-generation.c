/// @file ord-instance-generation.c
/// @brief implementation of utility functions that read and write .ord
///           files (node ordering on layers of a graph)
/// @author Matt Stallmann
/// @date 1 Jan 1999
///
/// $Id: ord-instance-generation.c 25 2011-07-05 16:20:07Z mfms $

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

#include"ord-instance-generation.h"
#include<stdlib.h>              // abort()
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<assert.h>

const char * ORD_SUFFIX = ".ord";
                                // suffix for ordering files

// characters with special meaning
const char NULL_CHAR = '\0';
const char BLANK_CHAR = ' ';
const char COMMENT_CHAR = '#';
const char END_OF_LINE = '\n';
const char OPEN_LIST = '{';
const char CLOSE_LIST = '}';

static char name_buffer[NAME_LENGTH + 1]; // used to report graph name
static bool valid_name = false;        // true if an actual name was found

static bool eatSpaceAndComments( FILE * in )
  // POST: 'in' is at the first non-blank character after its initial
  //       position (with comments excluded);
  //       retval == true iff there is another non-blank character before
  //                 the end of file 
{
  static bool first_comment = true; // name appears at the end of first comment
  enum { NOT_IN_COMMENT, IN_COMMENT } local_state = NOT_IN_COMMENT;
  int ch;
  int index = 0;
  while ( (ch = getc( in )) != EOF ) {
#ifdef DEBUG
    cerr << "..local state = " << local_state << ", ch = " << ch << endl;

#endif
    switch ( local_state ) {
    case NOT_IN_COMMENT:
      if ( COMMENT_CHAR == ch ) local_state = IN_COMMENT;
      else if ( ! isspace( ch ) ) {
        ungetc( ch, in );
        return true;
      }
      break;
    case IN_COMMENT:
      if ( END_OF_LINE == ch ) local_state = NOT_IN_COMMENT;
      // if this is the first comment line, save the last "word" -- it's
      // the name of the graph.
      if( first_comment ) {
        if ( END_OF_LINE == ch ) {
          name_buffer[index] = NULL_CHAR;
          if( index > 0 ) {
            valid_name = true;
#ifdef DEBUG
            cerr << "....valid name [" << name_buffer << "]" << endl;
#endif
          }
          first_comment = false;
        }
        else if ( BLANK_CHAR == ch ) {
          index = 0;
        }
        else {
          name_buffer[index++] = ch;
#ifdef DEBUG
          cerr << "....add to buffer " << ch << ", index = " << index << endl;
#endif
        }
      } // if this is the first comment
      break;
    default: assert( "bad local state" && false );
    }
  }
  return false;
}

bool getGraphName( FILE * in, char * buffer ) {
  eatSpaceAndComments( in );
#ifdef DEBUG
  cerr << "<- get_graph_name returns [" << name_buffer << "], valid = " <<
    valid_name << endl;
#endif
  if( valid_name ) {
    strcpy( buffer, name_buffer );
    return true;
  }
  return false;
}

static enum { OUTSIDE_LAYER,
              LAYER_NUMBER,
              INSIDE_LAYER } state = OUTSIDE_LAYER;

static int hold_layer = -1;     // most recent layer number encountered

bool nextLayer( FILE * in, int * layer )
{
  * layer = -1;
  int ch;
  while ( eatSpaceAndComments( in ) && (ch = getc( in )) != EOF ) {
#ifdef DEBUG
    cerr << "..state = " << state << ", ch = " << ch << endl;
#endif
    switch ( state ) {
      /**
       * @todo Need friendlier error messages if input is messed up
       */
    case OUTSIDE_LAYER:
      ungetc( ch, in );         // put back first digit of expected int
      fscanf( in, "%d", layer );
      hold_layer = * layer;
      state = LAYER_NUMBER;
      break;
    case LAYER_NUMBER:
      if ( OPEN_LIST == ch ) {
        state = INSIDE_LAYER;
        assert( * layer >= 0 );
        return true;
      }
      else {
        fprintf( stderr, "\nRead error in .ord file: %d"
                 " expected, reading %c instead.", OPEN_LIST, ch );
        abort();
      }
      break;
    case INSIDE_LAYER:
      if ( CLOSE_LIST == ch )
        state = OUTSIDE_LAYER; 
      break;
    default: assert( "bad state" && false );
    }
  }
  return false;
}

bool nextNode( FILE * in, char * * node )
{
  assert( INSIDE_LAYER == state );
  if ( ! eatSpaceAndComments( in ) ) {
    fprintf( stderr, "Read error in .ord file: unexpected end of file\n"
             "while reading nodes in layer %d\n", hold_layer );
    abort();
  }
  char buffer[ NAME_LENGTH + 1 ];
  int index = 0;
  int ch;
  while ( (ch = getc( in )) != EOF ) {
#ifdef DEBUG
    cerr << "..state = " << state << ", ch = " << ch << endl;
#endif
    if ( CLOSE_LIST == ch || COMMENT_CHAR == ch || isspace( ch ) ) {
      ungetc( ch, in );
      if ( 0 == index )
        return false;
      buffer[ index ] = '\0';
      * node = (char *) malloc( strlen( buffer ) + 1 );
      strcpy( * node, buffer );
      return true;
    }
    else {
      assert( index < NAME_LENGTH );
      buffer[ index++ ] = ch;
    }
  }
  assert( false );              // should never get here
  return false;
}

static int current_column = 0;  // keeps track of column while printing
static int number_of_nodes = 0; // number of nodes on current line
static int output_layer = -1;   // current layer during output

void ordPreamble( FILE * out, const char * graph_name,
                   const char * generation_method )
{
  fprintf( out, "# Ordering for graph %s\n", graph_name );
  fprintf( out, "# %s\n\n", generation_method );
}

void beginLayer( FILE * out, int layer )
{
  fprintf( out, "# Random order for layer %d\n", layer );
  fprintf( out, "%d {\n", layer );
  output_layer = layer;
  current_column = 0;
  number_of_nodes = 0;
}

void endLayer( FILE * out )
{
  assert( 0 <= output_layer );
  if ( 0 < number_of_nodes ) fprintf( out, "\n" );
  fprintf( out, "} # end of layer %d\n\n", output_layer );
  output_layer = -1;
}

void outputNode( FILE * out, const char * node )
{
  assert( 0 <= output_layer );
  if ( 0 < number_of_nodes
       && LINE_LENGTH <= current_column + (int) strlen( node ) ) {
    fprintf( out, "\n" );
    current_column = 0;
    number_of_nodes = 0;
  }
  if ( 0 < number_of_nodes ) {
    fprintf( out, " " );
    ++current_column;
  }
  fprintf( out, "%s", node );
  current_column += strlen( node );
  ++number_of_nodes;
}

//  [Last modified: 2011 06 28 at 15:37:12 GMT]
