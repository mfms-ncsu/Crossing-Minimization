/**
 * @file stretch.c @brief implementation of utilities for computing and
 * updating the "stretch" of edges, where stretch is of edge vw is defined to
 * be
 *      abs( p(v)/(|L(v)|-1) - p(w)/(|L(w)|-1) )
 * Here p(x), L(x) are the position and layer of x, respectively; if there is
 * only one node on a layer, the denominator is replaced by 2.
 */

#include <math.h>
#include "graph.h"
#include "stretch.h"

double stretch(Edgeptr e) {
  Nodeptr v = e->down_node;
  Nodeptr w = e->up_node;
  int v_layer = v->layer; 
  int w_layer = w->layer;
  int v_layer_size = layers[v_layer]->number_of_nodes;
  int w_layer_size = layers[w_layer]->number_of_nodes;
  double v_scale = v_layer_size > 1 ? 1.0 / (v_layer_size - 1) : 2.0;
  double w_scale = w_layer_size > 1 ? 1.0 / (w_layer_size - 1) : 2.0;
  return fabs( v->position / vscale - w->position / w_scale );
}

/*  [Last modified: 2016 02 15 at 17:10:11 GMT] */
