#! /usr/bin/env python

"""
createRandomLayeredGraph.py - offers a mechanism for producing layered
 graphs with different variances with respect to number of nodes per layer
 and degrees of nodes. Output is in sgf format, which can be converted to
 dot or graphml, as desired. Graphs have high probability of being connected
 (and interesting) because minimum degree of a node is 2.
"""

import sys
import os
import random

DEBUG = False

def usage( prog_name ):
    print "Usage:", prog_name, "nodes edges layers width_var degree_var seed"
    print "  where nodes, edges, layers = number of nodes, edges, layers, respectively"
    print "        width_var determines the variance in the number of nodes per layer"
    print "           0 means all layers have the same size"
    print "           1 means size can vary from 1 to 2 * average"
    print "        degree_var determines the extent to which node degree might vary"
    print "           0 means degree of nodes will be balanced as much as possible"
    print "           1 means all nodes have equal chance of adding an edge"
    print "           > 1 means there will be a bias toward adding edges to higher degree nodes"
    print "        width_var and degree_var are floating point numbers"
    print "        seed is the random number seed"

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M"' )
    return date_pipe.readlines()[0].split()[0]

def version():
    return "$Id: createRandomLayeredGraph.py 133 2016-01-13 20:09:11Z mfms $"

# for debugging
def debug_print( format_string, tuple_of_values ):
    if DEBUG:
        print format_string % tuple_of_values

# prints the name and value of a variable, separated by a space to the file
# stream
def printvar( file_stream, var_name ):
    file_stream.write( var_name )
    file_stream.write( " " )
    file_stream.write( str(eval( var_name )) )
    file_stream.write( "\n" )

# returns true with probability p; if p < 0, returns false, if p > 1, returns
# true
def true_with_probability( p ):
    random_real = random.random() # in the interval [0,1)
    if random_real < p:
        return True
        return False

# returns the list L with all occurrences of x removed
def removed_from( x, L ):
    return filter( lambda y: y != x, L )

# returns the median of L, a list of numbers, assumes L non-empty 
def median(L):
    sorted_version = sorted(L)
    length = len(sorted_version)
    if length % 2 == 1:
        # odd length
        return sorted_version[ (length - 1) / 2 ]
    else:
        # even length
        return (sorted_version[ length / 2 - 1 ] + sorted_version[ length / 2]) / 2.0

# L is a list of numbers
#
# returns a tuple of the form (min, min_at, max, max_at, mean, median), where
# min, max, mean, and median are the usual statistics; min_at and max_at are
# lists of indices where the minimum and maximum occur
# Assumes L is not empty
def list_statistics( L ):
    minimum = min(L)
    min_at = [i for i, x in enumerate(L) if x == minimum] 
    maximum = max(L)
    max_at = [i for i, x in enumerate(L) if x == maximum]
    mean = sum(L) / float(len(L))
    med = median(L)
    return (minimum, min_at, maximum, max_at, mean, med)

# print statistics (see list_statistics) with appropriate labels; name is a
# string representing the quantity for which statistics are to be printed and
# L is the list of numbers from which the statistics come.
def print_statistics( file_stream, name, L ):
    stats = list_statistics( L )
    file_stream.write( "min_%s %5.3f\n" % (name, stats[0]) )
    file_stream.write( "min_%s_at %s\n" % (name, str(stats[1])) )
    file_stream.write( "max_%s %5.3f\n" % (name, stats[2]) )
    file_stream.write( "max_%s_at %s\n" % (name, str(stats[3])) )
    file_stream.write( "avg_%s %5.3f\n" % (name, stats[4]) )
    file_stream.write( "med_%s %5.3f\n" % (name, stats[5]) )

# the following are global arrays (lists)
#    layer[i] is the list of (indices of) nodes on layer i
#    my_layer[i] is the layer that contains node i
#    adj_in[i], where my_layer[i] = k,
#               is the list of nodes on layer k-1 that are adjacent to i
#    potential_in[i] is the list of nodes on layer k-1 not adjacent to i 
#    adj_out[i] where my_layer[i] = k,
#               is the list of nodes on layer k+1 that are adjacent to i
#    potential_out[i] is the list of nodes on layer k+1 not adjacent to i 

