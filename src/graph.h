/**
 * @file graph.h
 * @brief Definition of data structures and access functions for a layered
 * graph.
 * @author Matt Stallmann, based on Saurabh Gupta's crossing heuristics
 * implementation.
 * @date 2008/12/19
 * $Id: graph.h 90 2014-08-13 20:31:25Z mfms $
 *
 * Global data
 *  - number_of_layers
 *  - layers: an array of pointers to layer_struct's
 *  - graph_name: used for output
 * Layers are referred to by number except when internal info is needed.<br>
 * Nodes are referred to by pointers to node_struct's and all information
 * about a node (including layer and position) is stored in the struct.
 *
 * To traverse all the nodes of a graph, do the following:
 *
     for( int layer = 0; layer < number_of_layers; layer++ )
       {
         for( int position = 0; position < layers[ layer ]->number_of_nodes; position++ )
           {
             Nodeptr node = layers[ layer ]->nodes[ position ];
             // do something with the node
           }
       }
 *
 * To traverse all the edges of the graph (as down edges of layers 1 through
 * number_of_layers - 1), do -- this is O(m):

  for( int layer = 1; layer < number_of_layers; layer++ )
    {
      for(
          int node_position = 0;
          node_position < layers[ layer ]->number_of_nodes;
          node_position++ )
        {
          Nodeptr node = layers[ layer ]->nodes[ node_position ];
          for( int edge_position = 0; edge_position < node->down_degree; edge_position++ )
            {
              Edgeptr edge = node->down_edges[ edge_position ];
              // do something with the edge
            }
        }
    }

 */

#include<stdbool.h>

#ifndef GRAPH_H
#define GRAPH_H

#include"defs.h"

typedef struct node_struct * Nodeptr;
typedef struct edge_struct * Edgeptr;
typedef struct layer_struct * Layerptr;

struct node_struct
{
  char * name;
  int id;                       /* unique identifier */
  int layer;
  /**
   * position of the node within its layer; this is essential for correct
   * computation of crossings; it is automatically updated by the update
   * functions for crossings in the crossings module and should be updated
   * locally by any heuristic that relies on dynamic information about
   * crossings.
   */
  int position;
  int up_degree;
  int down_degree;

  Edgeptr * up_edges;
  Edgeptr * down_edges;

  // for heuristics based on sorting (in most cases this will be an int, but
  // barycenter involves fractions
  double weight;

  // Added on 09-11-08 for max. crossings node heuristic
  bool fixed;
  int up_crossings;
  int down_crossings;
  
  // for DFS
  bool marked;
  int preorder_number;
};

#define DEGREE( node ) ( node->up_degree + node->down_degree )
#define CROSSINGS( node ) ( node->up_crossings + node->down_crossings )

struct edge_struct {
  Nodeptr up_node;
  Nodeptr down_node;
  int crossings;

  // for heuristics
  /**
   * true if edge has been processed in current iteration
   */
  bool fixed;
  /**
   * true if minimizing crossings for this edge should be given priority (not
   * used - instead, a list of priority edges is maintained)
   */
  //  bool prioritize;
};

struct layer_struct {
  int number_of_nodes;
  Nodeptr * nodes;

  // for algorithms that fix layers during an iteration
  bool fixed;
};

// The following are defined in graph_io.c

/**
 * Allows nodes to be accessed randomly by their unique id #'s; not used for
 * anything at this point.
 *
 * @todo this and master_edge_list could really be useful for heuristics such
 * as mcn and mce.
 */
extern Nodeptr * master_node_list;

/**
 * @see master_node_list
 */
extern Edgeptr * master_edge_list;

extern int number_of_layers;
extern int number_of_nodes;
extern int number_of_edges;
extern int number_of_isolated_nodes;
extern Layerptr * layers;
extern char graph_name[MAX_NAME_LENGTH];

#endif

/*  [Last modified: 2014 08 13 at 19:45:17 GMT] */
