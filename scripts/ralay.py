#! /usr/bin/env python3

"""
ralay.py - offers a mechanism for producing layered graphs with
different variances with respect to number of nodes per layer, number of
dummy nodes, degrees of nodes, shapes, etc.. Output is in sgf format, which
can be converted to dot or graphml, as desired. Graphs are guaranteed to be
connected (and interesting) because each node has indegree 1 (unless on layer 0)
and outdegree 1 (unless on the highest layer).

@author Matthias Stallmann
@date 2018/6/18
"""

import sys
import os
import random
from argparse import ArgumentParser
from argparse import RawTextHelpFormatter # to allow newlines in help messages
from functools import reduce    # for now - to get degree stats using reduce

import layeredGraph

DEBUG = True

# for debugging
def debug_print(format_string, tuple_of_values):
    if DEBUG:
        sys.stderr.write(format_string % tuple_of_values)

def parse_arguments():
    parser = ArgumentParser(
        formatter_class=RawTextHelpFormatter,
        description = "Creates a random (connected) layered graph following,\n"
        + " as much as possible, the specifications supplied by the user\n"
        + " output is stdout",
        epilog = "Each node has\n"
        + "  indegree at least 1 (unless on layer 0)\n"
        + "  outdegree at least 1 (unless on the highest layer)"
    )
    parser.add_argument("nodes", help = "number of nodes", type = int)
    parser.add_argument("edges", help = "number of edges", type = int)
    parser.add_argument("layers", help = "number of layers", type = int)
    parser.add_argument("-s", "--seed",
                        help = "random seed (default is based on internal system state)",
                        type = int)
    parser.add_argument("-dv", "--deg_variance",
                        help = "desired degree variance,\n"
                        + " 0 means regular (default),\n"
                        + " 1 means distribution is roughly uniform,\n"
                        + " > 1 means roughly power law", type=float, default = 0)
    parser.add_argument("-mw", "--min_width",
                        help = "miminum width of a layer;\n"
                        + " layer width is uniformly random between min and max width",
                        type = int)
    parser.add_argument("-MW", "--max_width",
                        help = "maximum width of a layer;\n"
                        + " default is for both min and max width to be roughly nodes / layers",
                        type = int)
    parser.add_argument("--shape",
                        help = "profile of layer widths, options are:\n"
                        + " sorted (ascending),\n"
                        + " balloon (widest in middle layers),\n"
                        + " hourglass (widest in outer layers),\n"
                        + " sawtooth (two ascending sequences),"
                        + " alternating (small, large, small, ...),\n"
                        + " or random (default)",
                        default = "random",
                        choices = ['sorted', 'balloon', 'hourglass', 'sawtooth', 'alternating', 'random'])
    parser.add_argument("--profile",
                        help = "comma-separated list of layer widths,\n"
                        + " overrides nodes, layers, min and max width, and shape")
    parser.add_argument("--dummies",
                        help = "number of dummy nodes (default 0); if > 0,\n"
                        + " some edges will skip layers and dummy nodes will be inserted",
                        type = int, default = 0)
    args = parser.parse_args()
    return args

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M"' )
    return date_pipe.readlines()[0].split()[0]

def version():
    return "ralay.py v1.1.1 mfms"

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

