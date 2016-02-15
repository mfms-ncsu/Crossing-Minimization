/**
 * @file order.h
 * @brief data structure and function headers for saving/restoring order
 * information.
 * @author Matt Stallmann
 * @date 2009/07/27
 * $Id: order.h 2 2011-06-07 19:50:41Z mfms $
 */

#ifndef ORDER_H
#define ORDER_H

#include"graph.h"

/**
 * Keeps track of order information for each layer; used to save the state
 * for orderings that give minimum crossings or edge crossings.
 */
typedef struct order_struct {
  int num_layers;
  int * num_nodes_on_layer;
  Nodeptr * * node_ptr_on_layer;
} * Orderptr;

/**
 * Allocates information inside the ord_info based on the current graph and
 * saves current (initial) layer ordering
 */
void init_order( Orderptr ord_info );

/**
 * Deallocates information.
 * ASSUMES: init_order( ord_info ) was previously called
 */
void cleanup_order( Orderptr ord_info );

/**
 * Copies information from the current graph configuration into the
 * order_struct referenced by ord_info.
 */
void save_order( Orderptr ord_info );

/**
 * Reaaranges layers of the graph to match the information stored in the
 * order_struct referenced by ord_info.
 */
void restore_order( Orderptr ord_info );

#endif

/*  [Last modified: 2011 05 23 at 16:11:40 GMT] */
