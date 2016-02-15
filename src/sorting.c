/**
 * @file sorting.c
 * @brief Interface for functions that perform various sorts. All sorts are
 * stable.
 * @author Matthias Stallmann
 * @date 2009/01/03
 * $Id: sorting.c 76 2014-07-21 21:10:32Z mfms $
 */

#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>              /* memcpy() */

#include"sorting.h"
#include"graph.h"

/**
 * Performs an insertion sort using the same argument types as qsort
 * @return true if the original order has changed
 */
static bool insertion_sort(void *base, size_t nmemb, size_t size,
                           int (*compar)(const void *, const void *)) {
  bool changed = false;
  // used to store the item to be inserted
  void * tmp = malloc(size);
  int i = 1;
  for( ; i < nmemb; i++ ) {
    // 1. insert the A[i] among A[0],...,A[i-1]
    //    (a) copy A[i] into tmp
    memcpy( tmp, base + i * size, size );
    //    (b) find largest j for which A[j] <= tmp (or -1 if none exists),
    //    shifting elements to the right as you go
    int j = i - 1;
    while( j >= 0 && compar( tmp, base + j * size) < 0 ) {
      changed = true;
      memcpy( base + (j + 1) * size, base + j * size, size );
      j--;
    }

    //    (c) copy tmp into A[i+1]
    memcpy( base + (j + 1) * size, tmp, size ); 
  }
  free( tmp );
  return changed;
}

/**
 * Performs an "unstable" insertion sort using the same argument types as
 * qsort; unstable means that elements with equal keys will be put in reverse
 * of their original order
 * @return true if the original order has changed
 */
static bool unstable_insertion_sort(void *base, size_t nmemb, size_t size,
                                    int (*compar)(const void *, const void *)) {
  bool changed = false;
  // used to store the item to be inserted
  void * tmp = malloc(size);
  int i = 1;
  for( ; i < nmemb; i++ ) {
    // 1. insert the A[i] among A[0],...,A[i-1]
    //    (a) copy A[i] into tmp
    memcpy( tmp, base + i * size, size );
    //    (b) find largest i for which A[i] <= tmp (or -1 if none exists),
    //    shifting elements to the right as you go
    int j = i - 1;
    while( j >= 0 && compar( tmp, base + j * size) <= 0 ) {
      changed = true;
      memcpy( base + (j + 1) * size, base + j * size, size );
      j--;
    }

    //    (c) copy tmp into A[i+1]
    memcpy( base + (j + 1) * size, tmp, size ); 
  }
  free( tmp );
  return changed;
}

/**
 * Comparison function to be used by qsort or insertion_sort to compare the
 * weights of two nodes. Assumes that each array element is a pointer to a
 * node. Insertion sort is preferred in most cases because it is stable (and
 * usually does not increase the asymptotic time).
 */
static int compare_weights( const void * ptr_i, const void * ptr_j ) {
  Nodeptr * entry_ptr_i = (Nodeptr *) ptr_i;
  Nodeptr * entry_ptr_j = (Nodeptr *) ptr_j;
  Nodeptr node_i = * entry_ptr_i;
  Nodeptr node_j = * entry_ptr_j;
  if( node_i->weight > node_j->weight ) return 1;
  else if( node_i->weight < node_j->weight ) return -1;
  else return 0;
}

/**
 * Comparison function to be used by qsort to compare degrees of nodes.
 */
static int compare_degrees( const void * ptr_i, const void * ptr_j ) {
  Nodeptr * entry_ptr_i = (Nodeptr *) ptr_i;
  Nodeptr * entry_ptr_j = (Nodeptr *) ptr_j;
  Nodeptr node_i = * entry_ptr_i;
  Nodeptr node_j = * entry_ptr_j;
  int node_i_degree = node_i->up_degree + node_i->down_degree;
  int node_j_degree = node_j->up_degree + node_j->down_degree;
  if( node_i_degree > node_j_degree ) return 1;
  else if( node_i_degree < node_j_degree ) return -1;
  else return 0;
}

