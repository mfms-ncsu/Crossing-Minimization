/**
 * @file random_tree.c
 * @brief Module for creating a random tree with a given number of nodes and
 * layers.
 *
 * Paths in the tree move in a single direction until they run out of layers
 * and then continue the process switching directions only when necessary.
 *
 * @author Matt Stallmann
 * @date 2011/05/30
 * $Id: random_tree.c 2 2011-06-07 19:50:41Z mfms $
 */

#include "graph.h"
#include "graph_io.h"
#include "random_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/**
 * New nodes randomly selected to be adjacent to the current node are put at
 * the rear of the master node list; this global variable keeps track of that
 * rear position; this rear position also determines the number of nodes that
 * have been added to the tree, hence the name
 */
static int number_of_tree_nodes = 0;

/**
 * The number of layers that have no nodes on them. Used to ensure that
 * branching does not cause the number of layers to be less than what was
 * requested.
 */
static int layers_remaining;

bool * going_up = NULL;

/**
 * Assigns a direction to each node, initially up
 */
static void init_node_directions( int num_nodes )
{
  going_up = calloc( num_nodes, sizeof( bool ) );
  for ( int i = 0; i < num_nodes; i++ )
    going_up[i] = true;
}

/**
 * initializes the data for a node at a given position in the master node list
 * @param node_number the position of the node
 * @return a pointer to the new node
 */
static Nodeptr create_node( int node_number )
{
  Nodeptr new_node = (Nodeptr) calloc( 1, sizeof(struct node_struct));
  new_node->id = node_number;
  
  // give the node a name based on its position in the master list
  char name_buffer[ MAX_NAME_LENGTH ];
  sprintf( name_buffer, "n_%d", node_number );
  new_node->name = (char *) malloc( strlen(name_buffer) + 1 );
  strcpy( new_node->name, name_buffer );

  new_node->layer = new_node->position = -1; /* to indicate "uninitialized" */
  new_node->up_degree = new_node->down_degree = 0;
  new_node->up_edges = new_node->down_edges = NULL;
  new_node->up_crossings = new_node->down_crossings = 0;
  new_node->marked = new_node->fixed = false;
  new_node->preorder_number = -1;
  return new_node;
}

/**
 * Creates and initializes the master node list
 */
static void create_master_node_list( int num_nodes )
{
  master_node_list = (Nodeptr *) calloc( num_nodes, sizeof( Nodeptr ) );
  for ( int i = 0; i < num_nodes; i++ )
    {
      master_node_list[i] = create_node( i );
    }
}

static void init_layers( int num_layers )
{
  layers = (Layerptr *) calloc( num_layers, sizeof( Layerptr ) );
  for ( int i = 0; i < num_layers; i++ )
    {
      layers[i] = calloc( 1, sizeof( struct layer_struct ) );
      layers[i]->number_of_nodes = 0;
      layers[i]->nodes = NULL;
      layers[i]->fixed = false;
    }
}

static void add_node_to_list( Nodeptr node )
{
  master_node_list[ number_of_tree_nodes++ ] = node;
}

static void add_node_to_layer( Nodeptr node, int layer )
{
#ifdef DEBUG
  printf( "-> add_node_to_layer: node = %s, layer = %d\n", node->name, layer );
#endif
  Layerptr layer_ptr = layers[layer];

  // First node on this layer means fewer layers remain
  if ( layer_ptr->number_of_nodes == 0 )
    layers_remaining--;

  if ( layer_ptr->number_of_nodes % CAPACITY_INCREMENT == 0 )
    {
      layer_ptr->nodes
        = (Nodeptr *) realloc( layer_ptr->nodes,
                               (layer_ptr->number_of_nodes + CAPACITY_INCREMENT) * sizeof(Nodeptr) );
    }
  node->layer = layer;
  node->position = layer_ptr->number_of_nodes;
  layer_ptr->nodes[ layer_ptr->number_of_nodes++ ] = node;
#ifdef DEBUG
  printf( "<- add_node_to_layer: position = %d, number_of_nodes = %d\n",
          node->position, layer_ptr->number_of_nodes );
#endif
}

