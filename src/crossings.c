/**
 * @file crossings.c
 * @brief Implementation of functions that keep track of and update the
 * number of crossings for each node, each layer, and for the whole graph.
 *
 * The algorithm used to count crossings between adjacent layers is the
 * O(|E|+|C|) algorithm from "Simple and efficient bilayer cross counting",
 * W. Barth, M. Juenger, P. Mutzel, in JGAA, 2004.
 *
 * @author Matt Stallmann
 * @date 2008/12/23
 * $Id: crossings.c 101 2014-10-22 16:32:05Z mfms $
 */

#include"graph.h"
#include"defs.h"
#include"min_crossings.h"
#include"crossings.h"
#include"crossing_utilities.h"
#include"heuristics.h"
#include"sorting.h"
#include"random.h"

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

/**
 * Information about edges between layers i - 1 and i; the entry for 0 is not
 * used.
 */
typedef struct inter_layer_struct {
  int number_of_edges;
  /**
   * Positions on the lower layer of endpoints of the edges; these are
   * initially sorted lexicographically by the positions of the upper
   * endpoints, then inversions are counted in a sort by positions of the
   * lower endpoints. 
   */
  Edgeptr * edges;
  int number_of_crossings;
} * InterLayerptr;

/**
 * between_layers[i] is information about edges between
 * layers i - 1 and i; the entry for i = 0 is not used
 */
static InterLayerptr * between_layers;

// ******** Allocation functions for initCrossings() ************

static int count_down_edges( int layer_number )
{
  Layerptr layer = layers[ layer_number ];
  int count = 0;
  int j = 0;
  for( ; j < layer->number_of_nodes; j++ )
    {
      count += layer->nodes[j]->down_degree; 
    }
  return count;
}

static InterLayerptr makeInterLayer( int upper_layer )
{
  InterLayerptr new_interlayer
    = (InterLayerptr) malloc( sizeof(struct inter_layer_struct ) );
  new_interlayer->number_of_edges = count_down_edges( upper_layer );
  new_interlayer->edges
    = (Edgeptr *) calloc( new_interlayer->number_of_edges,
                       sizeof(Edgeptr) );
  return new_interlayer;
}

/**
 * Initializes all crossing counts and allocates data structures used for
 * counting crossings. Assumes that the graph has been read from the two
 * input files and all basic data properly initialized - @see readgraph()
 */
void initCrossings( void )
{
  between_layers
    = (InterLayerptr *) calloc( number_of_layers, sizeof(InterLayerptr) );
  int i = 1;
  for( ; i < number_of_layers; i++ )
    {
      between_layers[i] = makeInterLayer( i );
    }
}

/**** Other functions ********/

int numberOfCrossings( void )
{
  int i = 1;
  int crossings = 0;
  for( ; i < number_of_layers; i++ )
    {
      crossings += between_layers[i]->number_of_crossings;
    }
  return crossings;
}

int maxEdgeCrossings( void )
{
  return maxCrossingsEdgeStatic()->crossings;
}

/**
 * @return the number of crossings for the given layer
 */
int numberOfCrossingsLayer( int layer )
{
  int crossings = 0;
  if( layer > 0 )
    crossings += between_layers[ layer ]->number_of_crossings;
  if( layer < number_of_layers - 1 )
    crossings += between_layers[ layer + 1 ]->number_of_crossings;
  return crossings;
}

/**
 * @return the number of crossings for the given node
 */
int numberOfCrossingsNode( Nodeptr node )
{
  return node->up_crossings + node->down_crossings;
}

/**
 * @return the number of crossings for the given edge
 */
int numberOfCrossingsEdge( Edgeptr edge )
{
  return edge->crossings;
}

void updateAllCrossings( void )
{
  updateAllPositions();
  for( int i = 1; i < number_of_layers; i++ )
    {
      updateCrossingsBetweenLayers( i );
    }
}

void updateCrossingsForLayer( int layer )
{
  updateNodePositions( layer );
  if( layer > 0 ) updateCrossingsBetweenLayers( layer );
  if( layer < number_of_layers - 1 )
    updateCrossingsBetweenLayers( layer + 1 );
}

void updatePositionsForLayer( int layer )
{
  Layerptr layer_ptr = layers[ layer ];
  for ( int i = 0; i < layer_ptr->number_of_nodes; i++ )
    {
      Nodeptr node = layer_ptr->nodes[i];
      node->position = i;
    }
}

