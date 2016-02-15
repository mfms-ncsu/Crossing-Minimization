/**
 * @file Statistics.h
 * @brief
 * Interface for a Statistics class with methods for computing min, median,
 * mean, max, and standard deviation.
 *
 * @author Matt Stallmann
 * @date 2009/05/18
 *
 * Migrate changes back to C-Utilities
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>

typedef struct statistics_struct {
  int array_size;
  int number_of_data_points;
  double sum;
  double * data;
} * Statistics;

/**
 * @param size the maximum number of data items
 * @return a new instance of the class
 * @todo allow for unlimited capacity
 */
Statistics init_statistics( int size );

double get_min( Statistics s );
double get_median( Statistics s );
double get_mean( Statistics s );
double get_max( Statistics s );
double get_standard_deviation( Statistics s );
int get_number_of_data_points( Statistics s );
void add_data( Statistics s, double data_point );

/**
 * Prints statistics in the form (tab separated items)
 *  min median mean max stdev N
 * @param format the printf format to use for each statistic, e.g. "%5.2f"
 */
void print_statistics( Statistics s, FILE * output_stream, const char * format );

/**
 * deallocates data structures for s
 */
void free_statistics( Statistics s );

#endif

//  [Last modified: 2011 06 02 at 21:41:49 GMT]
