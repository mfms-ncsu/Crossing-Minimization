/**
 * @file graph_io.c
 * @brief Implementation of functions that create graph structures from input
 * .dot and .ord files. 
 * @author Matt Stallmann, based on Saurabh Gupta's crossing heuristics
 * implementation.
 * @date 2008/12/19
 * $Id: graph_io.c 97 2014-09-10 17:05:19Z mfms $
 */

#include"graph.h"
#include"hash.h"
#include"defs.h"
#include"dot.h"
#include"ord.h"
#include"min_crossings.h"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#define MIN_LAYER_CAPACITY 1 

Nodeptr * master_node_list;
Edgeptr * master_edge_list;
int number_of_nodes = 0;
int number_of_layers = 0;
int number_of_edges = 0;
int number_of_isolated_nodes = 0;
Layerptr * layers = NULL;
char graph_name[MAX_NAME_LENGTH];

// initial allocated size of layer array (will double as needed)
static int layer_capacity = MIN_LAYER_CAPACITY;

// The input algorithm is as follows:
//   1. Read the ord file (first pass) and
//       (a) create each layer and expand the 'layers' array as needed
//       (b) count the number of nodes on each layer and store in
//           'number_of_nodes'; also count the global number of nodes
//       (c) allocate the 'nodes' array for each layer
//   2. Read the ord file again and 
//       (a) create each node
//       (b) add each node to the appropriate layer
//   3. Read the dot file (first pass) and
//       (a) count the 'up_degree' and 'down_degree' of each node
//       (b) go through all the nodes and allocate the 'up_edges' and
//           the 'down_edges'
//       (c) reset 'up_degree' and 'down_degree' to 0 so that edges can
//           be put in the right positions on the second pass 
//   4. Read the dot file again and put the nodes into the adjacency lists
//      based on the edges
//
// Note: The last phase ignores directions of the edges in the dot file and
// only looks at layer information to determine 'up' and 'down' edges for
// each node. For example, if a->b in the dot file and a is on layer 1 while
// b is on layer 0, then the edge is an up-edge for b and a down-edge for a.

/**
 * Creates a new node and maps its name to (a pointer to) its record
 * @param name the name of the node
 * @return (a pointer to) the newly created node
 */
Nodeptr makeNode( const char * name );

/**
 * Put a node in the next available position on a given layer
 */
void addNodeToLayer( Nodeptr node, int layer );

/**
 * Creates a new layer; the number is assumed to be the next available layer
 * number, i.e., layers are always created in ascending numerical order
 */
void makeLayer();

/**
 * Adds an edge to the graph. The edge comes directly from the dot
 * file. Although instances usually direct edges from lower to higher layers,
 * no such assumption is made here.  A fatal error occurs if the nodes are
 * not on adjacent layers.
 */
void addEdge( const char * name1, const char * name2 );

Nodeptr makeNode( const char * name )
{
  static int current_id = 0;

  Nodeptr new_node = (Nodeptr) malloc( sizeof(struct node_struct));
  new_node->name = (char *) malloc( strlen(name) + 1 );
  strcpy( new_node->name, name );
  // delay assignment of id's until edges are added so that the numbering
  // depends on .dot file only (easier to standardize)
  new_node->id = current_id++;
  new_node->layer = new_node->position = -1; /* to indicate "uninitialized" */
  new_node->up_degree = new_node->down_degree = 0;
  new_node->up_edges = new_node->down_edges = NULL;
  new_node->up_crossings = new_node->down_crossings = 0;
  new_node->marked = new_node->fixed = false;
  new_node->preorder_number = -1;
  insertInHashTable( name, new_node );
  master_node_list[ new_node->id ] = new_node;
  return new_node;
}

void addNodeToLayer( Nodeptr node, int layer )
{
  static int current_layer = 0;
  static int current_position = 0;
  if( layer != current_layer )
    {
      current_layer = layer;
      current_position = 0;
    }
  node->layer = current_layer;
  node->position = current_position;
  layers[ layer ]->nodes[ current_position++ ] = node;
}

