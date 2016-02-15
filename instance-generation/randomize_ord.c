/**
 * @file randomize_ord.c
 * @brief reads an ord file and outputs an equivalent one with nodes of each
 * layer randomly ordered
 * 
 * @author Matt Stallmann
 * @date 2008/09/13
 * $Id: randomize_ord.c 9 2011-06-13 01:29:18Z mfms $
 */

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include"randomNumbers.h"
#include"ord-instance-generation.h"

#define MAX_NODES_PER_LAYER 8192

static void printUsage( const char * progname )
{
  printf( "Usage: %s input_file output_file seed1,seed2,seed3\n", progname );
  printf( "       where seed1, seed2, and seed3 are the 3 16-bit\n" );
  printf( "       integers for the IEEE 48 random number generator\n" );
  printf( "Output is the three seeds at the end, separated by commas\n" );
}

/**
 * converts a string of three numbers separated by commas to an array of
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
  char graph_name[NAME_LENGTH + 1];
  getGraphName( in_stream, graph_name );
  char seed_comment[NAME_LENGTH + 1];
  sprintf( seed_comment, " randomly permuted using seed %s", seed_string );
  ordPreamble( out_stream, graph_name, seed_comment );

  // read the input one layer at a time, permuting and sending it to the
  // output
  int layer;
  char * node_array[ MAX_NODES_PER_LAYER ];
  while ( nextLayer( in_stream, & layer ) )
    {
      // read the layer into node_array
      int number_of_nodes = 0;
      char * node = NULL;
      while ( nextNode( in_stream, & node ) )
        {
          node_array[ number_of_nodes++ ] = node;
        }
      // randomly permute it
      RN_permute( node_array, number_of_nodes, sizeof(char *) );

      // output the layer
      beginLayer( out_stream, layer );
      for( int i = 0; i < number_of_nodes; i++ )
        outputNode( out_stream, node_array[i] );
      endLayer( out_stream );
      

      // free up the space for the nodes -- these were allocated by nextNode()
      for( int i = 0; i < number_of_nodes; i++ )
        {
          free( node_array[i] );
          node_array[i] = NULL;
        }
    }


  // close files (assume no errors)
  fclose( in_stream );
  fclose( out_stream );

  // output the new seed
  const unsigned short * new_seed = RN_getSeed();
  printf( "%hu,%hu,%hu\n", new_seed[0], new_seed[1], new_seed[2] );
  
  return EXIT_SUCCESS;
}

/*  [Last modified: 2011 06 08 at 21:28:25 GMT] */
