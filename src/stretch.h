/**
 * @file stretch.h @brief header for utilities for computing and updating the
 * "stretch" of edges, where stretch is of edge vw is defined to be
 *      abs( p(v)/(|L(v)|-1) - p(w)/(|L(w)|-1) )
 * Here p(x), L(x) are the position and layer of x, respectively; if there is
 * only one node on a layer, the denominator is replaced by 2.
 */

/**
 * @return the stretch of edge e
 */
double stretch(Edgeptr e);

/*  [Last modified: 2016 02 15 at 17:00:24 GMT] */