/**
 * Sets all node crossings relevant to the edges between layers upper_layer
 * and upper_layer - 1 to 0
 */
static void initialize_crossings( int upper_layer )
{
  Layerptr up_layer = layers[ upper_layer ];
  Layerptr down_layer = layers[ upper_layer - 1 ];
  Nodeptr * upper_nodes = up_layer->nodes;
  Nodeptr * lower_nodes = down_layer->nodes;
  int upper_node_count = up_layer->number_of_nodes;
  int lower_node_count = down_layer->number_of_nodes;
  int i = 0;
  for( ; i < upper_node_count; i++ )
    {
      upper_nodes[i]->down_crossings = 0;
      int j = 0;
      for( ; j < upper_nodes[i]->down_degree; j++ )
        {
          upper_nodes[i]->down_edges[j]->crossings = 0;
        }
    }
  for( ; i < lower_node_count; i++ )
    {
      lower_nodes[i]->up_crossings = 0;
    } 
}

/**
 * Updates crossings between two adjacent layers. Also updates the relevant
 * crossing fields of the two layers.
 * @param upper_layer the higher of the two layers; crossings between layers
 * upper_layer - 1 and upper_layer are counted
 */
void updateCrossingsBetweenLayers( int upper_layer )
{
  // sort edges lexicographically based primarily on upper layer endpoints
  Layerptr layer = layers[ upper_layer ];
  int index = 0;                /* current index into edge array */
  int upper_position = 0;
  for( ; upper_position < layer->number_of_nodes; upper_position++ )
    {
      Nodeptr node = layer->nodes[upper_position];
      sortByDownNodePosition( node->down_edges, node->down_degree );
      add_edges_to_array( between_layers[ upper_layer ]->edges,
                          node->down_edges, node->down_degree, index );
      index += node->down_degree;
    }
  initialize_crossings( upper_layer );
  between_layers[ upper_layer ]->number_of_crossings
    = count_inversions_down( between_layers[ upper_layer ]->edges,
                             between_layers[ upper_layer ]->number_of_edges,
                             1 );
}

int maxCrossingsLayer( void ) {
  int max_crossings_layer = -1;
  int max_crossings = -1;
  int * layer_sequence = (int *) malloc( number_of_layers * sizeof(int) );
  for ( int i = 0; i < number_of_layers; i++ ) {
    layer_sequence[i] = i;
  }
  if ( randomize_order ) {
    genrand_permute( layer_sequence, number_of_layers, sizeof(int) );
  }
  for( int i = 0; i < number_of_layers; i++ ) {
    int layer = layer_sequence[i];
    if( numberOfCrossingsLayer( layer ) > max_crossings 
        && ! isFixedLayer( layer ) ) {
          max_crossings = numberOfCrossingsLayer( layer );
          max_crossings_layer = layer;
    }
  }
  free( layer_sequence );
  return max_crossings_layer;
}

Nodeptr maxCrossingsNode( void ) {
  if ( randomize_order ) {
    genrand_permute( master_node_list, number_of_nodes, sizeof(Nodeptr) );
  }
  Nodeptr max_crossings_node = NULL;
  int max_crossings = -1;
  for ( int i = 0; i < number_of_nodes; i++ ) {
    Nodeptr node = master_node_list[i];
#ifdef DEBUG
    printf( " loop: maxCrossingsNode, i =%d, node = %s\n", i, node->name );
#endif
    if( numberOfCrossingsNode( node ) > max_crossings 
        && ! isFixedNode( node) ) {
      max_crossings = numberOfCrossingsNode( node );
      max_crossings_node = node;
    }
  }
  return max_crossings_node;
}

Edgeptr maxCrossingsEdge( void ) {
  Edgeptr max_crossings_edge = NULL;
  int max_crossings = -1;
  if ( randomize_order ) {
    genrand_permute( master_edge_list, number_of_edges, sizeof(Edgeptr) );
  }
  for ( int i = 0; i < number_of_edges; i++ ) {
    Edgeptr edge = master_edge_list[i];
    if( edge->crossings > max_crossings 
        && ! isFixedEdge( edge ) ) {
      max_crossings = edge->crossings;
      max_crossings_edge = edge;
    }
  }
  return max_crossings_edge;
}

Edgeptr maxCrossingsEdgeStatic( void ) {
  Edgeptr max_crossings_edge = NULL;
  int max_crossings = -1;
  if ( randomize_order ) {
    genrand_permute( master_edge_list, number_of_edges, sizeof(Edgeptr) );
  }
  for ( int i = 0; i < number_of_edges; i++ ) {
    Edgeptr edge = master_edge_list[i];
    if( edge->crossings > max_crossings ) { 
      max_crossings = edge->crossings;
      max_crossings_edge = edge;
    }
  }
  return max_crossings_edge;
}