def main():
    global _nodes
    global _edges
    global _layers
    if len( sys.argv ) != 7:
        usage( sys.argv[0] )
        sys.exit()

    _nodes = int(sys.argv[1])
    _edges = int(sys.argv[2])
    _layers = int(sys.argv[3])
    width_var = float(sys.argv[4])
    degree_var = float(sys.argv[5])
    seed = int(sys.argv[6])

    # check for various anomalies
    if _layers < 2:
        print "Need at least two layers. You asked for", _layers
        sys.exit()
    if int( _nodes / _layers ) < 3:
        print "Need to allow for at least three nodes per layer."
        print "You asked for", _nodes, "nodes and", _layers, "layers."
        sys.exit()

    random.seed( seed )
    initialize_globals()
    create_layers( width_var )
    add_tree_edges( degree_var )
    add_other_edges( degree_var )
    print_graph( sys.stdout, width_var, degree_var, seed )
    print_all_stats( sys.stderr )

# returns a string of the form XpY from the floating point number z = X.Y...
def convert_float_to_string( z ):
    X = int( z )
    Y = int( (z - X) * 10 )
    return "%dp%d" % (X, Y)

# returns a string of the form r_NODES_EDGES_LAYERS_WIDTHVAR_DEGREEVAR_SEED
# to be used as a graph name. Floating point numbers are of the form XpY,
# where X is the digit preceding the decimal point and Y is the first digit
# after it.
def create_name( width_var, degree_var, seed ):
    width_var_string = convert_float_to_string( width_var )
    degree_var_string = convert_float_to_string( degree_var )
    return "r_%d_%d_%d_%s_%s_%d" % (_nodes, _edges, _layers, width_var_string, degree_var_string, seed)

# prints the graph in sgf format into the file stream.
#
# lines beginning with n give info about nodes,
# those beginning with e give info about edges
# comments are used to document the provenance of the graph
#    c comment line 1
#    ...
#    c comment line k
#
#    t graph_name
#    n id_1 layer_1 position_1
#    n id_2 layer_2 position_2
#    ...
#    n id_n layer_n position_n
#    e source_1 target_1
#    ...
#    e source_m target_m
def print_graph( file_stream, width_var, degree_var, seed ):
    file_stream.write( "c generated by $Id: createRandomLayeredGraph.py 133 2016-01-13 20:09:11Z mfms $ on %s\n" % date() )
    file_stream.write( "c %s\n" % version() )
    file_stream.write( "c nodes edges layers width_var degree_var seed\n" )
    file_stream.write( "c %d %d %d %4.2f %4.2f %d\n" % (_nodes, _edges, _layers, width_var, degree_var, seed) )
    graph_name = create_name( width_var, degree_var, seed ); 
    file_stream.write( "t %s\n" % ( create_name( width_var, degree_var, seed ) ) )
    print_nodes( file_stream )
    print_edges( file_stream )
    # print "adj_in =", adj_in
    # print "adj_out =", adj_out

def print_nodes( file_stream ):
    global layer
    for k in range(_layers):
        this_layer = layer[k]
        position = 0
        for i in range(len(this_layer)):
            node = this_layer[i]
            file_stream.write( "n %d %d %d\n" % (node, k, position) )
            position = position + 1

def print_edges( file_stream ):
    global adj_out
    for source in range(_nodes):
        outgoing_neighbors = adj_out[source]
        for target_index in range(len(outgoing_neighbors)): 
            file_stream.write( "e %d %d\n" % (source, outgoing_neighbors[target_index]) )

# returns the number of edges between layers k and k+1 divided by the number
# of potential edges; node_list = the list of nodes on layer k
def channel_density( node_list ):
    global adj_out
    global my_layer
    number_of_edges = sum( map( outdegree, node_list ) )
    # print "channel density, num_edges =", number_of_edges, ", node_list =", node_list
    # use the first node of the layer as an arbitrary node to get layer number
    k = my_layer[ node_list[0] ]
    potential_edges = len( layer[k] ) * len( layer[k+1] )
    return number_of_edges / float( potential_edges )

# expression is some expression involving a node, e.g., "len(adj_in[ node ])"
# to get stats on the indegree for nodes, the list of nodes on the layer
def layer_stats( expression, node_list ):
    data_list = map( lambda node: eval( expression ), node_list )
    return list_statistics( data_list )

