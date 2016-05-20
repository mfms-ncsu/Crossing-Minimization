/**
 * @file stats.c
 * @brief Implementation of functions that print statistics
 * @author Matt Stallmann
 * @date 2009/05/19
 *
 * @todo To keep things simple, total stretch is rounded to an integer value
 * $Id: stats.c 131 2016-01-12 01:07:39Z mfms $
 */


#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<math.h>
#include<float.h>

#include"stats.h"
#include"defs.h"
#include"heuristics.h"
#include"graph.h"
#include"min_crossings.h"
#include"crossings.h"
#include"channel.h"
#include"priority_edges.h"
#include"Statistics.h"
#include"timing.h"

typedef struct pareto_item {
  double objective_one;
  double objective_two;
  int iteration;
  struct pareto_item * rest;
} * PARETO_LIST;

static PARETO_LIST pareto_list = NULL;

static void init_pareto_list( void ) { pareto_list = NULL; }

static void print_pareto_list( PARETO_LIST list, FILE * output_stream ) {
  PARETO_LIST local_list = list;
  while ( local_list != NULL ) {
    if ( pareto_objective == BOTTLENECK_TOTAL ) {
      fprintf(output_stream, "%d^%d",
              (int) local_list->objective_one,
              (int) local_list->objective_two);
    }
    else if ( pareto_objective == STRETCH_TOTAL ) {
      fprintf(output_stream, "%f^%d",
              local_list->objective_one,
              (int) local_list->objective_two);
    }
    else { //  pareto_objective == BOTTLENECK_STRETCH
      fprintf(output_stream, "%d^%f",
              (int) local_list->objective_one,
              local_list->objective_two);
    }
    local_list = local_list->rest;
    if ( local_list != NULL ) fprintf(output_stream, ";");
  }
  local_list = list;
  printf(", ");
  while ( local_list != NULL ) {
    fprintf(output_stream, "%d", local_list->iteration);
    local_list = local_list->rest;
    if ( local_list != NULL ) fprintf(output_stream, ";");
  }
} 

/**
 * Inserts, if appropriate, an item with the given values of objective_one and
 * and objective_two into the list representing a Pareto frontier. The frontier
 * is maintained in increasing objective_one, decreasing objective_two order.
 */
static PARETO_LIST pareto_insert(double objective_one,
                                 double objective_two,
                                 int iteration,
                                 PARETO_LIST list) {
#ifdef DEBUG
  printf("-> pareto_insert: %f, %f, %d, ",
         objective_one, objective_two, iteration);
  print_pareto_list(list, stdout);
  printf("\n");
#endif
  PARETO_LIST new_list = NULL;
  if ( list == NULL ) {
    new_list = (PARETO_LIST) calloc(1, sizeof(struct pareto_item));
    new_list->objective_one = objective_one;
    new_list->objective_two = objective_two;
    new_list->iteration = iteration;
    new_list->rest = NULL;
  }
  else {
    double first_objective_one = list->objective_one;
    double first_objective_two = list->objective_two;
    if ( objective_one < first_objective_one
         && objective_two > first_objective_two ) {
      // new pareto point
      new_list = (PARETO_LIST) calloc(1, sizeof(struct pareto_item));
      new_list->objective_one = objective_one;
      new_list->objective_two = objective_two;
      new_list->iteration = iteration;
      new_list->rest = list;
    }
    else if ( objective_one < first_objective_one
              && objective_two == first_objective_two ) {
      // replace first point, found one with smaller objective_one value
      list->objective_one = objective_one;
      list->iteration = iteration;
      new_list = list;
    }
    else if ( objective_one <= first_objective_one
              && objective_two < first_objective_two ) {
      // replace first point with better one; since the new point also has
      // smaller objective_two, it may replace others down the line; in this
      // case, we need to actually delete the existing first point
      new_list = pareto_insert(objective_one,
                               objective_two,
                               iteration,
                               list->rest);
      free(list);
    }
    else if ( objective_one > first_objective_one
              && objective_two < first_objective_two ) {
      // need to keep looking; point with greater or equal objective_one not
      // found
      list->rest = pareto_insert(objective_one,
                                 objective_two,
                                 iteration,                                 
                                 list->rest);
      new_list = list;
    }
    else {
      // otherwise, no need to continue: objective_one >= first_objective_one and
      // objective_two >= first_objective_two
      new_list = list;
    }
  }
#ifdef DEBUG
  printf("<- pareto_insert: ");
  print_pareto_list(new_list, stdout);
  printf("\n");
#endif
  return new_list;
}

