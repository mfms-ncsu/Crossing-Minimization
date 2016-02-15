/**
 * @file check_edge_duplication.c
 * @brief utility that uses hashing to check whether an edge already exists
 *
 * $Id: check_edge_duplication.c 2 2011-06-07 19:50:41Z mfms $
 */

#include"check_edge_duplication.h"
#include<stdbool.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

/**
 * Load factor for the bit vector, roughly the likelihood of a collision
 * between distinct edges; also determines the size of the bit vector; small
 * load factor implies large bit vector
 */
#define LOAD_FACTOR 0.1
/** each pair corresponds to a single bit */

static unsigned char * pairs_bit_vector;
static size_t bit_vector_length; 
static int collisions;          /*< Number of collisions during hashing  */
static size_t bits_per_pair;
static size_t bytes_per_pair;

/** two prime numbers that give good results for polynomial hashing */
#define HASH_VALUE_ONE 37
#define HASH_VALUE_TWO 113

/**
 * Uses a polynomial hash function to compute a hash value for a sequence of
 * bytes. If the k bytes are b_{k-1}, ..., b_0 the polynomial P(x) is
 * (b_{k-1} x^{k-1} + ... + b_0 x^0) mod z. The hash function evaluates P(x)
 * at x = some suitably chosen prime number and maps the result to an integer
 * in the range [0,max-1]; the modulus z should be >= max and relatively
 * prime to x (otherwise some integers in the range will never be mapped to)
 *
 * @param start_ptr pointer to b_{k-1}, the first byte in the sequence
 * @param length number of bytes in the sequence, i.e., k
 * @param x the prime number at which P(x) is evaluated
 * @param range_size the number of integers in the range, i.e., max
 * @return an (somewhat random) integer in the interval [0,max-1]; a
 * particular sequence of bytes always gives the same return value
 */
static unsigned long polynomial_hash( const char * start_ptr,
                                      unsigned int length,
                                      unsigned int x,
                                      unsigned long range_size )
{
  unsigned int modulus = range_size;
  /* adjust the modulus to ensure that it's relatively prime to evaluate at */
  if ( modulus % x == 0 || x % modulus == 0 )
    modulus++;
  /* evaluate the polynomial using Horner's rule */
  const char * current_byte = start_ptr;
  unsigned long hash_value = 0;
  while ( current_byte < start_ptr + length ) {
    unsigned int coefficient = (unsigned int) (* current_byte);
    hash_value = (x * hash_value + coefficient) % modulus;
    current_byte++;
  }
  return hash_value % range_size;
}

void create_hash_table_for_pairs( int expected_number_of_pairs )
{
  bits_per_pair = (size_t) round( 1 / LOAD_FACTOR );
  bytes_per_pair = bits_per_pair / sizeof( unsigned char ) + 1; 
  bit_vector_length = expected_number_of_pairs * bytes_per_pair;
  pairs_bit_vector = calloc( expected_number_of_pairs, bytes_per_pair );
}

/**
 * Deallocates the hash table
 */
void destroy_hash_table_for_pairs( void )
{
  free( pairs_bit_vector );
  pairs_bit_vector = NULL;
  printf( "Done adding edges: collisions = %d\n", collisions );
}

/**
 * @param first_int index of a node in the master_node_list
 * @param second_int index of another node in the master_node_list
 * @return true if an edge between the given nodes already exists
 * <em>This function is oblivious to the fact that we are dealing with a
 * random dag - it applies to any collection of pairs of integers</em>
 */
bool pair_already_exists( int first_int, int second_int )
{
#ifdef DEBUG
  printf( "-> pair_already_exists (%d,%d)\n", first_int, second_int );
#endif
  unsigned int hashstr[2];
  // make sure that the hash string will not be 0
  first_int++; second_int++;
  hashstr[0] = (first_int > second_int) ? first_int : second_int;
  hashstr[1] = (first_int < second_int) ? first_int : second_int;
  /* get the bit position and the byte using polynomial hashing*/
  unsigned char bit_position
    = polynomial_hash( (char *) hashstr,
                       2 * sizeof(unsigned int),
                       HASH_VALUE_ONE, sizeof(char) );
  unsigned int byte_position
    = polynomial_hash( (char *) hashstr,
                       2 * sizeof(unsigned int),
                       HASH_VALUE_TWO, bit_vector_length );

  /* check for collision */
  unsigned char bit = 1 << bit_position; 
  if ( ( pairs_bit_vector[ byte_position ] & bit ) )
    {
      collisions++;
#ifdef DEBUG
      printf( "<- pair_already_exists, return true\n" );
#endif
      return true;
    }

  /* Mark the bit in the bit vector and add the edge */
  pairs_bit_vector[ byte_position ] |= bit;
#ifdef DEBUG
  printf( "<- pair_already_exists, return false\n" );
#endif
  return false;
}

/*  [Last modified: 2011 06 02 at 13:56:09 GMT] */