# degree discrepancy in a channel is max_degree / median_degree
# all the 0's are filtered out first to ensure that the discrepancy makes sense
# if the resulting list is empty, a -1 is reported
def degree_discrepancy( node_list ):
    k = my_layer[ node_list[0] ]
    out_degrees = map( outdegree, node_list )
    in_degrees = map( indegree, layer[k+1] )
    degree_list = out_degrees + in_degrees
    degree_list = filter( lambda x: x != 0, degree_list )
    # print "degree_discrepancy, k =", k, ", degree_list =", degree_list
    if degree_list == 0:
        return -1
    median_degree = median( degree_list )
    return max( degree_list ) / float( median_degree )

# returns the maximum indegree among nodes on the list
def get_max_indegree( node_list ):
    indegrees = map( indegree, node_list )
    return max( indegrees )

# returns the maximum outdegree among nodes on the list
def get_max_outdegree( node_list ):
    outdegrees = map( outdegree, node_list )
    return max( outdegrees )

# returns the maximum total degree among nodes on the list
def get_max_degree( node_list ):
    degrees = map( degree, node_list )
    return max( degrees )

def indegree_zero_nodes( node_list ):
    return len( filter( lambda node: indegree( node ) == 0, node_list ) )

def outdegree_zero_nodes( node_list ):
    return len( filter( lambda node: outdegree( node ) == 0, node_list ) )

def channel_edge_count( node_list ):
    return sum( map( outdegree, node_list ) )

# prints a varety of statistics about layers and degrees to the file_stream
def print_all_stats( file_stream ):
    print_statistics( file_stream, "layer_width", map( len, layer ) )
    file_stream.write( "channel_edge_counts = %s\n" % (map( channel_edge_count, layer[0:_layers-1]) ) )
    print_statistics( file_stream, "channel_density", map( channel_density, layer[0:_layers-1] ) )
    print_statistics( file_stream, "indeg_0_nodes", map( indegree_zero_nodes, layer[1:_layers] ) )
    print_statistics( file_stream, "outdeg_0_nodes", map( outdegree_zero_nodes, layer[0:_layers-1] ) )
    print_statistics( file_stream, "max_in_degree", map( get_max_indegree, layer[1:_layers] ) )
    print_statistics( file_stream, "max_out_degree", map( get_max_outdegree, layer[0:_layers-1]) )
    print_statistics( file_stream, "max_degree", map( get_max_degree, layer ) )
    print_statistics( file_stream, "degree_discrepancy", map( lambda node_list: degree_discrepancy( node_list ), layer[0:_layers-1] ) ) 

def initialize_globals():
    global layer
    global my_layer
    global adj_in
    global adj_out
    layer = [[]] * _layers
    my_layer = [-1] * _nodes
    # the following need to be created more carefully; the lists will get
    # appended to later
    adj_in = []
    adj_out = []
    for v in range(_nodes):
        adj_in.append([])
        adj_out.append([])

def create_layers( width_var ):
    global layer
    global my_layer
    avg_nodes_per_layer = _nodes / float(_layers)
    width_deviance = avg_nodes_per_layer * width_var
    nodes_added_so_far = 0
    nodes_remaining = _nodes
    layers_remaining = _layers
    for k in range(_layers-1):
        average_remaining_width = nodes_remaining / float( layers_remaining )
        # print "k =", k
        # print "layers_remaining =", layers_remaining
        # print "nodes remaining =", nodes_remaining
        # print "nodes_added_so_far =", nodes_added_so_far
        # print "average_remaining_width =", average_remaining_width
        layer_size = random.randrange( int(average_remaining_width - width_deviance), int(average_remaining_width + width_deviance + 1 ) )

        # ensure that (i) every layer has at least three nodes; and (ii) all
        # subsequent layers can have at least four nodes (to allow some
        # variance if desired)
        if layer_size < 3: layer_size = 3
        if nodes_remaining - layer_size < 4 * (_layers - 1 - k):
            layer_size = nodes_remaining - 4 * (_layers - 1 - k)
        # ensure that remaining layers are no wider than
        # average_remaining_width + width_deviance, i.e., make this layer
        # larger if necessary so that subsequent layers don't get too large
        if (nodes_remaining - layer_size) / float(layers_remaining - 1)  > average_remaining_width + width_deviance:
            layer_size = int( average_remaining_width + width_deviance )

        # print "layer", k, "has", layer_size, "nodes, ", nodes_added_so_far, "nodes have been added"

        add_nodes_to_layer( k, nodes_added_so_far, layer_size )
        nodes_added_so_far = nodes_added_so_far + layer_size
        nodes_remaining = nodes_remaining - layer_size
        layers_remaining = layers_remaining - 1

    # print "layer", layers - 1, "has", nodes_remaining, "nodes"
    add_nodes_to_layer( _layers - 1, nodes_added_so_far, nodes_remaining )

