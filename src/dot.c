/**
 * @file dot.cxx
 * @brief
 * Module for reading files in .dot format
 * @author Matt Stallmann
 * @date 1998/07/17, adapted for new Crossings experiments, 2008/12/17
 * @compile test program: gcc -g -Wall dot.c -o test_dot
 * $Id: dot.c 2 2011-06-07 19:50:41Z mfms $
 */

//     Copyright (C) 2008 Matthias Stallmann.
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

#include<stdlib.h>              // abort()
#include<stdio.h>
#include<string.h>
#include<assert.h>
#include<ctype.h>

#include"dot.h"
#include"defs.h"

#define MAX_MSG_LENGTH 512
#define MAX_NAME_LENGTH 512

static char error_message[MAX_MSG_LENGTH];
static char graph_name[MAX_NAME_LENGTH];
static int line_number = 1;

/* -----------  UTILITY FUNCTIONS -------------- */

/**
 * Prints the message in 'error_message' with the line number
 * @param fatal if true, program will exit (fatal error)
 */
static void error( bool fatal )
{
  fprintf( stderr, "Line %d: %s\n", line_number, error_message );
  if( fatal ) exit( EXIT_FAILURE );
}

/**
 * Skips blanks and comments in the input stream using a finite-state
 * machine. Also updates the line number when appropriate.
 * @param ch the starting character for the sequence, as provided by the
 * client
 * @return the character that ends the sequence of blanks and comments, to be
 * used by the client
 */
static int skip_blanks_and_comments( int ch, FILE * in_stream )
{
  enum state_type { BLANK, C_COMMENT, CPP_COMMENT, SLASH, STAR } state = BLANK;
#ifdef DEBUG
  printf( "-> skip_blanks_and_comments, ch = %d\n", ch ); 
#endif
  bool done = false;
  while( ! done )
    {
      switch( ch ) {
      case '\n':
        line_number++;
        if( state == CPP_COMMENT ) { state = BLANK; break; }
      case ' ': case '\t': case '\r':
        if( state == SLASH )
          {
            // the slash was not the beginning of a comment and therefore ends
            // the sequence
            ungetc( ch, in_stream );
            ch = '/';
            done = true;
          }
        else if( state == STAR ) state = C_COMMENT;
        break;
      case '/':
        if( state == BLANK ) state = SLASH;
        else if( state == SLASH ) state = CPP_COMMENT;
        else if( state == STAR ) state = BLANK;
        break;
      case '*':
        if( state == C_COMMENT ) state = STAR;
        else if( state == SLASH ) state = C_COMMENT;
        else if( state == STAR ) state = C_COMMENT;
        else if( state == BLANK ) done = true;
        break;
      case EOF:
        done = true;
      default:
        if( state == SLASH )
          {
            // the slash was not the beginning of a comment and therefore ends
            // the sequence
            ungetc( ch, in_stream );
            ch = '/';
            done = true;
          }
        else if( state == STAR ) state = C_COMMENT;
        else if( state == BLANK ) done = true;
        break;
      } // end, switch
#ifdef DEBUG
      printf( " skipping, ch = %d, state = %d\n", ch, state );
#endif      
      if( ! done ) ch = getc( in_stream );
    } // end, infinite loop
#ifdef DEBUG
  printf( "<- skip_blanks_and_comments, ch = %d\n", ch ); 
#endif
  return ch;
}

/**
 * Reads an identifier from the input stream into the buffer
 * @param ch the beginning of the identifier
 * @return the character after the identifier
 */
static char read_identifier( int ch, FILE * in, char * id_buf )
{
  int index = 0;
  while( isalnum( ch ) || ch == '_' ) {
    id_buf[ index++ ] = ch;
    ch = getc( in );
  }
  id_buf[index] = '\0';
  return ch;
}

/* -----------  INPUT FUNCTIONS -------------- */

void initDot( FILE * in )
{
  int ch = getc( in );
  ch = skip_blanks_and_comments( ch, in );
  /** @todo eventually will want to pick up the first comment */
  ungetc( ch, in );
  char digraph[MAX_NAME_LENGTH];
  fscanf( in, "%s", digraph );
  if( strcmp( digraph, "digraph" ) != 0 )
    {
      sprintf( error_message, "expected 'digraph', got '%s'", digraph );
      error( true );
    }
  fscanf( in, "%s", graph_name );
  ch = getc( in );
  ch = skip_blanks_and_comments( ch, in );
  if( ch != '{' )
    {
      sprintf( error_message, "expected '{', got %c", ch );
      error( true );
    }
}

void getNameFromDotFile( char * buffer )
{
  strcpy( buffer, graph_name );
}

bool nextEdge( FILE * in, char * src_buf, char * dst_buf )
{
  char ch = getc( in );
  ch = skip_blanks_and_comments( ch, in );
  if( ch == EOF || ch == '}' ) return false;
  ch = read_identifier( ch, in, src_buf ); 
  if( ch == EOF )
    {
      sprintf( error_message, "premature end of file" );
      error( true );
    }
  ch = skip_blanks_and_comments( ch, in );
  int dash = ch;
  int arrow_head = getc( in );
  if( dash != '-' || arrow_head != '>' )
    {
      sprintf( error_message, "expected '->', got '%c%c'", dash, arrow_head );
      error( true );
    }
  ch = getc( in );
  ch = skip_blanks_and_comments( ch, in );
  ch = read_identifier( ch, in, dst_buf ); 
  if( ch == EOF )
    {
      sprintf( error_message, "premature end of file" );
      error( true );
    }
  ch = skip_blanks_and_comments( ch, in );
  if( ch != ';' )
    {
      sprintf( error_message, "expected ';', got '%c'", ch );
      error( true );
    }
  return true;
}

/* -----------  OUTPUT FUNCTIONS -------------- */

void dotPreamble( FILE * out, const char * graph_name,
                   const char * seed_info )
{
  fprintf( out, "/* %s */\n", seed_info );
  fprintf( out, "digraph %s {\n", graph_name );
}

void endDot( FILE * out )
{
  fprintf( out, "}\n" );
}

void outputEdge( FILE * out, const char * src_name,
                 const char * dst_name )
{
  fprintf( out, " %s -> %s;\n", src_name, dst_name );
}

#ifdef TEST

/**
 * Test program: reads a dot file from standard input and sends it to
 * standard output.
 */
static void test_dot()
{
  initDot( stdin );
  char name_buf[100];
  getName( name_buf );
  fprintf( stderr, "name = %s\n", name_buf );
  dotPreamble( stdout, name_buf, "seed" );
  char src_buf[100];
  char dst_buf[100];
  while ( nextEdge( stdin, src_buf, dst_buf ) ) {
    fprintf( stderr, "src = %s, dst = %s\n", src_buf, dst_buf );
    outputEdge( stdout, src_buf, dst_buf );
  }
  endDot( stdout );
}

int main()
{
  test_dot();
  return 0;
}

#endif

/*  [Last modified: 2011 04 19 at 20:48:01 GMT] */
