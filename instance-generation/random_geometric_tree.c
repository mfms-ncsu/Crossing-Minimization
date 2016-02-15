/**
 * @file random_geometric_tree.c
 * @brief Program to create a random tree with a given number of layers.
 *
 * The tree is created as a minimum spanning tree on random points in a unit
 * rectangle with given aspect ratio using the infinity norm.  Paths in the
 * tree move in a single direction until they run out of layers and then
 * continue the process switching directions only when necessary.
 *
 * 2009/07/29 - can also use completely random distances (is aspect ratio is
 * set to 0)
 *
 * @author Matt Stallmann
 * @date 2008/07/21
 * $Id: random_geometric_tree.c 19 2011-06-23 01:46:27Z mfms $
 */

#include "LayeredGraph.h"
#include "randomNumbers.h"
#include "IO.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>              /* DBL_MAX */
#include <math.h>               /* fabs() */
#include <string.h>
#include <assert.h>

#define PROGRAM "random_geometric_tree"

#define NAME_FORMAT "n_%d"
#define NODE_NAME_LENGTH 6
#define CREATION_STRING_LENGTH 511
#define DOT_EXTENSION ".dot"
#define ORD_EXTENSION ".ord"

static char name_buffer[ NODE_NAME_LENGTH + 1 ];
/** Used to store information about how the graph was created, i.e., the
    arguments that were used to run this program */
static char creation_string[ CREATION_STRING_LENGTH + 1 ];

/**
 * aspect ratio of the rectangle of points on which MST is to be computed, or
 * 0 if distances are to be random; this is global because it's used by the
 * distance function.
 */
static int aspect_ratio = 0;

static void printUsage( const char * progname )
{
  printf( "Usage: %s basename number_of_nodes number_of_layers"
          " aspect_ratio seed\n",
          progname );
  printf( " basename is name before"
          " the .dot and .ord in the output file names\n" );
  printf( " number_of_nodes, number_of_layers are obvious\n");
  printf( " aspect_ratio is that of the rectangle in which random"
          " points are placed (= 0 for random distances)\n" );
  printf( " -- points represent nodes, edges are based on an MST\n");
  printf( "    using distance between points as a weight\n");
  printf( " seed is three 16-bit integers, separated by commas\n");
  printf( " prints a new seed to stdout at the end\n" );
}

/* ======== Begin MST ADT ========= */

/**
 * Used to keep track of data about a node during the minimum spanning tree
 * algorithm.
 */
typedef struct mst_node
{
  /** true if the node is in the tree already */
  bool in_tree;
  /** current closest tree node to this node */
  int parent;
  /** distance to that closest node */
  double distance_to_parent;
  /** direction of the edge from this node to its parent; only valid when
      this node becomes part of the tree */
  bool going_up;
  /** x-coordinate of node in the plane */
  double x;
  /** y-coordinate of node in the plane */
  double y;
  /** The internal represntation of the node in the tree being contructed */
  Node node;
} * MstNode;

static MstNode * createMstNodeArray( int number_of_nodes )
{
  MstNode * new_array = (MstNode *) calloc( number_of_nodes, sizeof(MstNode) );
  for( int i = 0; i < number_of_nodes; i++ )
    {
      new_array[ i ] = (MstNode) calloc( 1, sizeof(struct mst_node) );
    }
  return new_array;
}

static void destroyMstNodeArray( MstNode * array, int number_of_nodes )
{
  for( int i = 0; i < number_of_nodes; i++ )
    {
      free( array[ i ] );
    }
  free( array );
}

/**
 * Computes distance between two nodes.
 * WARNING: If aspect ratio is 0, this should not be called twice for the
 * same pair of nodes as it will give different results.
 */
static double distance( MstNode node_one, MstNode node_two )
{
  if ( aspect_ratio > 0 )
    {
      double xdiff = fabs( node_one->x - node_two->x );
      double ydiff = fabs( node_one->y - node_two->y );
      return ( (xdiff > ydiff) ? xdiff : ydiff );
    }
  else return RN_real( 0, 1 );
}

/* ============= End MST ADT =============== */

/* ============= Begin utilities================= */

/**
 * converts a string of three numbers separtated by commas to an array of
 * three unsigned short integers
 * @param seed_string the input string
 * @param seed_array the output array (should have room for 3 entries)
 */
static void parseSeed( const char * seed_string, unsigned short * seed_array )
{
  unsigned long temp_seed[ 3 ];
  int number_read = sscanf( seed_string, "%lu,%lu,%lu", 
                            temp_seed, temp_seed + 1, temp_seed + 2 );
  if( number_read != 3 ) {
    fprintf( stderr, "Improper format for seed: %s\n", seed_string );
    fprintf( stderr, "Should be seed1,seed2,seed3\n" );
    exit(1);
  }
  for( int i = 0; i < 3; ++i ) {
    if( temp_seed[ i ] > USHRT_MAX ) {
      fprintf( stderr, "Seed %lu is too large (limit = %u)\n",
               temp_seed[ i ], USHRT_MAX );
      exit(1);
    }
    seed_array[ i ] = (unsigned short) temp_seed[ i ];
  }
}

/**
 * Opens the file with the given name for output; reports the error and exits
 * if unable to do so.
 * @return the stream associated with the file
 */
FILE * openOutput( const char * name )
{
  FILE * retval = fopen( name, "w" );
  if( retval == NULL )
    {
      printf("Unable to open file %s for writing.\n", name);
      exit(1);
    }
  return retval;
}

/* ============= End utilities================= */