# returns the median of L, a list of numbers, assumes L non-empty 
def median(L):
    sorted_version = sorted(L)
    length = len(sorted_version)
    if length % 2 == 1:
        # odd length
        return sorted_version[ (length - 1) // 2 ]
    else:
        # even length
        return (sorted_version[ length // 2 - 1 ] + sorted_version[ length // 2]) / 2.0

def removed_from(x, L):
    """
    @return the list L with all occurrences of x removed; if x does not
    occur, L is returned; this makes it more flexible (though less efficient
    than) the built-in remove
    """
    return [y for y in L if y != x]

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

# returns a string of the form XpY from the floating point number z = X.Y...
def convert_float_to_string( z ):
    X = int( z )
    Y = int( (z - X) * 10 )
    return "%dp%d" % (X, Y)

# returns a string of the form r_NODES_EDGES_LAYERS_WIDTHVAR_DEGREEVAR_SEED
# to be used as a graph name. Floating point numbers are of the form XpY,
# where X is the digit preceding the decimal point and Y is the first digit
# after it.
def create_name( degree_var, seed ):
    degree_var_string = convert_float_to_string( degree_var )
    return "r_{}_{}_{}_{}_{}".format(_nodes, _edges, _layers, degree_var_string, seed)

# prints the graph in sgf format into the file stream.
#
# lines beginning with n give info about nodes,
# those beginning with e give info about edges
# comments are used to document the provenance of the graph
#    c comment line 1
#    ...
#    c comment line k
#
#    t graph_name num_nodes num_edges num_layers
#    n id_1 layer_1 position_1
#    n id_2 layer_2 position_2
#    ...
#    n id_n layer_n position_n
#    e source_1 target_1
#    ...
#    e source_m target_m
def print_graph( file_stream, degree_var, seed ):
    file_stream.write("c generated by {} on {}\n".format(version(), date()))
    file_stream.write( "c nodes edges layers degree_var seed\n" )
    file_stream.write( "c {} {} {} {:4.2f} {}\n"
                       .format(_nodes, _edges, _layers, degree_var, seed) )
    graph_name = create_name(degree_var, seed); 
    file_stream.write("t %s %d %d %d\n"
                       .format(create_name(degree_var, seed),
                       _nodes, _edges, _layers))
    print_nodes(file_stream)
    print_edges(file_stream)

def print_nodes(file_stream):
    global layer
    for k in range(_layers):
        this_layer = layer[k]
        position = 0
        for i in range(len(this_layer)):
            node = this_layer[i]
            file_stream.write( "n %d %d %d\n" % (node, k, position) )
            position = position + 1

def print_edges(file_stream):
    global adj_out
    for source in range(_nodes):
        outgoing_neighbors = adj_out[source]
        for target_index in range(len(outgoing_neighbors)): 
            file_stream.write( "e %d %d\n" % (source, outgoing_neighbors[target_index]) )

###############################################
# STATISTICS
###############################################

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
    data_list = [eval( expression ) for node in node_list]
    return list_statistics( data_list )

# degree discrepancy in a channel is max_degree / median_degree
# all the 0's are filtered out first to ensure that the discrepancy makes sense
# if the resulting list is empty, a -1 is reported
def degree_discrepancy( node_list ):
    k = my_layer[ node_list[0] ]
    out_degrees = list(map( outdegree, node_list ))
    in_degrees = list(map( indegree, layer[k+1] ))
    degree_list = out_degrees + in_degrees
    degree_list = [x for x in degree_list if x != 0]
    # print "degree_discrepancy, k =", k, ", degree_list =", degree_list
    if degree_list == 0:
        return -1
    median_degree = median( degree_list )
    return max( degree_list ) / float( median_degree )

# returns the maximum indegree among nodes on the list
def get_max_indegree( node_list ):
    indegrees = list(map( indegree, node_list ))
    return max( indegrees )

# returns the maximum outdegree among nodes on the list
def get_max_outdegree( node_list ):
    outdegrees = list(map( outdegree, node_list ))
    return max( outdegrees )

# returns the maximum total degree among nodes on the list
def get_max_degree( node_list ):
    degrees = list(map( degree, node_list ))
    return max( degrees )

def indegree_zero_nodes( node_list ):
    return len( [node for node in node_list if indegree( node ) == 0] )

def outdegree_zero_nodes( node_list ):
    return len( [node for node in node_list if outdegree( node ) == 0] )

def channel_edge_count( node_list ):
    return sum( map( outdegree, node_list ) )

# prints a varety of statistics about layers and degrees to the file_stream
def print_all_stats( file_stream ):
    print_statistics( file_stream, "layer_width", list(map( len, layer )) )
    file_stream.write( "channel_edge_counts = %s\n" % (list(map( channel_edge_count, layer[0:_layers-1])) ) )
    print_statistics( file_stream, "channel_density", list(map( channel_density, layer[0:_layers-1] )) )
    print_statistics( file_stream, "indeg_0_nodes", list(map( indegree_zero_nodes, layer[1:_layers] )) )
    print_statistics( file_stream, "outdeg_0_nodes", list(map( outdegree_zero_nodes, layer[0:_layers-1] )) )
    print_statistics( file_stream, "max_in_degree", list(map( get_max_indegree, layer[1:_layers] )) )
    print_statistics( file_stream, "max_out_degree", list(map( get_max_outdegree, layer[0:_layers-1])) )
    print_statistics( file_stream, "max_degree", list(map( get_max_degree, layer )) )
    print_statistics( file_stream, "degree_discrepancy", [degree_discrepancy( node_list ) for node_list in layer[0:_layers-1]] ) 

######################################
# END STATISTICS
######################################

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

def create_profile(min_width, max_width, shape):
    """
    @return a list 'width' of layer widths, so that width[k] is the number of
    nodes in layer k; constraint is min_width <= width[k] <= max_width; shape
    will govern whether the layer widths are in random order or
    structured/sorted in some fashion (see args.shape).
    """
    nodes_remaining = _nodes
    layers_remaining = _layers - 1
    width = []
    for k in range(_layers - 1):
        # nodes_remaining is _nodes - number of nodes already assigned to layers
        # layers_remaining is the number of layers remaining *after* the current one
        lower_limit = max(min_width, nodes_remaining - max_width * layers_remaining)
        upper_limit = min(max_width, nodes_remaining - min_width * layers_remaining)
        layer_size = random.randrange(lower_limit, upper_limit + 1)
        width.append(layer_size)
        nodes_remaining = nodes_remaining - layer_size
        layers_remaining = layers_remaining - 1
    width.append(nodes_remaining)
    if shape == "random":
        return width
    elif shape == "sorted":
        return sorted(width)
    # for balloon/hourglass can use width.insert(0,x) to prepend items
    elif shape == "balloon":
        return balloon(sorted(width))
    elif shape == "hourglass":
        return balloon(sorted(width, reverse=True))
    elif shape == "sawtooth":
        # use for loops with ranges that step by 2, or apply ballooning in reverse 
        return []
    elif shape == "alternating":
        # want to maintain some of the randomness here, so want to
        # alternately choose an integer >= median and one <= median;
        # maybe first create a list with everything < median at the
        # beginning, i.e., go through list and prepend if < median, append
        # otherwise;
        # then can alternate choosing an integer from the beginning and the end
        return []
    else:
        print("unrecognized shape", shape)
        sys.exit()

def balloon(sorted_width_list):
    """
    @return a list of the integers in sorted_width_list, arranged so that
    integers that occur earlier in sorted_width_list appear at the beginning
    and end, while those that occur later appear in the middle
    """
    # start with a list of 0's and systematically fill in from both ends
    balloon_list = len(sorted_width_list) * [0]
    input_index = 0
    low_index = 0
    high_index = len(sorted_width_list) - 1
    while high_index >= low_index:
        next_index = random.choice([low_index, high_index])
        balloon_list[next_index] = sorted_width_list[input_index]
        input_index += 1
        if next_index == low_index:
            low_index += 1
        else:
            high_index -= 1
    return balloon_list

def create_layers(layer_profile):
    """
    Adds nodes to layers based on the layer_profile:
    specifically, layer k ends up with layer_profile[k] nodes
    """
    global layer
    global my_layer
    nodes_added_so_far = 0
    for k in range(_layers):
        add_nodes_to_layer(k, nodes_added_so_far, layer_profile[k])
        nodes_added_so_far = nodes_added_so_far + layer_profile[k]

def add_node_to_layer(node, k):
    my_layer[node] = k

# determines which nodes are on layer k and records the appropriate
# information
def add_nodes_to_layer(k, nodes_added_so_far, layer_size):
    global layer
    global my_layer
    layer[k] = list(range(nodes_added_so_far, nodes_added_so_far + layer_size))
    debug_print("layer %d is %s\n", (k, layer[k]))
    list(map(lambda node: add_node_to_layer(node, k), layer[k]))

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
    return [node for node in range( _nodes ) if my_layer[node] != 0 and adj_in[node] == []]

def nodes_needing_out_edges():
    global my_layer
    global adj_out
    return [node for node in range( _nodes ) if my_layer[node] != _layers - 1 and adj_out[node] == []]

def isolated_nodes():
    return [node for node in range( _nodes ) if degree( node ) == 0];

def is_available( node ):
    """
    @returns true if the node is *not* already adjacent to all nodes on the
    preceding and following layers
    """
    return len(adj_in[node]) + len(adj_out[node]) < len(get_incoming_layer(my_layer[node])) + len(get_outgoing_layer(my_layer[node]))

def add_tree_edges(degree_var):
    """
    repeatedly picks a node not in the current tree and attaches it to a tree
    node; the frontier is the set of nodes that are on layers adjacent to
    layers of tree nodes; the set of tree nodes is maintained by the buckets:
    a node not in the tree is not in any bucket
    """
    global _nodes
    # first create an initial edge between a node on layer 0 and a node on
    # layer 1
    start_node = random.choice( layer[0] )
    next_node = random.choice( layer[1] )
    init_buckets( [ start_node, next_node ] )
    debug_print("add_edge %d,%d\n", (start_node, next_node))
    add_edge( start_node, next_node )
    if _layers > 2:
        frontier_layer = 2
        frontier = layer[0] + layer[1] + layer[2]
    else:
        frontier_layer = 1
        frontier = layer[0] + layer[1]
    frontier = removed_from( start_node, frontier )
    frontier = removed_from( next_node, frontier )
    for i in range(_nodes - 2):
        debug_print("i = %d, frontier_layer = %d,\n frontier = %s\n",
                    (i, frontier_layer, str(frontier)))
        frontier_node = random.choice( frontier )
        frontier = removed_from( frontier_node, frontier )
        tree_node_choices = get_adjacent_node_choices( frontier_node, degree_var )
        tree_node = random.choice( tree_node_choices )
        debug_print("add_edge %d,%d\n", (frontier_node, tree_node))
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
            print("adding edges: v =", v, "adjacent choices =", adjacent_node_choices)
        w = random.choice( adjacent_node_choices )
        add_edge(v, w)

def add_edge(v, w):
    """
    adds an edge between v and w, taking care to direct the edge from a lower
    numbered layer to a higher numbered one, and ensuring that not too many
    edges get added to the top and bottom layers
    """
    global adj_in
    global adj_out
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

def init_buckets(node_list):
    """
    puts each node into a bucket corresponding to degree 0
    """
    global bucket
    global my_bucket
    global highest_bucket_index
    bucket = [node_list]
    my_bucket = [0] * _nodes
    highest_bucket_index = 0

def increment_degree(node):
    """
    moves the node into the next higher bucket to reflect the fact that its
    degree has increased
    """
    global bucket
    global my_bucket
    global highest_bucket_index
    k = my_bucket[node]
    bucket[k] = removed_from(node, bucket[k])
    if highest_bucket_index == k:
        bucket.append([])
        highest_bucket_index = highest_bucket_index + 1
    bucket[k+1].append( node )
    my_bucket[node] = k + 1

def get_available_nodes(node_list):
    """
    @return all the nodes on node_list that are not yet adjacent to all nodes
     on the layer above and the layer below.
    """
    return [node for node in node_list if is_available( node )]

def choose_random_bucket(buckets_to_choose_from, degree_var):
    """
    @return a random bucket from a (filtered) list of buckets; the bucket
    chosen is based on a mapping from the unit interval to the range of indices
    of buckets_to_choose_from; degree_var determines the bias in the choice:
    0 => favor nodes of lowest current degree
    0 < dv < 1 => normal distribution with increasing stdev
    1 => treat all nodes equally
    > 1 => exponential distribution with increasing tail
    """
    print(buckets_to_choose_from, degree_var)
    if buckets_to_choose_from == []:
        return []
    if degree_var <= 1:
        uniform_number = random.random()
        choice_index = int(uniform_number * degree_var * len(buckets_to_choose_from))
    else:
        random_exp = random.expovariate(degree_var)
        choice_index = int((1 - random_exp) * len(buckets_to_choose_from))
    choice_index = min(choice_index, len(buckets_to_choose_from) - 1)
    choice_index = max(choice_index, 0)
    return buckets_to_choose_from[choice_index]
    
def get_node_choices(degree_var):
    """
    @return a random bucket from those that contain available nodes
    as determined by degree variance: @see choose_random_bucket
    """
    buckets_to_choose_from = list(map(get_available_nodes, bucket))
    buckets_to_choose_from = [node_list for node_list in buckets_to_choose_from if node_list != []]
    return choose_random_bucket( buckets_to_choose_from, degree_var )

def get_adjacent_node_choices(v, degree_var):
    """
    @return a list of potential neighbors for v based on degree variance and
    several constraints:
    - if v has no incoming edges it must be connected to a node on the
      layer below (assuming there is one)
    - if v has no outgoing edges it must be connected to a node on the
      layer above (assuming there is one)
    - a neighbor is ineligible if it is already a neighbor of v
    """
    debug_print("-> get_adjacent_node_choices, v = %d, degree_var = %d\n", (v, degree_var))
    if my_layer[v] > 0 and adj_in[v] == [] and adj_out[v] != []:
        # not bottom layer and no incoming edges
        choices = get_incoming_layer( my_layer[v] )
        desperate = [node for node in choices if adj_out[node] == 0]
    elif my_layer[v] < _layers - 1 and adj_out[v] == [] and adj_in[v] != []:
        # not top layer and no outgoing edges
        choices = get_outgoing_layer( my_layer[v] )
        desperate = [node for node in choices if adj_in[node] == 0]
    else:
        in_choices = list( set( get_incoming_layer( my_layer[v] ) ) - set( adj_in[v] ) )
        desperate = [node for node in in_choices if adj_out[node] == 0]
        out_choices = list( set( get_outgoing_layer( my_layer[v] ) ) - set( adj_out[v] ) )
        desperate = desperate + [node for node in out_choices if adj_in[node] == 0]
        choices = in_choices + out_choices

        debug_print("  get_adjacent_node_choices: choices = %s\n", choices)
        debug_print("                             desperate = %s\n", desperate)
    if desperate != []:
        return desperate

    buckets_to_choose_from = [list( set( node_list ) & set( choices ) ) for node_list in bucket]
    buckets_to_choose_from = [node_list for node_list in buckets_to_choose_from if node_list != []]
    debug_print("  get_adjacent_node_choices: buckets_to_choose_from = %s\n",
                buckets_to_choose_from)
    # at this point, buckets is a list of non-empty buckets each of which
    # consists entirely of potential neighbors for v
    return choose_random_bucket(buckets_to_choose_from, degree_var)

def print_layer_widths(stream):
    widths = [len(layer[k]) for k in range(_layers)]
    stream.write("widths: {}\n".format(widths))
        
def print_degree_sequences(stream):
    all_nodes = reduce(lambda x,y: x + y, layer)
    indegrees = [indegree(node) for node in all_nodes]
    outdegrees = [outdegree(node) for node in all_nodes]
    degrees = [indegree(node) + outdegree(node) for node in all_nodes]
    stream.write("indeg:  {}\n".format(sorted(indegrees, reverse=True)))
    stream.write("outdeg: {}\n".format(sorted(outdegrees, reverse=True)))
    stream.write("degree: {}\n".format(sorted(degrees, reverse=True)))
        
if __name__ == "__main__":
    global _nodes
    global _edges
    global _layers
    args = parse_arguments()
    _nodes = args.nodes 
    _edges = args.edges
    _layers = args.layers
    # for now
    min_width = args.min_width
    max_width = args.max_width
    if not min_width and not max_width:
        # neither limit on width specified
        # => make each as close to the average as possible
        min_width = _nodes // _layers
        if _nodes // _layers == _nodes / _layers:
            # average is an integer
            max_width = min_width
        else:
            # if not, need to round up
            max_width = _nodes // _layers + 1
    elif not min_width:
        min_width = max(2 * _nodes // _layers - max_width, 1)
        # no min specified: make it so (min + max) / 2 is roughly average
    elif not max_width:
        # ditto if no max specified
        max_width = 2 * _nodes // _layers - min_width

    print(min_width, max_width)
    # check for various anomalies
    if _layers < 2:
        print("Need at least two layers. You asked for", _layers)
        sys.exit()
    if _nodes < min_width * _layers: 
        print("min_width too large: nodes < min_width * layers, i.e., {} < {} * {}"
              .format(_nodes, min_width, _layers))
        sys.exit()
    if _nodes > max_width * _layers: 
        print("max_width too small: nodes > max_width * layers, i.e., {} > {} * {}"
              .format(_nodes, max_width, _layers))
        sys.exit()
    degree_var = args.deg_variance
    seed = args.seed

    print(_nodes, _edges, _layers, degree_var, seed)
    
    random.seed(seed)
    initialize_globals()
    if args.profile:
        layer_profile = [int(x) for x in args.profile.split(",")]
        _nodes = sum(layer_profile)
        _layers = len(layer_profile)
    else:
        layer_profile = create_profile(min_width, max_width, args.shape)
    debug_print("layer_profile = %s\n", str(layer_profile))
    create_layers(layer_profile)
    add_tree_edges(degree_var)
    add_other_edges(degree_var)
    print_graph(sys.stdout, degree_var, seed)

    #    print_all_stats( sys.stderr )
    print_layer_widths(sys.stderr)
    print_degree_sequences(sys.stderr)

#  [Last modified: 2020 12 29 at 23:32:12 GMT]
