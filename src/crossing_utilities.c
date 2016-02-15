/**
 * @file crossings.c
 * @brief Implementation of functions that are used to count and update
 * crossings locally.
 *
 * @author Matt Stallmann
 * @date 2009/05/15
 * $Id: crossing_utilities.c 56 2014-03-13 21:12:01Z mfms $
 */

#include"graph.h"
#include"defs.h"
#include"crossing_utilities.h"
#include"sorting.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#ifdef DEBUG
/**
 * Prints an edge (for debugging)
 */
static void print_edge( Edgeptr edge )
{
  printf("[%d, %d]", edge->up_node->position, edge->down_node->position );
}

/**
 * Prints the current sequence of edges between the layers - used for
 * debugging
 */
static void print_edge_array( Edgeptr * edge_array, int number_of_edges )
{
  int i = 0;
  for( ; i < number_of_edges; i++ )
    {
      print_edge( edge_array[i] );
      printf("\n");
    }
}
#endif

/**
 * Updates crossings for edges and their endpoints when two edges form an
 * inversion.
 *
 * @param diff indicates whether to increment the number of crossings for
 * nodes and edges (+1) or decrement them (-1)
 */
static void update_crossings( Edgeptr edge_one, Edgeptr edge_two, int diff )
{
  edge_one->crossings += diff;
  edge_two->crossings += diff;
  Nodeptr up_node_one = edge_one->up_node;
  Nodeptr up_node_two = edge_two->up_node;
  Nodeptr down_node_one = edge_one->down_node;
  Nodeptr down_node_two = edge_two->down_node;
  up_node_one->down_crossings += diff;
  up_node_two->down_crossings += diff;
  down_node_one->up_crossings += diff;
  down_node_two->up_crossings += diff;
}

int insert_and_count_inversions_down( Edgeptr * edge_array,
                                      int starting_index,
                                      int diff )
{
  int number_of_crossings = 0;
  int index = starting_index - 1;
  Edgeptr edge_to_insert = edge_array[starting_index];
  while( index >= 0
         && edge_array[index]->down_node->position
         > edge_to_insert->down_node->position )
    {
      number_of_crossings++;
      update_crossings( edge_array[index], edge_to_insert, diff );
      edge_array[index + 1] = edge_array[index];
      index--;
    }
  edge_array[index + 1] = edge_to_insert;
  return number_of_crossings;
}

int count_inversions_down( Edgeptr * edge_array, int number_of_edges, int diff )
{
#ifdef DEBUG
  printf("-> count_inversions_down\n");
  printf( " edge array for upper layer %d:\n",
          edge_array[0]->up_node->layer );
  print_edge_array( edge_array, number_of_edges );
#endif
  int number_of_inversions = 0;
  int i = 1;
  for( ; i < number_of_edges; i++ )
    {
      number_of_inversions
        += insert_and_count_inversions_down( edge_array, i, diff );
    }
#ifdef DEBUG
  printf("<- count_inversions_down, number = %d\n", number_of_inversions);
  printf( " edge array for upper layer %d:\n", 
          edge_array[0]->up_node->layer );
  print_edge_array( edge_array, number_of_edges );
#endif
  return number_of_inversions;
}

int insert_and_count_inversions_up( Edgeptr * edge_array,
                                    int starting_index, int diff )
{
  int number_of_crossings = 0;
  int index = starting_index - 1;
  Edgeptr edge_to_insert = edge_array[starting_index];
  while( index >= 0
         && edge_array[index]->up_node->position
         > edge_to_insert->up_node->position )
    {
      number_of_crossings++;
      update_crossings( edge_array[index], edge_to_insert, diff );
      edge_array[index + 1] = edge_array[index];
      index--;
    }
  edge_array[index + 1] = edge_to_insert;
  return number_of_crossings;
}

int count_inversions_up( Edgeptr * edge_array, int number_of_edges, int diff  )
{
#ifdef DEBUG
  printf("-> count_inversions_up\n");
  print_edge_array( edge_array, number_of_edges );
#endif
  int number_of_inversions = 0;
  int i = 1;
  for( ; i < number_of_edges; i++ )
    {
      number_of_inversions
        += insert_and_count_inversions_up( edge_array, i, diff );
    }
#ifdef DEBUG
  printf("<- count_inversions_up, number = %d\n", number_of_inversions);
  print_edge_array( edge_array, number_of_edges );
#endif
  return number_of_inversions;
}

void add_edges_to_array( Edgeptr * edge_array, Edgeptr * edges_to_add,
                         int num_edges, int start_pos )
{
  int edges_added = 0;
  for( ; edges_added < num_edges; edges_added++ )
    {
      edge_array[ start_pos + edges_added ] = edges_to_add[ edges_added ];
    }
}

/*  [Last modified: 2014 03 10 at 16:39:24 GMT] */