CROSSING_STATS_INT total_crossings;
CROSSING_STATS_INT max_edge_crossings;
CROSSING_STATS_INT favored_edge_crossings;
CROSSING_STATS_DOUBLE total_stretch;
CROSSING_STATS_DOUBLE bottleneck_stretch;
Statistics overall_degree;

static void init_specific_crossing_stats_int( CROSSING_STATS_INT * stats,
                                              const char * name )
{
  stats->name = name;
  stats->at_beginning = INT_MAX;
  stats->after_preprocessing = INT_MAX;
  stats->after_heuristic = INT_MAX;
  stats->after_post_processing = INT_MAX;
  stats->best = INT_MAX;
  stats->previous_best = INT_MAX;
  stats->best_heuristic_iteration = -1;
  stats->post_processing_iteration = -1;
}

static void init_specific_crossing_stats_double( CROSSING_STATS_DOUBLE * stats,
                                                 const char * name )
{
  stats->name = name;
  stats->at_beginning = DBL_MAX;
  stats->after_preprocessing = DBL_MAX;
  stats->after_heuristic = DBL_MAX;
  stats->after_post_processing = DBL_MAX;
  stats->best = DBL_MAX;
  stats->previous_best = DBL_MAX;
  stats->best_heuristic_iteration = -1;
  stats->post_processing_iteration = -1;
}

void init_crossing_stats( void )
{
  init_specific_crossing_stats_int( & total_crossings, "Crossings" );
  init_specific_crossing_stats_int( & max_edge_crossings, "EdgeCrossings" );
  init_specific_crossing_stats_double( & total_stretch, "Stretch" );
  init_specific_crossing_stats_double( & bottleneck_stretch, "BottleneckStretch" );
#ifdef FAVORED
  init_specific_crossing_stats( & favored_edge_crossings, "FavoredCrossings" );
#endif
  if ( pareto_objective != NO_PARETO )
    init_pareto_list();
}

void capture_beginning_stats( void )
{
  total_crossings.at_beginning = numberOfCrossings();
  max_edge_crossings.at_beginning = maxEdgeCrossings();
  total_stretch.at_beginning = totalStretch();
  bottleneck_stretch.at_beginning = maxEdgeStretch();
#ifdef FAVORED
  favored_edge_crossings.at_beginning = priorityEdgeCrossings();
#endif
}

void capture_preprocessing_stats( void )
{
  total_crossings.after_preprocessing = numberOfCrossings();
  max_edge_crossings.after_preprocessing = maxEdgeCrossings();
  total_stretch.after_preprocessing = totalStretch();
  bottleneck_stretch.after_preprocessing = maxEdgeStretch();
#ifdef FAVORED
  favored_edge_crossings.after_preprocessing = priorityEdgeCrossings();
#endif
}

void capture_heuristic_stats( void )
{
  total_crossings.after_heuristic = total_crossings.best;
  max_edge_crossings.after_heuristic = max_edge_crossings.best;
  total_stretch.after_heuristic = total_stretch.best;
  bottleneck_stretch.after_heuristic = bottleneck_stretch.best;
#ifdef FAVORED
  favored_edge_crossings.after_heuristic = priority_edge_crossings.best;
#endif
}

void capture_post_processing_stats( void )
{
  // the post processing iterations are merely counted since the total number
  // of crossings improves after each one by definition; not so with
  // bottleneck crossings or stretch
  total_crossings.after_post_processing = total_crossings.best;
  total_crossings.post_processing_iteration = post_processing_iteration;
  max_edge_crossings.after_post_processing = max_edge_crossings.best;
  total_stretch.after_post_processing = total_stretch.best;
  bottleneck_stretch.after_post_processing = bottleneck_stretch.best;
#ifdef FAVORED
  // ditto for favored edge crossings
  favored_edge_crossings.after_post_processing =
    favored_edge_crossings.after_heuristic;
#endif
}