static void assign_child_layer_and_direction( int parent, int child )
{
  Nodeptr parent_node = master_node_list[ parent ];
  Nodeptr child_node =  master_node_list[ child ];
  int parent_layer = parent_node->layer;
#ifdef DEBUG
  printf( "-> assign_child_layer_and_direction: parent = %d, child = %d\n"
          "    parent_layer = %d, going_up = %d\n",
          parent, child, parent_layer, going_up[parent] );
#endif

  going_up[child] = going_up[parent];
  // handle cases where the path needs to turn around
  if ( going_up[parent] && parent_layer == number_of_layers - 1 )
    going_up[child] = false;
  else if ( ! going_up[parent] && parent_layer == 0 )
    going_up[child] = true;
  if ( going_up[child] )
    child_node->layer = parent_layer + 1;
  else child_node->layer = parent_layer - 1;
  add_node_to_layer( child_node, child_node->layer );

#ifdef DEBUG
  printf( "<- assign_child_layer_and_direction: parent = %d, child = %d\n"
          "    child_layer = %d, child_going_up = %d\n",
          parent, child, child_node->layer, going_up[child] );
#endif
}

/**
 * Adds an edge between two nodes whose layers are determined and adjacent to
 * one another
 */
static void add_edge( Nodeptr upper_node, Nodeptr lower_node )
{
#ifdef DEBUG
  printf( "-> add_edge: upper_node = (%s,%d,%d), lower_node = (%s,%d,%d)\n",
          upper_node->name, upper_node->layer, upper_node->position,
          lower_node->name, lower_node->layer, lower_node->position );
#endif
  assert( upper_node->layer == lower_node->layer + 1 );
  Edgeptr new_edge = (Edgeptr) calloc( 1, sizeof(struct edge_struct) );
  new_edge->up_node = upper_node;
  new_edge->down_node = lower_node;
  new_edge->fixed = false;

  // add new edge to master edge list, making room if necessary
  if ( number_of_edges % CAPACITY_INCREMENT == 0 )
    {
      master_edge_list
        = (Edgeptr *) realloc( master_edge_list,
                               (number_of_edges + CAPACITY_INCREMENT) * sizeof(Edgeptr) );
    }
  master_edge_list[ number_of_edges++ ] = new_edge;

  // add new edge to lower edge list of upper node, making room if necessary
  if ( upper_node->down_degree % CAPACITY_INCREMENT == 0 )
    {
      upper_node->down_edges
        = (Edgeptr *) realloc( upper_node->down_edges,
                               (upper_node->down_degree + CAPACITY_INCREMENT) * sizeof(Edgeptr) );
    }
  upper_node->down_edges[ upper_node->down_degree++ ] = new_edge;
  // add new edge to upper edge list of lower node, making room if necessary
  if ( lower_node->up_degree % CAPACITY_INCREMENT == 0 )
    {
      lower_node->up_edges
        = (Edgeptr *) realloc( lower_node->up_edges,
                               lower_node->up_degree + CAPACITY_INCREMENT * sizeof(Edgeptr) );
    }
  lower_node->up_edges[ lower_node->up_degree++ ] = new_edge;

#ifdef DEBUG
  printf( "<- add_edge: edge = %s -> %s\n",
          new_edge->up_node->name, new_edge->down_node->name );
#endif
}