/**
 * Prints down crossings for the nodes on layer i
 */
static void print_down_crossings_nodes( int i )
{
  Layerptr layer = layers[ i ];
  int j = 0;
  for( ; j < layer->number_of_nodes; j++ )
    {
      Nodeptr node = layer->nodes[j];
      printf( "    %-10s layer = %3d, position = %3d, down_x = %3d\n",
              node->name, node->layer, node->position, node->down_crossings );
    }
}

/**
 * Prints the crossings for the down edges of nodes on layer i
 */
static void print_down_crossings_edges( int i )
{
  Layerptr layer = layers[ i ];
  int j = 0;
  for( ; j < layer->number_of_nodes; j++ )
    {
      Nodeptr node = layer->nodes[j];
      int edge_position = 0;
      for( ; edge_position < node->down_degree; edge_position++ )
        {
          Edgeptr edge = node->down_edges[ edge_position ];
          printf( " ::  %10s -> %10s has %4d crossings\n",
                  edge->down_node->name, edge->up_node->name, edge->crossings );
        }
    }
}

/**
 * Prints up crossings for the nodes on layer i
 */
void print_up_crossings_nodes( int i )
{
  Layerptr layer = layers[ i ];
  int j = 0;
  for( ; j < layer->number_of_nodes; j++ )
    {
      Nodeptr node = layer->nodes[j];
      printf( "    %-10s layer = %3d, position = %3d,   up_x = %3d\n",
              node->name, node->layer, node->position, node->up_crossings );
    }
}

/**
 * Prints information for crossings between layers i-1 and i
 */
void print_crossings_between_layers( int i )
{
  printf( "  --- between layers %d and %d crossings = %3d\n",
          i - 1, i, between_layers[i]->number_of_crossings );
  printf( "    ___ upper nodes\n" );
  print_down_crossings_nodes( i );
  printf( "    ^^^ lower nodes\n" );
  print_up_crossings_nodes( i - 1 );
  printf( "  ---\n" );
}

/**
 * Prints information about the crossings, mostly for debugging
 */
void printCrossings( void )
{
  printf( "xxx total_crossings = %d\n", numberOfCrossings() );
  int i = 1;
  for( ; i < number_of_layers; i++ )
    {
      print_crossings_between_layers( i );
    }
  printf("->-> edge crossings\n");
  i = 1;
  for( ; i < number_of_layers; i++ )
    {
      print_down_crossings_edges( i );
    }
}

#ifdef TEST

#include"graph_io.h"

// the following are to avoid bringing in more modules than necessary
int capture_iteration;
int max_iterations;
void barycenterDownSweep(int layer) {}
void barycenterUpSweep(int layer) {}

int main( int argc, char * argv[] )
{
  readGraph( argv[1], argv[2] );
  initCrossings();
  updateAllCrossings();
  printCrossings();
  fprintf( stderr, "total crossings = %d\n", numberOfCrossings() );
  // test maximum crossings node
  printf("\nMax crossings nodes:\n");
  clearFixedNodes();
  Nodeptr node = maxCrossingsNode();
  for( ; node != NULL; node = maxCrossingsNode() )
    {
      printf( "max crossings node = %s, crossings = %d\n",
              node->name, numberOfCrossingsNode( node ) );
      fixNode( node );
    }
  // test maximum crossings edge
  printf("\nMax crossings edges:\n");
  clearFixedEdges();
  Edgeptr edge = maxCrossingsEdge();
  for( ; edge != NULL; edge = maxCrossingsEdge() )
    {
      printf( "max crossings edge: %s -> %s, crossings = %d\n",
              edge->down_node->name, edge->up_node->name,
              numberOfCrossingsEdge( edge ) );
      fixEdge( edge );
    }
  // test maximum crossings layer
  printf("\nMax crossings layers:\n");
  clearFixedLayers();
  int layer = maxCrossingsLayer();
  for( ; layer != -1; layer = maxCrossingsLayer() )
    {
      printf( "max crossings layer = %d, crossings = %d\n",
              layer, numberOfCrossingsLayer( layer ) );
      fixLayer( layer );
    }
  return 0;
}

#endif

/*  [Last modified: 2014 10 22 at 16:05:26 GMT] */