void update_best_int( CROSSING_STATS_INT * stats, Orderptr order,
                      int (* crossing_retrieval_function) (void) )
{
#ifdef DEBUG
  printf("-> update_best_int, %s, %d\n", stats->name, stats->best);
#endif
  int current_value = crossing_retrieval_function();
  if( current_value < stats->best )
    {
      stats->best = current_value;
      stats->best_heuristic_iteration = iteration;
      save_order( order );
    }
#ifdef DEBUG
  printf("<- update_best_int, %s, %d\n", stats->name, stats->best);
#endif  
}

void update_best_double( CROSSING_STATS_DOUBLE * stats, Orderptr order,
                         double (* crossing_retrieval_function) (void) )
{
#ifdef DEBUG
  printf("-> update_best_double, %s, %f\n", stats->name, stats->best);
#endif
  double current_value = crossing_retrieval_function();
  if( current_value < stats->best )
    {
      stats->best = current_value;
      stats->best_heuristic_iteration = iteration;
      save_order( order );
    }
#ifdef DEBUG
  printf("<- update_best_double, %s, %f\n", stats->name, stats->best);
#endif  
}

void update_best_all( void )
{
  update_best_int( & total_crossings, best_crossings_order, numberOfCrossings );
  update_best_int( & max_edge_crossings,
                   best_edge_crossings_order, maxEdgeCrossings );
  update_best_double( & total_stretch, best_total_stretch_order, totalStretch );
  update_best_double( & bottleneck_stretch,
                      best_bottleneck_stretch_order, maxEdgeStretch );
#ifdef FAVORED
  update_best( & favored_edge_crossings, best_favored_crossings_order, priorityEdgeCrossings );
#endif
  if ( pareto_objective == BOTTLENECK_TOTAL )
    pareto_list = pareto_insert( maxEdgeCrossings(),
                                 numberOfCrossings(),
                                 iteration,
                                 pareto_list );
  else if ( pareto_objective == STRETCH_TOTAL )
    pareto_list = pareto_insert( totalStretch(),
                                 numberOfCrossings(),
                                 iteration,
                                 pareto_list );
  else if ( pareto_objective == BOTTLENECK_STRETCH )
    pareto_list = pareto_insert( maxEdgeCrossings(),
                                 totalStretch(),
                                 iteration,
                                 pareto_list );
}

bool has_improved_int( CROSSING_STATS_INT * stats )
{
#ifdef DEBUG
  printf( "-> has_improved_int, stats = %s, best = %d, previous = %d\n",
          stats->name, stats->best, stats->previous_best );
#endif
  bool improved = false;
  if ( stats->best < stats->previous_best ) {
    improved = true;
    stats->previous_best = stats->best;
  }
#ifdef DEBUG
  printf( "<- has_improved, return %d, best = %d, previous = %d, iteration = %d\n",
          improved, stats->best, stats->previous_best, iteration );
#endif
  return improved;
}

bool has_improved_double( CROSSING_STATS_DOUBLE * stats )
{
#ifdef DEBUG
  printf( "-> has_improved_double, stats = %s, best = %f, previous = %f\n",
          stats->name, stats->best, stats->previous_best );
#endif
  bool improved = false;
  if ( stats->best < stats->previous_best ) {
    improved = true;
    stats->previous_best = stats->best;
  }
#ifdef DEBUG
  printf( "<- has_improved_double, return %d, best = %f, previous = %f, iteration = %d\n",
          improved, stats->best, stats->previous_best, iteration );
#endif
  return improved;
}

static int total_layer_degree( int layer )
{
  int total_degree = 0;
  int position = 0;
  for( ; position < layers[ layer ]->number_of_nodes; position++ )
    {
      Nodeptr node = layers[ layer ]->nodes[ position ];
      total_degree += DEGREE( node );
    }
  return total_degree;
}

static void add_layer_degrees( int layer, Statistics s )
{
  int position = 0;
  for( ; position < layers[ layer ]->number_of_nodes; position++ )
    {
      Nodeptr node = layers[ layer ]->nodes[ position ];
      if ( DEGREE( node ) > 0 )
        add_data( s, DEGREE( node ) );
    }
}