def add_node_to_layer( node, k ):
    my_layer[ node ] = k

# determines which nodes are on layer k and records the appropriate
# information
def add_nodes_to_layer( k, nodes_added_so_far, layer_size ):
    global layer
    global my_layer
    # print "add_nodes_to_layer, k =", k, ", added_so_far =", nodes_added_so_far, ", layer_size =", layer_size
    layer[k] = range(nodes_added_so_far, nodes_added_so_far + layer_size)
    debug_print( "layer %d is %s", ( k, layer[k] ) )
    map( lambda node: add_node_to_layer( node, k), layer[k] )
    #   print "my_layer =", my_layer
    #   print "layer =", layer

def indegree( node ):
    global adj_in
    return len( adj_in[node] )

def outdegree( node ):
    global adj_out
    return len( adj_out[node] )

def degree( node ):
    return indegree( node ) + outdegree( node )

def get_incoming_layer( k ):
    if k == 0:
        return []
    return layer[ k - 1 ]

def get_outgoing_layer( k ):
    if k == _layers - 1:
        return []
    return layer[ k + 1 ]

def nodes_needing_in_edges():
    global my_layer
    global adj_in
    return filter( lambda node: my_layer[node] != 0 and adj_in[node] == [], range( _nodes ) )

def nodes_needing_out_edges():
    global my_layer
    global adj_out
    return filter( lambda node: my_layer[node] != _layers - 1 and adj_out[node] == [], range( _nodes ) )

def isolated_nodes():
    return filter( lambda node: degree( node ) == 0, range( _nodes ) );

# returns true if the node is *not* already adjacent to all nodes on the
# preceding and following layers
def is_available( node ):
    return len( adj_in[node] ) + len( adj_out[node] ) < len( get_incoming_layer( my_layer[node] ) ) + len( get_outgoing_layer( my_layer[node] ) )

# repeatedly picks a node not in the current tree and attaches it to a tree
# node; the frontier is the set of nodes that are on layers adjacent to
# layers of tree nodes; the set of tree nodes is maintained by the buckets (a
# node not in the tree is not in any bucket
def add_tree_edges( degree_var ):
    global _nodes
    # first create an initial edge between a node on layer 0 and a node on
    # layer 1
    start_node = random.choice( layer[0] )
    next_node = random.choice( layer[1] )
    init_buckets( [ start_node, next_node ] )
    add_edge( start_node, next_node )
    if _layers > 2:
        frontier_layer = 2
        frontier = layer[0] + layer[1] + layer[2]
    else:
        frontier_layer = 1
        frontier = layer[0] + layer[1]
    frontier = removed_from( start_node, frontier )
    frontier = removed_from( next_node, frontier )
    for i in range( _nodes - 2 ):
        if DEBUG:
            print "adding tree edges: i =", i, ", frontier =", frontier
        frontier_node = random.choice( frontier )
        frontier = removed_from( frontier_node, frontier )
        tree_node_choices = get_adjacent_node_choices( frontier_node, degree_var )
        tree_node = random.choice( tree_node_choices )
        add_edge( frontier_node, tree_node )
        if my_layer[ frontier_node ] >= frontier_layer and frontier_layer < _layers - 1:
            frontier_layer = frontier_layer + 1
            frontier = frontier + layer[ frontier_layer ]