void makeLayer()
{
  Layerptr new_layer = (Layerptr) malloc( sizeof(struct layer_struct) );
  new_layer->number_of_nodes = 0;
  new_layer->nodes = NULL;
  if( number_of_layers >= layer_capacity )
    {
      layer_capacity *= 2;
      layers
        = (Layerptr *) realloc( layers, layer_capacity * sizeof(Layerptr) );
    }
  layers[ number_of_layers++ ] = new_layer;
}

void addEdge( const char * name1, const char * name2 )
{
  static int num_edges_so_far = 0;
  Nodeptr node1 = getFromHashTable( name1 );
  Nodeptr node2 = getFromHashTable( name2 );
  if ( node1->layer == node2->layer ) {
    fprintf( stderr, "FATAL: addEdge, nodes on same layer.\n" );
    fprintf( stderr, " Nodes %s and %s are on layer %d.\n",
             node1->name, node2->name, node1->layer);
    abort();
  }

  // a warning about missing nodes was already issued in the first pass
  if ( node1 == NULL || node2 == NULL ) return;

  Nodeptr upper_node
    = ( node1->layer > node2->layer ) ? node1 : node2;
  Nodeptr lower_node
    = ( node1->layer < node2->layer ) ? node1 : node2;
  if ( upper_node->layer - lower_node->layer != 1 ) {
      fprintf( stderr, "FATAL: addEdge, nodes not on adjacent layers.\n" );
      fprintf( stderr, " Nodes %s is on layer %d and %s is on layer %d.\n",
               upper_node->name, upper_node->layer,
               lower_node->name, lower_node->layer);
      abort();
  }
  Edgeptr new_edge = malloc( sizeof(struct edge_struct) );
  new_edge->up_node = upper_node;
  new_edge->down_node = lower_node;
  new_edge->fixed = false;
  upper_node->down_edges[ upper_node->down_degree++ ] = new_edge;
  lower_node->up_edges[ lower_node->up_degree++ ] = new_edge;
  master_edge_list[ num_edges_so_far++ ] = new_edge;
}

/**
 * Sets number of nodes for the layer and allocates space for them. Note: the
 * global number of nodes is updated in allocateLayers().
 */
static void setNumberOfNodes( int layer, int number )
{
  layers[ layer ]->number_of_nodes = number;
  layers[ layer ]->nodes = (Nodeptr *) calloc( number, sizeof(Nodeptr) );
}

/**
 * Implements the first pass of reading the ord file: allocates a record for
 * each layer and space for the nodes on each layer. Creates the nodes
 * and maps their names to (pointers to) their records. Counts the total
 * number of nodes.
 */
static void allocateLayers( const char * ord_file )
{
  FILE * in = fopen( ord_file, "r" );
  if( in == NULL )
    {
      fprintf( stderr, "Unable to open file %s for input\n", ord_file );
      exit( EXIT_FAILURE );
    }
  layer_capacity = MIN_LAYER_CAPACITY;
  layers = (Layerptr *) calloc( layer_capacity, sizeof(Layerptr) );

  int layer;
  int expected_layer = 0;
  char name_buf[MAX_NAME_LENGTH];
  while ( nextLayer( in, & layer ) )
    {
      if( layer != expected_layer )
        {
          fprintf( stderr, "Fatal error: Expected layer %d, found layer %d\n",
                   expected_layer, layer );
          abort();
        }
      expected_layer++;
      makeLayer();
      int node_count = 0;
      while ( nextNode( in, name_buf ) )
        {
          node_count++;
          number_of_nodes++;    /* global node count */
        }
      setNumberOfNodes( layer, node_count );
  }
  fclose( in );
}

/**
 * Reads the ord file and put the nodes on their appropriate layers
 */
static void assignNodesToLayers( const char * ord_file )
{
  FILE * in = fopen( ord_file, "r" );
  int layer;
  char name_buf[MAX_NAME_LENGTH];
  while( nextLayer( in, & layer ) )
    {
      while( nextNode( in, name_buf ) )
        {
          Nodeptr node = makeNode( name_buf );
          addNodeToLayer( node, layer );
        }
    }
  fclose( in );
}

/**
 * Increments the degrees of the endpoints of an edge between name1 and name2
 */