/**
 * Comparison function to be used by qsort to compare the weights of two
 * nodes. Assumes that each array element is a pointer to a node.
 */
static int compare_down_edges( const void * ptr_i, const void * ptr_j ) {
  Edgeptr * entry_ptr_i = (Edgeptr *) ptr_i;
  Edgeptr * entry_ptr_j = (Edgeptr *) ptr_j;
  Edgeptr edge_i = * entry_ptr_i;
  Edgeptr edge_j = * entry_ptr_j;
  if( edge_i->down_node->position > edge_j->down_node->position ) return 1;
  else if( edge_i->down_node->position < edge_j->down_node->position )
    return -1;
  else return 0;
}

/**
 * Comparison function to be used by qsort to compare the weights of two
 * nodes. Assumes that each array element is a pointer to a node.
 */
static int compare_up_edges( const void * ptr_i, const void * ptr_j ) {
  Edgeptr * entry_ptr_i = (Edgeptr *) ptr_i;
  Edgeptr * entry_ptr_j = (Edgeptr *) ptr_j;
  Edgeptr edge_i = * entry_ptr_i;
  Edgeptr edge_j = * entry_ptr_j;
  if( edge_i->up_node->position > edge_j->up_node->position ) return 1;
  else if( edge_i->up_node->position < edge_j->up_node->position )
    return -1;
  else return 0;
}

void updateAllPositions( void )
{
  for( int i = 0; i < number_of_layers; i++ )
    {
      updateNodePositions( i );
    }
}

void updateNodePositions( int layer )
{
  Layerptr layerptr = layers[layer];
  int i = 0;
  for( ; i < layerptr->number_of_nodes; i++ )
    {
      layerptr->nodes[i]->position = i;
    }
}

void layerSort( int layer )
{
  Layerptr layer_ptr = layers[ layer ];
#ifdef DEBUG
  printf( "before layerSort: ");
  for ( int i = 0; i < layer_ptr->number_of_nodes; i++ ) {
    printf( "%3d/%3.1f" , layer_ptr->nodes[i]->id, layer_ptr->nodes[i]->weight );
  }
  printf( "\n" );
#endif
  insertion_sort( layer_ptr->nodes, layer_ptr->number_of_nodes,
                  sizeof( Nodeptr ), compare_weights );
#ifdef DEBUG
  printf( "after layerSort:  ");
  for ( int i = 0; i < layer_ptr->number_of_nodes; i++ ) {
    printf( "%3d/%3.1f" , layer_ptr->nodes[i]->id, layer_ptr->nodes[i]->weight );
  }
  printf( "\n" );
#endif
  updateNodePositions( layer );
}

void layerUnstableSort( int layer )
{
  Layerptr layer_ptr = layers[ layer ];
  unstable_insertion_sort( layer_ptr->nodes, layer_ptr->number_of_nodes,
                           sizeof( Nodeptr ), compare_weights );
  updateNodePositions( layer );
}

void layerQuicksort( int layer )
{
  Layerptr layer_ptr = layers[ layer ];
  qsort( layer_ptr->nodes, layer_ptr->number_of_nodes,
         sizeof( Nodeptr ), compare_weights );
  updateNodePositions( layer );
}

/**
 * Sort the array of edges by the positions of the nodes on the lower layer
 */
void sortByDownNodePosition( Edgeptr * edge_array, int num_edges )
{
  insertion_sort( edge_array, num_edges, sizeof(Edgeptr),
                  compare_down_edges );
}

/**
 * Sort the array of edges by the positions of the nodes on the upper layer
 */
void sortByUpNodePosition( Edgeptr * edge_array, int num_edges )
{
  insertion_sort( edge_array, num_edges, sizeof(Edgeptr),
                  compare_up_edges );
}

void sortByDegree( Nodeptr * node_array, int num_nodes )
{
  qsort( node_array, num_nodes, sizeof(Nodeptr), compare_degrees );
}

/*  [Last modified: 2014 07 21 at 18:21:49 GMT] */
