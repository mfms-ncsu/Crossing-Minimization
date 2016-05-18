/**
 * @file stats.h
 * @brief Functions for reporting various statistics about the graph and the
 * outcomes.
 * @author Matthias Stallmann
 * @date 2009/05/19
 * $Id: stats.h 12 2011-06-17 22:00:01Z mfms $
 */

#ifndef STATS_H
#define STATS_H

#include<stdio.h>
#include<stdbool.h>

#include"order.h"

typedef struct crossing_stats_int {
  int at_beginning;
  int after_preprocessing;
  int after_heuristic;
  int after_post_processing;
  int best;
  int previous_best;
  int best_heuristic_iteration;
  int post_processing_iteration;
  const char * name; 
} CROSSING_STATS_INT;

typedef struct crossing_stats_double {
  double at_beginning;
  double after_preprocessing;
  double after_heuristic;
  double after_post_processing;
  double best;
  double previous_best;
  int best_heuristic_iteration;
  int post_processing_iteration;
  const char * name; 
} CROSSING_STATS_DOUBLE;

extern CROSSING_STATS_INT total_crossings;
extern CROSSING_STATS_INT max_edge_crossings;
extern CROSSING_STATS_INT favored_edge_crossings;
extern CROSSING_STATS_DOUBLE total_stretch;
extern CROSSING_STATS_DOUBLE bottleneck_stretch;

/**
 * Initializes crossing stats structures
 */
void init_crossing_stats( void );

/**
 * Saves statistics about crossings before run for later printing
 */
void capture_beginning_stats( void );

/**
 * Saves statistics about crossings after preprocessing for later printing
 */
void capture_preprocessing_stats( void );

/**
 * Saves statistics about crossings after the main heuristic has completed
 * for later printing
 * <em>Assumes that the best heuristic iteration has already been saved and
 * the best heuristic ordering has been restored.</em>
 */
void capture_heuristic_stats( void );

/**
 * Saves statistics about crossings after post processing for later printing
 */
void capture_post_processing_stats( void );

/**
 * Updates the best value if needed, usually at the end of an iteration
 * @param stats the stats struct to be updated
 * @param order the order that needs to be captured when the best is updated
 * @param crossing_retrieval_function function that returns the current
 * value -- to be compared to the best value
 */
void update_best_int( CROSSING_STATS_INT * stats, Orderptr order,
                      int (* crossing_retrieval_function) (void) );

/**
 * Updates the best value if needed, usually at the end of an iteration
 * @param stats the stats struct to be updated
 * @param order the order that needs to be captured when the best is updated
 * @param crossing_retrieval_function function that returns the current
 * value -- to be compared to the best value
 */
void update_best_double( CROSSING_STATS_DOUBLE * stats, Orderptr order,
                         double (* crossing_retrieval_function) (void) );

/**
 * Updates the best value of all stats if needed, i.e., calls update_best on
 * all stats 
 */
void update_best_all( void );

/**
 * @return true if stats.best has improved since the last time this function
 * was called
 *
 * <em>Side effect</e> stats.previous_best is updated
 */
bool has_improved_int( CROSSING_STATS_INT * stats );

/**
 * @return true if stats.best has improved since the last time this function
 * was called
 *
 * <em>Side effect</e> stats.previous_best is updated
 */
bool has_improved_double( CROSSING_STATS_DOUBLE * stats );

/**
 * Print statistics that are intrinsic to the graph, independent of any
 * heuristics 
 */
void print_graph_statistics( FILE * output_stream );

/**
 * Print statistics that document the results of the heuristic just completed
 */
void print_run_statistics( FILE * output_stream );

#endif

/*  [Last modified: 2016 05 18 at 19:59:14 GMT] */