void incrementDegrees( const char * name1, const char * name2 )
{
  Nodeptr node1 = getFromHashTable( name1 );
  if( node1 == NULL )
    {
      fprintf( stderr, "Fatal error: Node '%s' does not exist in .ord file\n"
               " edge is %s->%s\n", name1, name1, name2);
      abort();
    }
  Nodeptr node2 = getFromHashTable( name2 );
  if( node2 == NULL )
    {
      fprintf( stderr, "Fatal error: Node '%s' does not exist in .ord file\n"
               " edge is %s->%s\n", name2, name1, name2);
      abort();
    }
  Nodeptr upper_node
    = ( node1->layer > node2->layer ) ? node1 : node2;
  Nodeptr lower_node
    = ( node1->layer < node2->layer ) ? node1 : node2;
  upper_node->down_degree++;
  lower_node->up_degree++;
}

/**
 * Reads the dot file and makes room for nodes on all the adjacency lists;
 * resets up and down node degrees. This is the first pass of reading the dot
 * file.  Also saves the name of the graph.
 */
void allocateAdjacencyLists( const char * dot_file )
{
  FILE * in = fopen( dot_file, "r" );
  if( in == NULL )
    {
      fprintf( stderr, "Unable to open file %s for input\n", dot_file );
      exit( EXIT_FAILURE );
    }
  initDot( in );
  getNameFromDotFile( graph_name );
  // read the edges and use each edge to update the appropriate degree for
  // each endpoint
  char src_buf[MAX_NAME_LENGTH];
  char dst_buf[MAX_NAME_LENGTH];
  while ( nextEdge( in, src_buf, dst_buf ) )
    {
#ifdef DEBUG
      printf( " new edge: %s -> %s\n", src_buf, dst_buf );
#endif
      number_of_edges++;
      incrementDegrees( src_buf, dst_buf );
    }
  // allocate adjacency lists for all nodes based on the appropriate degrees
  int layer = 0;
  for( ; layer < number_of_layers; layer++ )
    {
      int position = 0;
      for( ; position < layers[ layer ]->number_of_nodes; position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ position ];
          node->up_edges
            = (Edgeptr *) calloc( node->up_degree, sizeof(Nodeptr) );
          node->down_edges
            = (Edgeptr *) calloc( node->down_degree, sizeof(Nodeptr) );
          node->up_degree = 0;
          node->down_degree = 0;
        }
    }
  fclose( in );
}

/**
 * Reads the dot file and adds all the edges. This is the second pass.
 */
void createEdges( const char * dot_file )
{
  FILE * in = fopen( dot_file, "r" );
  initDot( in );
  char src_buf[MAX_NAME_LENGTH];
  char dst_buf[MAX_NAME_LENGTH];
  while ( nextEdge( in, src_buf, dst_buf ) )
    {
      addEdge( src_buf, dst_buf );
    }
  fclose( in );
}

/**
 * @return number of nodes whose up_degree and
 * down_degree are both zero
 */
static int countIsolatedNodes()
{
  /**
   * @todo
   * Should eliminate the isolated nodes altogether; on each layer -
   * <pre>
   *  deallocate isolated nodes and mark positions with NULL
   *  position = real_position = 0;
   *  for position < number_of_nodes, position++
   *     while real_position < number_of_nodes and node[position] == NULL
   *        real_position++
   *     if node[position] == NULL && real_position < number_of_nodes
   *        node[position] = node[real_position++]
   * </pre>
   */
  int isolated_nodes = 0;
  int layer = 0;
  for( ; layer < number_of_layers; layer++ )
    {
      int position = 0;
      for( ; position < layers[ layer ]->number_of_nodes; position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ position ];
          if( node->up_degree + node->down_degree == 0 )
            isolated_nodes++;
        }
    }
  return isolated_nodes;
}