static void print_layer_degree_statistics( int layer, FILE * output_stream )
{
  int layer_size = layers[layer]->number_of_nodes;
  Nodeptr * nodes = layers[ layer ]->nodes;
  Statistics layer_degree = init_statistics( layer_size );
  int position = 0;
  for( ; position < layer_size; position++ )
    {
      Nodeptr node = nodes[ position ];
      if ( DEGREE( node ) > 0 )
        add_data( layer_degree, DEGREE( node ) );
    }
  fprintf( output_stream, "NDegree,%3d,", layer );
  print_statistics( layer_degree, output_stream, "%7.2lf" );
  fprintf( output_stream, "\n" );
  free_statistics( layer_degree );
}

static void print_channel_degree_statistics( FILE * output_stream )
{
  Statistics channel_degree_discrepancy
    = init_statistics( number_of_layers - 1 );
  for ( int layer = 1; layer < number_of_layers; layer++ )
    {
      Layerptr upper_layer = layers[layer];
      Layerptr lower_layer = layers[layer - 1];
      int upper_layer_size = upper_layer->number_of_nodes;
      int lower_layer_size = lower_layer->number_of_nodes;
      Statistics channel_degree
        = init_statistics( upper_layer_size + lower_layer_size );
      for ( int i = 0; i < upper_layer_size; i++ )
        {
          Nodeptr node = upper_layer->nodes[i];
          if ( node->down_degree > 0 )
            add_data( channel_degree, node->down_degree );
        }
      for ( int i = 0; i < lower_layer_size; i++ )
        {
          Nodeptr node = lower_layer->nodes[i];
          if ( node->up_degree > 0 )
            add_data( channel_degree, node->up_degree );
        }
      fprintf( output_stream, "CDegree,%3d,", layer );
      print_statistics( channel_degree, output_stream, "%7.2lf" );
      fprintf( output_stream, "\n" );
      add_data( channel_degree_discrepancy,
                get_max( channel_degree ) / get_median( channel_degree ) );
      free_statistics( channel_degree );
    }
  fprintf( output_stream, "AvgCDegreeDisc," );
  print_statistics( channel_degree_discrepancy, output_stream, "%7.2f" );
      fprintf( output_stream, "\n" );
  free_statistics( channel_degree_discrepancy );
}

static void print_channel_edge_counts( FILE * output_stream ) {
  for ( int layer = 1; layer < number_of_layers; layer++ ) {
    Layerptr upper_layer = layers[layer];
    int upper_layer_size = upper_layer->number_of_nodes;
    // count the number of edges into the channel from the upper layer (these
    // are the same as the edges from the lower layer into the channel)
    int edges_from_upper_layer = 0;
    for ( int i = 0; i < upper_layer_size; i++ ) {
      Nodeptr node = upper_layer->nodes[i];
      edges_from_upper_layer += node->down_degree;
    }
    fprintf( output_stream, "EdgesInChannel\t%d\t%d\n",
             layer,
             edges_from_upper_layer );
  }
}

static void compute_degree_statistics( void )
{
  for( int layer = 0; layer < number_of_layers; layer++ )
    {
      for( int position = 0; position < layers[ layer ]->number_of_nodes; position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ position ];
          add_data( overall_degree, DEGREE( node ) );
        }
    }
}

static void print_degree_statistics( FILE * output_stream )
{
  Statistics nodes_per_layer = init_statistics( number_of_layers );
  Statistics layer_degrees = init_statistics( number_of_layers );
  for ( int i = 0; i < number_of_layers; i++ )
    {
      add_layer_degrees( i, overall_degree );
      add_data( nodes_per_layer, layers[i]->number_of_nodes );
      add_data( layer_degrees, total_layer_degree( i ) );
      print_layer_degree_statistics( i, output_stream );
    }
  fprintf( output_stream, "LDegree,%3d,", -1 );
  print_statistics( layer_degrees, output_stream, "%7.2lf" );
  fprintf( output_stream, "\n" );
  fprintf( output_stream, "TDegree,%3d,", -1 );
  print_statistics( overall_degree, output_stream, "%7.2lf" );
  fprintf( output_stream, "\n" );
  fprintf( output_stream, "PerLayerNodes,%3d,", -1 );
  print_statistics( nodes_per_layer, output_stream, "%7.2lf" );
  fprintf( output_stream, "\n" );
  free_statistics( layer_degrees );
  free_statistics( nodes_per_layer );
}

