/**
 * @file randomize_dot.c
 * @brief reads a dot file and outputs an equivalent one with the edges
 * listed in random order
 * 
 * Assumes a simplified .dot format:
 *
 * <pre>
 * comments
 * digraph NAME {
 *   v_1 -> w_1;
 *   ...
 *   v_m -> w_m; 
 * }
 * </pre>
 *
 * @author Matt Stallmann
 * @date 2008/10/23
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<limits.h>
#include<assert.h>
#include"randomNumbers.h"

#define MAX_TOKEN_SIZE 128
#define MAX_DOT_LINE 30
/**
 * @todo expand array of edges automatically
 */
#define MAX_EDGES 200000

/**
 * @todo The following functions should eventually go into a separate module.
 */

void readDotPreamble( FILE * input, char * graph_name )
{
  char buffer[ MAX_TOKEN_SIZE + 1 ];
  bool end_of_preamble = false;
  while( ! end_of_preamble )
    {
      fscanf( input, "%s", buffer );
      if( strcmp( buffer, "digraph" ) == 0 ) end_of_preamble = true;
    }
  fscanf( input, "%s", graph_name );
  // read to the end of the line
  fgets( buffer, MAX_TOKEN_SIZE + 1, input );
}

/**
 * Puts the whole line representing the edge into the buffer
 * @return true if there was an edge, false if the final } was encountered
 */
bool readEdge( FILE * input, char * edge_array_position )
{
  fgets( edge_array_position, MAX_DOT_LINE, input );
  if( * edge_array_position == '}' ) return false;
  return true;
}

void writeDotPreamble( FILE * output,
                       const char * graph_name, const char * comment )
{
  fprintf( output, "/* %s */\n", comment );
  fprintf( output, "digraph %s {\n", graph_name );
}

/**
 * End of 'module' for reading/writing .dot files
 */

static void printUsage( const char * progname )
{
  printf( "Usage: %s input_file output_file seed1,seed2,seed3\n", progname );
  printf( "       where seed1, seed2, and seed3 are the 3 16-bit\n" );
  printf( "       integers for the IEEE 48 random number generator\n" );
  printf( "Output is the three seeds at the end, separated by commas\n" );
}

/**
 * converts a string of three numbers separtated by commas to an array of
 * three unsigned short integers
 * @param seed_string the input string
 * @param seed_array the output array (should have room for 3 entries)
 */
static void parseSeed( const char * seed_string, unsigned short * seed_array )
{
  unsigned long temp_seed[ 3 ];
  int number_read = sscanf( seed_string, "%lu,%lu,%lu", 
                            temp_seed, temp_seed + 1, temp_seed + 2 );
  if( number_read != 3 ) {
    fprintf( stderr, "Improper format for seed: %s\n", seed_string );
    fprintf( stderr, "Should be seed1,seed2,seed3\n" );
    exit(1);
  }
  for( int i = 0; i < 3; ++i ) {
    if( temp_seed[ i ] > USHRT_MAX ) {
      fprintf( stderr, "Seed %lu is too large (limit = %u)\n",
               temp_seed[ i ], USHRT_MAX );
      exit(1);
    }
    seed_array[ i ] = (unsigned short) temp_seed[ i ];
  }
}

/**
 * Opens the file with the given name for input; reports the error and exits
 * if unable to do so.
 * @return the stream associated with the file
 */
FILE * openInput( const char * name )
{
  FILE * stream = fopen( name, "r" );
  if( stream == NULL )
    {
      printf("Unable to open file %s for reading.\n", name);
      exit(1);
    }
  return stream;
}

/**
 * Opens the file with the given name for output; reports the error and exits
 * if unable to do so.
 * @return the stream associated with the file
 */
FILE * openOutput( const char * name )
{
  FILE * retval = fopen( name, "w" );
  if( retval == NULL )
    {
      printf("Unable to open file %s for writing.\n", name);
      exit(1);
    }
  return retval;
}

int main( int argc, char * argv [] )
{
  unsigned short seed[ 3 ];

  // get command line arguments
  if( argc != 4 )
    {
      printUsage( argv[0] );
      return EXIT_FAILURE;
    }

  const char * input_file_name = argv[ 1 ];
  const char * output_file_name = argv[ 2 ];
  const char * seed_string = argv[ 3 ];
  parseSeed( seed_string, seed );
  RN_setSeed( seed );

  // open files
  FILE * in_stream = openInput( input_file_name );
  FILE * out_stream = openOutput( output_file_name );

  // read the header from the input file and add a comment with seed before
  // passing it to the output file
  char graph_name[ MAX_TOKEN_SIZE ];
  readDotPreamble( in_stream, graph_name );
  char seed_comment[MAX_TOKEN_SIZE ];
  sprintf( seed_comment, " randomly permuted using seed %s", seed_string );
  /**
   * @todo Use a real graph name
   */
  writeDotPreamble( out_stream, graph_name, seed_comment );

#ifdef DEBUG
  printf( "\nDone with preamble, graph_name = %s, seed_comment = %s\n",
          graph_name, seed_comment );
#endif

  char * edges = (char *) malloc( MAX_EDGES * MAX_DOT_LINE * sizeof(char) );
  assert( edges || "allocation of edge array failed" );

#ifdef DEBUG
  printf( "Start reading edges\n" );
#endif

  int num_edges = 0;

  // read input one edge at a time
  while ( readEdge( in_stream, edges + num_edges * MAX_DOT_LINE ) )
    {
      num_edges++;
      if( num_edges  >= MAX_EDGES )
        {
          printf("Too many edges! Limit is %d\n", MAX_EDGES);
          return EXIT_FAILURE;
        }
    }
#ifdef DEBUG
  printf( "Done reading edges, num_edges = %d\n", num_edges );
#endif
  
  // randomly permute the edges
  RN_permute( edges, num_edges, MAX_DOT_LINE );

  // write the edges to the output file
  int i = 0;
  for( ; i < num_edges; i++ )
    {
      fprintf( out_stream, "%s", edges + i * MAX_DOT_LINE );
    }

  fprintf( out_stream, "}\n" );

  // close files (assume no errors) and free memory
  fclose( in_stream );
  fclose( out_stream );
  free( edges );
  

  // output the new seed
  const unsigned short * new_seed = RN_getSeed();
  printf( "%hu,%hu,%hu\n", new_seed[0], new_seed[1], new_seed[2] );
  
  return EXIT_SUCCESS;
}

/*  [Last modified: 2011 11 08 at 20:04:15 GMT] */
