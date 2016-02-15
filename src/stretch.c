/**
 * @file stretch.c @brief implementation of utilities for computing and
 * updating the "stretch" of edges, where stretch is of edge vw is defined to
 * be
 *      abs( p(v)/(|L(v)|-1) - p(w)/(|L(w)|-1) )
 * Here p(x), L(x) are the position and layer of x, respectively; if there is
 * only one node on a layer, the denominator is replaced by 2.
 */

#include <stdio.h>
#include <math.h>
#include "graph.h"
#include "stretch.h"

double stretch(Edgeptr e) {
  Nodeptr v = e->down_node;
  Nodeptr w = e->up_node;
#ifdef DEBUG
  printf("-> stretch, %s, %s\n", v->name, w->name);
#endif
  int v_layer = v->layer; 
  int w_layer = w->layer;
  int v_layer_size = layers[v_layer]->number_of_nodes;
  int w_layer_size = layers[w_layer]->number_of_nodes;
  double v_scale = v_layer_size > 1 ? v_layer_size - 1.0 : 2.0;
  double w_scale = w_layer_size > 1 ? w_layer_size - 1.0 : 2.0;
  double stretch = fabs( v->position / v_scale - w->position / w_scale );
#ifdef DEBUG
  printf("<- stretch, v: scale, position = %f, %d; w: scale, position = %f, %d;"
         " stretch = %f\n",
         v_scale, v->position, w_scale, w->position, stretch);
#endif
  return stretch;
}

/*  [Last modified: 2016 02 15 at 20:19:36 GMT] */