// The following has not been used or tested

/**
 * @todo minimize maximum crossings on any layer
 */
/* static void print_layer_crossing_statistics( int layer, FILE * output_stream ) */
/* { */
/* }  */

void print_graph_statistics( FILE * output_stream )
{
  int effective_number_of_nodes = number_of_nodes - number_of_isolated_nodes;
  fprintf( output_stream, "GraphName,%s\n", graph_name );
  fprintf( output_stream, "NumberOfLayers,%d\n", number_of_layers );
  fprintf( output_stream, "NumberOfNodes,%d\n", number_of_nodes );
  fprintf( output_stream, "IsolatedNodes,%d\n", number_of_isolated_nodes );
  fprintf( output_stream, "EffectiveNodes,%d\n", effective_number_of_nodes );
  fprintf( output_stream, "NumberOfEdges,%d\n", number_of_edges );
  fprintf( output_stream, "EdgeDensity,%2.2f\n",
          (double) number_of_edges / effective_number_of_nodes );
  overall_degree = init_statistics( number_of_nodes );
  if ( verbose )
    {
      print_degree_statistics( output_stream );
      print_channel_degree_statistics( output_stream );
      print_channel_edge_counts( output_stream );
    }
  else
    compute_degree_statistics();
  fprintf( output_stream, "MinDegree,%d\n", (int) get_min( overall_degree ) );
  fprintf( output_stream, "MaxDegree,%d\n", (int) get_max( overall_degree ) );
  fprintf( output_stream, "MeanDegree,%2.2f\n", get_mean( overall_degree ) );
  fprintf( output_stream, "MedianDegree,%2.1f\n", get_median( overall_degree ) );
  free_statistics( overall_degree );
}

static void print_crossing_stats_int(FILE * output_stream,
                                     CROSSING_STATS_INT stats) {
  fprintf( output_stream, "Start%s,%d\n", stats.name, stats.at_beginning );
  fprintf( output_stream, "Pre%s,%d\n", stats.name, stats.after_preprocessing );
  fprintf( output_stream, "Heuristic%s,%d,iteration,%d\n",
           stats.name, stats.after_heuristic, stats.best_heuristic_iteration );
  fprintf( output_stream, "Final%s,%d,iteration,%d\n",
           stats.name, stats.after_post_processing, stats.post_processing_iteration );
}

static void print_crossing_stats_double(FILE * output_stream,
                                        CROSSING_STATS_DOUBLE stats) {
  fprintf( output_stream, "Start%s,%f\n", stats.name, stats.at_beginning );
  fprintf( output_stream, "Pre%s,%f\n", stats.name, stats.after_preprocessing );
  fprintf( output_stream, "Heuristic%s,%f,iteration,%d\n",
           stats.name, stats.after_heuristic, stats.best_heuristic_iteration );
  fprintf( output_stream, "Final%s,%f,iteration,%d\n",
           stats.name, stats.after_post_processing, stats.post_processing_iteration );
}

void print_run_statistics( FILE * output_stream )
{
  fprintf( output_stream, "Preprocessor,%s\n", preprocessor );
  fprintf( output_stream, "Heuristic,%s\n", heuristic );
  fprintf( output_stream, "Iterations,%d\n", iteration );
  fprintf( output_stream, "Runtime,%2.3f\n", RUNTIME );

  print_crossing_stats_int( output_stream, total_crossings );
  print_crossing_stats_int( output_stream, max_edge_crossings );
  print_crossing_stats_double( output_stream, total_stretch );
  print_crossing_stats_double( output_stream, bottleneck_stretch );
#ifdef FAVORED
  print_crossing_stats_int( output_stream, favored_edge_crossings );
#endif
  if ( pareto_objective != NO_PARETO ) {
    fprintf( output_stream, "Pareto,");
    print_pareto_list( pareto_list, output_stream );
    fprintf( output_stream, "\n" );
  }
}

/*  [Last modified: 2016 05 20 at 21:28:41 GMT] */