void create_random_tree( int num_nodes,
                         int num_layers,
                         int branching_factor )
{
  assert( num_nodes > 0
          && num_layers > 1
          && branching_factor > 0 );

  number_of_nodes = num_nodes;
  create_master_node_list( num_nodes );
  number_of_layers = num_layers;
  layers_remaining = num_layers;
  number_of_edges = 0;
  master_edge_list = NULL;

  init_layers( num_layers );
  init_node_directions( num_nodes );

  // create node 0, put it on layer 0, and make it the current node
  int current_node_id = 0;
  Nodeptr current_node = master_node_list[ current_node_id ];
  add_node_to_layer( current_node, 0 );
  add_node_to_list( current_node );

  // create the tree as follows
  // - take the node at the front of the list (current_node_id)
  // - choose a degree d in the range [1 .. branching_factor]
  // - put d new nodes at the rear of the list, assign them to the
  // appropriate layer and add the appropriate edges
  while( number_of_tree_nodes < number_of_nodes )
    {
      // Need to make sure that there are enough nodes left to ensure that
      // every layer has nodes: layers_remaining may be 1 even if so; this
      // ensures that the number of children does not exceed the number of
      // nodes remaining
      int nodes_remaining = number_of_nodes - number_of_tree_nodes;
      int path_length_to_highest_layer = num_layers - 1 - current_node->layer;
      int branch_limit = branching_factor;
      // need to leave enough nodes for next node to have access to the top
      // layer; this may turn out to be <= 0, in which case there should be no
      // branching 
      if ( layers_remaining > 0 &&  
          branch_limit > nodes_remaining - path_length_to_highest_layer )
        branch_limit = nodes_remaining - path_length_to_highest_layer;
      // also, can't have more branches than number of nodes remaining
      if ( branch_limit > nodes_remaining )
        branch_limit = nodes_remaining;
      // finally, smooth out the number of nodes in each layer
      if ( layers[current_node->layer]->number_of_nodes
           > num_nodes / num_layers )
        branch_limit = 1;

      int max_branches
        = branching_factor > branch_limit ? branch_limit : branching_factor;
      int out_degree = 0;
      if ( max_branches > 0 )
        out_degree = random() % max_branches;
      // need to have at least one successor if there are no nodes left
      if ( out_degree == 0
           && current_node_id + 1 >= number_of_tree_nodes )
        out_degree = 1;
#ifdef DEBUG
      printf( 
             " Picking # of children: node = %d, remaining = %d,"
             " path_length = %d, limit = %d"
             " max = %d, degree = %d\n",
             current_node_id,
             nodes_remaining,
             path_length_to_highest_layer,
             branch_limit,
             max_branches,
             out_degree
              );
#endif
      for ( int i = 0; i < out_degree; i++ )
        {
          int child_node_id = number_of_tree_nodes;
          Nodeptr child_node = master_node_list[ child_node_id ];
          add_node_to_list( child_node );

          assign_child_layer_and_direction( current_node_id, child_node_id );
          
          if ( current_node->layer > child_node->layer )
            add_edge( current_node, child_node );
          else
            add_edge( child_node, current_node );
        }

      current_node_id++;
      current_node = master_node_list[ current_node_id ];

#ifdef OMITTED
      printf( " __ iteration %d: master_node_list, number_of_nodes = %d\n", current_node_id, number_of_tree_nodes );
      for ( int i = 0; i < number_of_tree_nodes; i++ )
        printf( " %d(%d,%d)",
                master_node_list[i]->id,
                master_node_list[i]->layer,
                master_node_list[i]->position );
      printf( "\n" );
      printf( " master_edge_list, number_of_edges = %d\n", number_of_edges );
      for ( int i = 0; i < number_of_edges; i++ )
        printf( " %d->%d",
                master_edge_list[i]->up_node->id,
                master_edge_list[i]->down_node->id
                );
      printf( "\n" );
#endif      
    }
}

#ifdef TEST
/**
 * For testing
 */
int main( int argc, char * argv[] )
{
  assert( argc == 4 && "nodes layers braching" );
  int num_nodes = atoi( argv[1] );
  int num_layers = atoi( argv[2] );
  int branching = atoi( argv[3] );

  srandom( 1 );

  create_random_tree( num_nodes, num_layers, branching );

  writeDot( "test.dot",
            "test_graph",
             "",
             master_edge_list,
             number_of_edges
            );

  writeOrd( "test.ord" );
  
  return EXIT_SUCCESS;
}
#endif

/*  [Last modified: 2011 06 03 at 19:01:11 GMT] */