def add_other_edges( degree_var ):
    # init_buckets( range( nodes ) )
    # init_sets( nodes )

    for i in range( _edges - _nodes + 1 ): 
        # print "adding_edges: i =", i
        # print "need_in =", need_in
        # print "need_out =", need_out 
        # print "lengths =", len( need_in ) + len( need_out )
        # need_in = nodes_needing_in_edges()
        # need_out = nodes_needing_out_edges()
        # print "adding_edges: i =", i
        # print "need_in =", need_in
        # print "need_out =", need_out 
        # print "lengths =", len( need_in ) + len( need_out )
        # if len( need_in ) + len( need_out ) >= edges - i:
            # ensure that all nodes on interior layers have indegree > 0
            # and outdegree > 0 and that nodes on outer layers are not
            # isolated
        #     v = random.choice( get_nodes_in_smallest_components() )
        # else:
        choices = get_node_choices( degree_var )
        if choices == []:
            return
        v = random.choice( choices )
        adjacent_node_choices = get_adjacent_node_choices( v, degree_var )
        if DEBUG:
            print "adding edges: v =", v, "adjacent choices =", adjacent_node_choices
        # filtered = filter_for_connectivity( v, adjacent_node_choices )
        # if filtered != []:
        #     adjacent_node_choices = filtered
        # print "post-filter: v =", v, "adjacent choices =", adjacent_node_choices
        w = random.choice( adjacent_node_choices )
        #        print "remaining edges =", edges - i
        #        print "chosen node =", v, ", neighbor =", w
        add_edge( v, w )
        # union( v, w )
        #        print "adj_out[%d] =" % (v), adj_out[v]
        #        print "adj_in[%d] = " % (w), adj_in[w]

def filter_for_connectivity( v, choices ):
    return filter( lambda w: find( w ) != find( v ), choices )

def add_edge( v, w ):
    global adj_in
    global adj_out
    if DEBUG:
        print "add_edge", v, w

    # the following code is designed to keep the degree of nodes on small
    # layers from getting too large; if layer widths differ a lot, a small
    # layer between two larger ones will have every node connected to every
    # node on the larger layers.
    # layer_width_v = len( layer[ my_layer[v] ] )
    # layer_width_w = len( layer[ my_layer[w] ] )
    # if layer_width_v > layer_width_w:
    #     if true_with_probability( layer_width_v / float( layer_width_w ) - 1 ):
    #         increment_degree(v)
    #         increment_degree(w)

    if my_layer[v] > my_layer[w]:
        temp = v
        v = w
        w = temp
    # at this point vw is directed from lower numbered to higher numbered layer
    adj_in[w].append(v)
    adj_out[v].append(w)
    increment_degree(v)
    increment_degree(w)
    # without the following adjustment, nodes on the first and last layer end
    # up with too many edges (their degree never gets incremented for
    # incoming or outgoing edges, respectively)
    if my_layer[v] == 0:
        increment_degree(v)
    if my_layer[w] == _layers - 1:
        increment_degree(w)

# The degree bucket data structure is the key to controlling degree
# distributions. Here, bucket[k] stores nodes whose total degree is k and
# my_bucket[node] stores the index of the bucket containing the node or -1 if
# it is no longer available for having edges added to it; my_bucket is
# initially 0 even if the node is not in a bucket yet.
#
# When degree variance is small, nodes in buckets with small indices are
# given preference. If degree variance is 1.0 all nodes are treated
# equally. If it's greater than 1, the probability of a node from bucket[k]
# being chosen is 2^k/2^m, where m is the maximum index + 1.
#
# @todo Obviously this should be a class, but the current implementation is
# meant to be quick and dirty

def init_buckets( node_list ):
    global bucket
    global my_bucket
    global lowest_bucket_index
    global highest_bucket_index
    bucket = [ node_list ]
    my_bucket = [0] * _nodes
    highest_bucket_index = 0
    #    print "bucket =", bucket
    #    print "my_bucket =", my_bucket

# moves the node into the next higher bucket
def increment_degree( node ):
    global bucket
    global my_bucket
    global highest_bucket_index
    k = my_bucket[node]
    bucket[k] = removed_from( node, bucket[k] )
    if highest_bucket_index == k:
        bucket.append( [] )
        highest_bucket_index = highest_bucket_index + 1
    bucket[k+1].append( node )
    my_bucket[node] = k + 1

    # print "bucket =", bucket
    # print "my_bucket[%d] =" % (node), my_bucket[node]
    # print "lowest_bucket_index = ", lowest_bucket_index
    # print "highest_bucket_index =", highest_bucket_index

# returns all the available nodes on the list: those not yet
# adjacent to all nodes on the layer above and the layer below.
def get_available_nodes( node_list ):
    return filter( lambda node: is_available( node ), node_list )