void readGraph( const char * dot_file, const char * ord_file )
{
  number_of_nodes = 0;
  number_of_edges = 0;
  number_of_layers = 0;
  allocateLayers( ord_file );
  master_node_list = (Nodeptr *) calloc( number_of_nodes, sizeof(Nodeptr) );
  initHashTable( number_of_nodes );
  assignNodesToLayers( ord_file );
#ifdef DEBUG
  printf( "Master node list after reading ord file:\n" );
  for ( int i = 0; i < number_of_nodes; i++ ) {
    printf( "%s, layer = %d, position = %d\n", master_node_list[i]->name,
            master_node_list[i]->layer, master_node_list[i]->position );
  }
#endif
  allocateAdjacencyLists( dot_file );
  // at this point the number of edges is known
  master_edge_list = (Edgeptr *) calloc( number_of_edges, sizeof(Edgeptr) );
  createEdges( dot_file );
  number_of_isolated_nodes = countIsolatedNodes();
  removeHashTable();
}

// --------------- Output to dot and ord files

static void writeNodes( FILE * out, Layerptr layerptr )
{
  int i = 0;
  for( ; i < layerptr->number_of_nodes; i++ )
    {
      outputNode( out, layerptr->nodes[i]->name );
    }
}

/**
 * @todo ord file should have information about heuristic that created it, etc.,
 * embedded in it. Use of the file/graph name can be problematic if we want to
 * check what happens when one heuristic follows another for a whole class
 * using a script. There needs to be some way to differentiate among these
 * files.
 */
void writeOrd( const char * ord_file )
{
  FILE * out = fopen( ord_file, "w" );
  if( out == NULL )
    {
      fprintf( stderr, "Unable to open file %s for output\n", ord_file );
      exit( EXIT_FAILURE );
    }
  ordPreamble( out, graph_name, "" );
  int layer = 0;
  for( ; layer < number_of_layers; layer++ )
    {
      beginLayer( out, layer, "heuristic-based" );
      writeNodes( out, layers[ layer ] );
      endLayer( out );
    }
  fclose( out );
}

void writeDot( const char * dot_file_name,
               const char * graph_name,
               const char * header_information,
               Edgeptr * edge_list,
               int edge_list_length
               )
{
  FILE * out = fopen( dot_file_name, "w" );
  if( out == NULL )
    {
      fprintf( stderr, "Unable to open file %s for output\n", dot_file_name );
      exit( EXIT_FAILURE );
    }
  dotPreamble( out, graph_name, header_information );
  for ( int i = 0; i < edge_list_length; i++ )
    {
      Edgeptr current = edge_list[i];
      Nodeptr up_node = current->up_node;
      Nodeptr down_node = current->down_node;
      outputEdge( out, up_node->name, down_node->name );
    }
  endDot( out );
  fclose( out );
}

// --------------- Debugging output --------------

static void printNode( Nodeptr node )
{
  printf("    [%3d ] %s layer=%d position=%d up=%d down=%d up_x=%d down_x=%d\n",
         node->id, node->name, node->layer, node->position,
         node->up_degree, node->down_degree,
         node->up_crossings, node->down_crossings );
  printf("      ^^^^up");
  int i = 0;
  for( ; i < node->up_degree; i++ )
    {
      Edgeptr edge = node->up_edges[i];
      printf(" %s", edge->up_node->name );
    }
  printf("\n");
  printf("      __down");
  i = 0;
  for( ; i < node->down_degree; i++ )
    {
      Edgeptr edge = node->down_edges[i];
      printf(" %s", edge->down_node->name );
    }
  printf("\n");
}

static void printLayer( int layer )
{
  printf("  --- layer %d nodes=%d fixed=%d\n",
         layer, layers[layer]->number_of_nodes, layers[layer]->fixed );
  int node = 0;
  for( ; node < layers[layer]->number_of_nodes; node++ )
    {
      printNode( layers[layer]->nodes[node] );
    }
}

void printGraph()
{
  printf("+++ begin-graph %s nodes=%d, layers=%d\n",
         graph_name, number_of_nodes, number_of_layers);
  int layer = 0;
  for( ; layer < number_of_layers; layer++ )
    {
      printLayer( layer );
    }
  printf("=== end-graph\n");
}

#ifdef TEST

int main( int argc, char * argv[] )
{
  readGraph( argv[1], argv[2] );
  printGraph();
  fprintf( stderr, "Average number of probes = %5.2f\n",
           getAverageNumberOfProbes() );
  return 0;
}

#endif

/*  [Last modified: 2014 09 10 at 14:34:28 GMT] */
