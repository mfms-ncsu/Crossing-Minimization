/**
 * @file randomNumbers.c
 * @brief Implementation of C-utilities that are based on the
 *         IEEE-48 random number standard.
 * @author Matt Stallmann
 * @date 19 Feb 2003
 * $Id: randomNumbers.c 9 2011-06-13 01:29:18Z mfms $
 * [Part of my C-Utilities library -- any updates should be propagated
 *  back to there]
 */

// Uncomment the line below if running on VCL RH Enterprise Linux 5
// #define NO_RAND48 

#include<stdlib.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>              /* memcpy() */
#include<assert.h>
#include"randomNumbers.h"

#if ! defined(RAND48)
extern double   erand48(unsigned short[3]); 
extern long     nrand48(unsigned short[3]);
#endif

/** to keep track of the current seed */
static unsigned short my_seed[ 3 ] = {0, 0, 0};

void RN_setSeed( unsigned short seed[] ) {
  int i = 3;
  while( i > 0 ) {
    --i;
    my_seed[ i ] = seed[ i ];
  }
}

void RN_setRandomSeed(void) {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  my_seed[0] = current_time.tv_sec;
  my_seed[1] = current_time.tv_usec;
  my_seed[2] = getpid();
}

const unsigned short * RN_getSeed( void ) {
  return my_seed;
}

int RN_integer( int lb, int ub ) {
  assert( lb <= ub );
  return lb + nrand48( my_seed ) % (ub - lb + 1);
}

double RN_real( double lb, double ub ) {
  assert( lb <= ub );
  double random_double = erand48( my_seed );
#ifdef DEBUG
  fprintf( stderr, "RN_real: lb = %g, ub = %g, random_double = %g\n",
           lb, ub, random_double );
#endif
  return lb + (ub - lb) * random_double;
}

bool RN_boolean( double p ) {
  assert( 0 <= p && p <= 1 );
  return ( erand48( my_seed ) < p );
}

void RN_permute( void * A, int length, int element_size ) {
  int i = length;
  void * temp = malloc( element_size );
  while ( --i > 0 ) {
    /* swap A[i] with a random element among A[0],...,A[i] */
    int j = RN_integer( 0, i );
    if ( j != i ) {
      memcpy( temp, A + (i * element_size), element_size );
      memcpy( A + (i * element_size),  A + (j * element_size), element_size );
      memcpy( A + (j * element_size), temp, element_size );
    }
  }
}

int * RN_permutation( void * A, int length, int element_size ) {
  int i;
  void * temp = malloc( element_size );
  int * retval = malloc( length * sizeof( int ) );
  int tmp = 0;

  /* initialize retval to be the identity permutation */
  for( i = 0; i < length; ++i ) {
    retval[ i ] = i;
  }

  i = length;
  while ( --i > 0 ) {
    /* swap A[i] with a random element among A[0],...,A[i] */
    int j = RN_integer( 0, i );
    if ( j != i ) {
      memcpy( temp, A + (i * element_size), element_size );
      memcpy( A + (i * element_size),  A + (j * element_size), element_size );
      memcpy( A + (j * element_size), temp, element_size );

      tmp = retval[ i ];
      retval[ i ] = retval[ j ];
      retval[ j ] = tmp;
    }
  }
  return retval;
}

/*  [Last modified: 2008 09 17 at 20:57:45 GMT] */