int main( int argc, char * argv[] )
{
  if( argc != 6 )
    {
      printUsage( argv[0] );
      return EXIT_FAILURE;
    }

  const char * basename = argv[1];
  int number_of_nodes = atoi( argv[2] );
  int number_of_layers = atoi( argv[3] );
  aspect_ratio = atof( argv[4] );
  const char * seed_string = argv[5];

  assert( number_of_nodes > 0
          && number_of_layers > 0
          && aspect_ratio >= 0 );

  unsigned short seed[ 3 ];
  parseSeed( seed_string, seed );
  RN_setSeed( seed );

  char * dot_file_name = (char *) malloc( strlen(basename)
                                          + strlen(DOT_EXTENSION) + 1);
  strcpy( dot_file_name, basename );
  strcat( dot_file_name, DOT_EXTENSION );
  FILE * dot_file_stream = openOutput( dot_file_name );
  free( dot_file_name );

  char * ord_file_name = (char *) malloc( strlen(basename)
                                          + strlen(ORD_EXTENSION) + 1);
  strcpy( ord_file_name, basename );
  strcat( ord_file_name, ORD_EXTENSION );
  FILE * ord_file_stream = openOutput( ord_file_name );
  free( ord_file_name );

  /* select a random position in a rectangle of dimensions A x 1 for each
     node, where A is the aspect ratio */
  MstNode * mst_nodes = createMstNodeArray( number_of_nodes );
  if( aspect_ratio > 0 )
    {
      for( int i = 0; i < number_of_nodes; i++ )
        {
          mst_nodes[ i ]->x = RN_real( 0, aspect_ratio );
          mst_nodes[ i ]->y = RN_real( 0, 1 );
        }
    }

  sprintf( creation_string, "%s %s %s %s %s %s",
           PROGRAM, argv[1], argv[2], argv[3], argv[4], argv[5] );
  Graph G = createGraph( number_of_layers, basename, creation_string );

  /* compute a minimum spanning tree based on the infinity norm: 
   * distance((x1,y1), (x2,y2)) = max(|x1 - x2|, |y1 - y2|);
   */
  for( int i = 0; i < number_of_nodes; i++ )
    {
      mst_nodes[ i ]->in_tree = false;
      mst_nodes[ i ]->parent = -1;
      mst_nodes[ i ]->distance_to_parent = DBL_MAX;
      mst_nodes[ i ]->going_up = true;
      mst_nodes[ i ]->node = NULL;
    }

  mst_nodes[ 0 ]->in_tree = true;
  sprintf( name_buffer, NAME_FORMAT, 0 );
  mst_nodes[ 0 ]->node = addNode( G, name_buffer, 0 );
  mst_nodes[ 0 ]->in_tree = true;
  mst_nodes[ 0 ]->going_up = true;
  int current_node_index = 0;
  int number_of_tree_nodes = 1;

  // Prim's algorithm, but on the complete graph.  The easiest implementation
  // starts with a full priority queue (array) and all distances at DBL_MAX.
  while( number_of_tree_nodes < number_of_nodes )
    {
      /* update distances to tree given that current_node is in the tree */
      for( int i = 0; i < number_of_nodes; i++ )
        {
          double new_distance
            = distance( mst_nodes[ i ], mst_nodes[ current_node_index ] );
          if( ! mst_nodes[ i ]->in_tree
              && ( new_distance < mst_nodes[ i ]->distance_to_parent ) )
            {
              mst_nodes[ i ]->parent = current_node_index;
              mst_nodes[ i ]->distance_to_parent
                = new_distance;
            }
        }

      /* find the non-tree node closest to the tree */
      current_node_index = -1;
      double current_distance = DBL_MAX;
      for( int i = 0; i < number_of_nodes; i++ )
        {
          if( ! mst_nodes[i]->in_tree
              && mst_nodes[ i ]->distance_to_parent < current_distance )
            {
              current_distance = mst_nodes[ i ]->distance_to_parent;
              current_node_index = i;
            }
        }
      /* create the current node in the layered graph, add it to the tree, and
         add the edge between it and its parent to the tree. */
      MstNode current = mst_nodes[ current_node_index ];
      MstNode parent = mst_nodes[ current->parent ];
      /* nodes are named in order of appearance; the layer of the current
         node needs to be adjacent to that of its parent and move in the same
         direction unless this puts it into a nonexistent layer */
      int parent_layer = getLayer( G, parent->node );
      current->going_up = parent->going_up;
      if( current->going_up && parent_layer == number_of_layers - 1 )
        current->going_up = false;
      else if( ! current->going_up && parent_layer == 0 )
        current->going_up = true;
      int current_layer
        = current->going_up ? (parent_layer + 1) : (parent_layer - 1); 
      sprintf( name_buffer, NAME_FORMAT, number_of_tree_nodes );
      current->node = addNode( G, name_buffer, current_layer );
      current->in_tree = true;
      if( current_layer > parent_layer )
        addEdge( G, parent->node, current->node );
      else
        addEdge( G, current->node, parent->node );
      number_of_tree_nodes++;
    }

  writeDot( dot_file_stream, G );
  writeOrd( ord_file_stream, G );

  destroyGraph( G );
  destroyMstNodeArray( mst_nodes, number_of_nodes );

  // output the new seed
  const unsigned short * new_seed = RN_getSeed();
  printf( "%hu,%hu,%hu\n", new_seed[0], new_seed[1], new_seed[2] );

  return EXIT_SUCCESS;
}

/*  [Last modified: 2011 06 22 at 19:57:30 GMT] */
