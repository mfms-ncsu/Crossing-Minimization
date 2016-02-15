/**
 * @file LayeredGraph.c
 * @brief Functions that allow creation and manipulation of a layered graph.
 * @author Matt Stallmann
 * @date 2008/07/17
 * $Id: LayeredGraph.c 9 2011-06-13 01:29:18Z mfms $
 */

#include "LayeredGraph.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * Original size of all dynamic arrays, i.e., arrays whose size is increased
 * by doubling as needed. This should be a power of 2 to take advantage of
 * the allocation strategy of most systems.
 */
#define INITIAL_SIZE 2

/**
 * Creates a duplicate copy of a string. Some C implementations have the
 * function strdup() for this but it's not in the standard. Important: the
 * new string has to be deallocated at some point.
 */
#define CLONE_STRING( target, source )\
  target = malloc( strlen(source) + 1 );\
  strcpy( target, source );

/**
 * Generic macro for doubling the amount of space for an array.
 * @param type The type of each array entry (hence the need for a macro).
 * @param size The original array size
 * @param pointer_to_start Pointer to the beginning of the array.
 * @todo Does not work: always complains about "parse error before ')' token"
 */
#define DOUBLE_ARRAY( element_type, size, pointer_to_start )   \
  size *= 2;                                                   \
  pointer_to_start                                             \
  = ((element_type) *)                                         \
    realloc( pointer_to_start, size * sizeof(element_type) );  \
  assert( pointer_to_start );

struct node
{
  char * name;
  int layer;
};

/** Array of nodes on a layer */
typedef struct layer
{
  Node * node_array;
  size_t node_array_size;
  int number_of_nodes;
} * Layer;

/**
 * For the purpose of generating random graphs, a graph only needs to
 * "memorize" its nodes and edges.
 */
struct graph
{
  char * name;
  char * how_created;
  /** array of layers */
  Layer * layer_array;
  int number_of_layers;
  Edge * edge_array;
  size_t edge_array_size;
  int number_of_edges;
};

/**
 * Deallocates the node and its name. No need to know the graph here.
 */
static void destroyNode( Node the_node )
{
  free( the_node->name );
  free( the_node );
}

/**
 * Creates a new layer and initializes its fields
 */
static Layer createLayer( void )
{
  Layer new_layer = (Layer) calloc( 1, sizeof(struct layer));
  new_layer->node_array_size = INITIAL_SIZE;
  new_layer->node_array = (Node *) calloc( INITIAL_SIZE, sizeof(Node) );
  new_layer->number_of_nodes = 0;
  return new_layer;
}

/**
 * Deallocates a layer, including anything allocated within it
 */
static void destroyLayer( Layer layer_ptr )
{
  for( int i = 0; i < layer_ptr->number_of_nodes; i++ )
    {
      destroyNode( layer_ptr->node_array[i] );
    }
  free( layer_ptr->node_array );
  free( layer_ptr );
}

Graph createGraph( int number_of_layers,
                   const char * name,
                   const char * how_created )
{
  Graph new_graph = calloc( 1, sizeof(struct graph) );
  new_graph->layer_array = (Layer *) calloc( number_of_layers, sizeof(Layer) );
  new_graph->number_of_layers = number_of_layers;
  for( int k = 0; k < number_of_layers; k++ )
    {
      new_graph->layer_array[k] = createLayer();
    }
  new_graph->edge_array_size = INITIAL_SIZE;
  new_graph->edge_array = (Edge *) calloc( INITIAL_SIZE, sizeof(Edge) ); 
  new_graph->number_of_edges = 0;
  CLONE_STRING( new_graph->name, name );
  CLONE_STRING( new_graph->how_created, how_created );
  return new_graph;
}

void destroyGraph( Graph G )
{
  for( int k = 0; k < G->number_of_layers; k++ )
    {
      destroyLayer( G->layer_array[k] );
    }
  free( G->layer_array );
  free( G->edge_array );
  free( G );
}

int getNumberOfLayers( Graph G )
{
  return G->number_of_layers;
}

const char * getGraphName( Graph G )
{
  return G->name;
}

const char * getHowCreated( Graph G )
{
  return G->how_created;
}

/**
 * Adds a node to a layer.
 */
static void addNodeToLayer( Graph G, Node node, Layer layer_ptr )
{
  unsigned number_of_nodes = layer_ptr->number_of_nodes;
  unsigned current_size = layer_ptr->node_array_size;
  if( number_of_nodes >= current_size )
    // double the size of the array if needed
    {
      layer_ptr->node_array_size *= 2;
      layer_ptr->node_array
        = (Node *)
        realloc( layer_ptr->node_array, 
                 layer_ptr->node_array_size * sizeof(Node) );
      assert( layer_ptr->node_array );
  /* This did not compile:
      DOUBLE_ARRAY( Node,
                    layer_ptr->node_array_size,
                    layer_ptr->node_array )
  */
    }
  layer_ptr->node_array[ number_of_nodes ] = node;
  layer_ptr->number_of_nodes++;
}

Node addNode( Graph G, const char * name, int layer )
{
  Node new_node = (Node) calloc( 1, sizeof(struct node) );
  CLONE_STRING( new_node->name, name );
  new_node->layer = layer;
  addNodeToLayer( G, new_node, G->layer_array[layer] );
  return new_node;
}

const char * getName( Graph G, Node node )
{
  return node->name;
}

int getLayer( Graph G, Node node )
{
  return node->layer;
}

int getNumberOfNodesOnLayer( Graph G, int layer )
{
  return G->layer_array[ layer ]->number_of_nodes;
}

const Node * getNodesOnLayer( Graph G, int layer )
{
  return G->layer_array[ layer ]->node_array;
}

void addEdge( Graph G, Node v, Node w )
{
  if( G->number_of_edges >= G->edge_array_size )
    // double the size of the array if needed
    {
      G->edge_array_size *= 2;
      G->edge_array
        = (Edge *)
        realloc( G->edge_array, 
                 G->edge_array_size * sizeof(Edge) );
      assert( G->edge_array );
    }
  G->edge_array[ G->number_of_edges ].from = v;
  G->edge_array[ G->number_of_edges ].to = w;
  G->number_of_edges++;
}

int getNumberOfEdges( Graph G )
{
  return G-> number_of_edges;
}

const Edge * getAllEdges( Graph G )
{
  return G->edge_array;
}

/*  [Last modified: 2008 07 28 at 16:51:14 GMT] */
