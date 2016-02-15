/**
 * @file Statistics.c
 * @brief
 * Implements a Statistics class with methods for computing min, median,
 * mean, max, and standard deviation.
 *
 * @author Matt Stallmann
 * @date 2009/05/18
 *
 * Migrate changes back to C-Utilities
 */

#include "Statistics.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

/**
 * Data invariant: the data array remains sorted in increasing order; this is
 * obviously inefficient but this class is not intended for large data sets.
 * The sorting is done via insertion sort, so it will be efficient if the
 * data comes in increasing order.
 */

Statistics init_statistics( int size )
{
#ifdef TEST
  printf( "-> init_statistics, size = %d\n", size );
#endif
  Statistics s = (Statistics) calloc( 1, sizeof( struct statistics_struct ) );
  s->array_size = size;
  s->number_of_data_points = 0;
  s->sum = 0.0;
  s->data = (double *) calloc( size, sizeof( double ) );
#ifdef TEST
  printf( "<- init_statistics, size = %d\n", size );
#endif
  return s;
}

double get_min( Statistics s ) { return s->data[0]; } 

double get_median( Statistics s )
{
  if ( s->number_of_data_points % 2 == 0 )
    // even
    {
      int end_of_first_half = (s->number_of_data_points - 1) / 2;
      int start_of_second_half = s->number_of_data_points / 2;
      return (s->data[ end_of_first_half ]
              + s->data[ start_of_second_half ]) / 2;
    }
  else
    // odd
    {
      return s->data[ s->number_of_data_points / 2 ];
    }
}

double get_mean( Statistics s ) { return s->sum / s->number_of_data_points; }

double get_max( Statistics s ) { return s->data[ s->number_of_data_points - 1 ]; }

double get_standard_deviation( Statistics s )
{
  double sum_of_squares = 0;
  for ( int i = 0; i < s->number_of_data_points; i++ )
    sum_of_squares += s->data[i] * s->data[i];
  double mean = get_mean( s );
  return sqrt( sum_of_squares / s->number_of_data_points
               - mean * mean );
}

int get_number_of_data_points( Statistics s )
{ return s->number_of_data_points; }

/**
 * Initially
 *  - A[0] <= A[1] <= ... <= A[n] and there is room for one more item
 * When insert is done
 *  - A[0] <= ... <= A[i] <= x == A[i+1] <= A[i+2] <= ... <= A[n+1]
 * where the A[0] ... A[i] are as before and A[i+2] ... A[n+1] have the
 * values of A[i+1] ... A[n] 
 */
static void insert( double x, double * A, int n )
{
  int i = n;
  while ( i >= 0 && x < A[i] )
    {
      A[i+1] = A[i];
      i--;
    }
  A[i+1] = x;
}

#ifdef TEST
static void print_data( Statistics s )
{
  fprintf( stderr, "-----\n" );
  fprintf( stderr, "%d |", s->number_of_data_points ); 
  for ( int i = 0; i < s->number_of_data_points; i++ )
    fprintf( stderr, " %5.2lf", s->data[i] );
  fprintf( stderr, "\n" );
  fprintf( stderr, "array_size = %d\n", s->array_size );
  fprintf( stderr, "sum = %5.2lf\n", s->sum );
  fprintf( stderr, "-----\n" );
}
#endif

void add_data( Statistics s, double data_point )
{
#ifdef TEST
  fprintf( stderr, "-> add_data, data_point = %5.2lf\n", data_point );
  print_data( s );
#endif
  assert( s->number_of_data_points < s->array_size );
  s->sum += data_point;
  insert( data_point, s->data, s->number_of_data_points - 1 );
  s->number_of_data_points++;
#ifdef TEST
  fprintf( stderr, "<- add_data, data_point = %5.2lf\n", data_point );
  print_data( s );
#endif
}

void print_statistics( Statistics s, FILE * output_stream, const char * format )
{
  fprintf( output_stream, format, get_min(s) );
  fprintf( output_stream, "\t" );
  fprintf( output_stream, format, get_median(s) );
  fprintf( output_stream, "\t" );
  fprintf( output_stream, format, get_mean(s) );
  fprintf( output_stream, "\t" );
  fprintf( output_stream, format, get_max(s) );
  fprintf( output_stream, "\t" );
  fprintf( output_stream, format, get_standard_deviation(s) );
  fprintf( output_stream, "\t" );
  fprintf( output_stream, "%d", get_number_of_data_points(s) );
}

void free_statistics( Statistics s )
{
  assert( s != NULL );
  if ( s->data != NULL ) free( s->data );
  free( s );
}

#ifdef TEST

/**
 * Simple test program. Reads the number of data points, followed by a list of
 * the data items.
 */
int main()
{
  int n = 0;
  scanf( "%d", &n );
  Statistics s = init_statistics( n ); 
  int i = 0;
  for ( ; i < n ; i++ )
    {
      double data;
      if ( scanf( "%lf", &data ) == EOF ) break;
      add_data( s, data );
    }
/*   printf( "min = \t%5.2f\n", get_min( s ) ); */
/*   printf( "med = \t%5.2f\n", get_median( s ) ); */
/*   printf( "mean = \t%5.2f\n", get_mean( s ) ); */
/*   printf( "max = \t%5.2f\n", get_max( s ) ); */
/*   printf( "stdev = \t%5.2f\n", get_standard_deviation( s ) ); */

  printf( "Stats = \t" );
  print_statistics( s, stdout, "%5.2lf" );
  printf( "\n" );
  free_statistics( s );
}

#endif

//  [Last modified: 2009 05 19 at 13:05:11 GMT]