# returns a random bucket from a (filtered) list of buckets; the bucket
# chosen is based on a mapping from the unit interval to the range of indices
# of buckets_to_choose_from
def choose_random_bucket( buckets_to_choose_from, degree_var ):
    if buckets_to_choose_from == []:
        return []
    uniform_number = random.random()
    choice_index = int( uniform_number * degree_var * len( buckets_to_choose_from ) + 0.5 )
    if choice_index >= len( buckets_to_choose_from ):
        choice_index = len( buckets_to_choose_from ) - 1
    return buckets_to_choose_from[ choice_index ]
    
# returns a random bucket from those that contain available nodes
# determined by degree variance
def get_node_choices( degree_var ):
    # print "get_node_choices, bucket =", bucket
    buckets_to_choose_from = map( get_available_nodes, bucket )
    buckets_to_choose_from = filter( lambda node_list: node_list != [], buckets_to_choose_from )
    # print "get_node_choices: buckets_to_choose_from =", buckets_to_choose_from
    return choose_random_bucket( buckets_to_choose_from, degree_var )

# returns node choices based on degree variance and several constraints:
# - if
# v has no incoming edges it must be connected to a node on the previous
# layer - nodes must be available - nodes must be chosen from a non-empty
# bucket, depending on degree variance
def get_adjacent_node_choices( v, degree_var ):
    if DEBUG:
        print "-> get_adjacent_node_choices, v =", v, ", degree_var =", degree_var
    if my_layer[v] > 0 and adj_in[v] == [] and adj_out[v] != []:
        choices = get_incoming_layer( my_layer[v] )
        desperate = filter( lambda node: adj_out[node] == 0, choices )
    elif my_layer[v] < _layers - 1 and adj_out[v] == [] and adj_in[v] != []:
        choices = get_outgoing_layer( my_layer[v] )
        desperate = filter( lambda node: adj_in[node] == 0, choices )
    else:
        in_choices = list( set( get_incoming_layer( my_layer[v] ) ) - set( adj_in[v] ) )
        desperate = filter( lambda node: adj_out[node] == 0, in_choices )
        out_choices = list( set( get_outgoing_layer( my_layer[v] ) ) - set( adj_out[v] ) )
        desperate = desperate + filter( lambda node: adj_in[node] == 0, out_choices )
        choices = in_choices + out_choices

        if DEBUG:
            print "  get_adjacent_node_choices: choices =", choices
            print "                             desperate =", desperate
    if desperate != []:
        return desperate

    buckets_to_choose_from = map( lambda node_list: list( set( node_list ) & set( choices ) ), bucket )
    buckets_to_choose_from = filter( lambda node_list: node_list != [], buckets_to_choose_from )
    if DEBUG:
        print "  get_adjacent_node_choices: buckets_to_choose_from =", buckets_to_choose_from
    # at this point, buckets is a list of non-empty buckets each of which
    # consists entirely of potential neighbors of v
    return choose_random_bucket( buckets_to_choose_from, degree_var)


# disjoint set union (without union by rank)
# size_list[i] = list of nodes whose component size is i 
def init_sets( _nodes ):
    global parent
    global my_component_size
    global size_list
    global smallest_component_size
    parent = range( _nodes )
    my_component_size = [ 1 ] * _nodes
    smallest_component_size = 1
    size_list = []
    for v in range(_nodes):
        size_list.append([])
    size_list[ 1 ] = range( _nodes )

def find( x ):
    global parent
    if x == parent[x]:
        return x
    root = find( parent[x] )
    parent[x] = root
    return root

def link( x, y ):
    global parent
    parent[x] = y

def union( x, y ):
    global my_component_size
    global size_list
    global smallest_component_size
    link( find( x ), find( y ) )
    size_x = my_component_size[ x ]
    size_y = my_component_size[ y ]
    total_size = size_x + size_y
    size_list[ size_x ] = list( set( size_list[ size_x ] ) - set( [ x ] ) )
    size_list[ size_y ] = list( set( size_list[ size_y ] ) - set( [ y ] ) )
    my_component_size[ x ] = total_size
    my_component_size[ y ] = total_size
    size_list[ total_size ].append( x )
    size_list[ total_size ].append( y )
    while size_list[ smallest_component_size ] == []:
        smallest_component_size = smallest_component_size + 1
        
def get_nodes_in_smallest_components():
    return size_list[ smallest_component_size ]

main()

#  [Last modified: 2020 12 21 at 16:06:27 GMT]
