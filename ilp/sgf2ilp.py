#! /usr/bin/env python

# Converts a graph in .sgf format to a linear integer program
# This program translate from standard input to standard output.

import sys
import os
import argparse
import math
import random

TOLERANCE = 0
MAX_TERMS_IN_LINE = 100
INDENT = "  "

parser = argparse.ArgumentParser(description='Creates an ILP to minimize some quantity based on an sgf representation of a layered graph',
                                 epilog='reads sgf from standard input, prints lp file on standard output')
parser.add_argument('--objective', choices={'total','bottleneck',
                                            'stretch','bn_stretch',
                                            'quadratic'},
                    default='total',
                    required=True,
                    help='minimize ...'
                    + ' total/bottleneck (total/bottleneck crossings)'
                    + ' stretch/bn_stretch (total/bottleneck edge length)'
                    + ' quadratic (use quadratic programming to miminze total stretch)')
parser.add_argument('--total', type=int,
                    help='constraint on the total number of crossings (default: none)')
parser.add_argument('--bottleneck', type=int,
                    help='constraint on the maximum number of crossings of any edge (default: none)')
parser.add_argument('--stretch', type=float,
                    help='constraint on the total edge length (default: none)')
parser.add_argument('--bn_stretch', type=float,
                    help='constraint on the maximum length of any edge (default: none)')
parser.add_argument('--seed', type=int,
                    help='a random seed if ILP constraints are to be permuted (default: none)')

args = parser.parse_args()

# creates two global data structures that describe the graph
# _node_dictionary: the item whose key is the id of a node and whose value is
# the layer of the node
#
# _edge_list: each item is a tuple of the form:
#       (source, target)

_node_dictionary = {}
_edge_list = []
_comments = []

def read_sgf(input):
    global _comments
    line = read_nonblank( input )
    while ( line ):
        type = line.split()[0]
        if type == 'n':
            process_node(line)
        elif type == 'e':
            process_edge(line)
        elif type == 'c':
            _comments.append(line[1:])
            
        line = read_nonblank(input)

# adds a _node_dictionary entry to map a node id to its layer
def process_node(line):
    global _node_dictionary
    line_fields = line.split()
    id = line_fields[1]
    layer = line_fields[2]
    _node_dictionary[id] = layer

def process_edge( line ):
    global _edge_list
    line_fields = line.split()
    source = line_fields[1]
    target = line_fields[2]
    _edge_list.append((source, target))

# @return the first non-blank line in the input
def read_nonblank( input ):
    line = input.readline()
    while ( line and line.strip() == "" ):
        line = input.readline()
    return line

# computes the global array _layer_factor so that _layer_factor[i] gives the
# multiplier on a position variable in layer i when stretch is computed;
# this will be 1/(L-1), where L is the number of nodes in layer i
def compute_layer_factors():
    global _layer_factor
    # first compute number of layers (layer numbers are 0-based) and the size
    # of each
    max_layer = 0
    for node_id in _node_dictionary:
        layer = int(_node_dictionary[node_id])
        max_layer = max(layer, max_layer)
    number_of_layers = max_layer + 1
    layer_size = [0] * number_of_layers
    for node_id in _node_dictionary:
        layer = int(_node_dictionary[node_id])
        layer_size[layer] += 1
    # then the appropriate denominator for each layer
    denominator = [0] * number_of_layers
    for layer in range(number_of_layers):
        this_layer_size = layer_size[layer]
        if this_layer_size < 1:
            sys.exit("Error: layer " + layer + " has no nodes")
        denominator[layer] = this_layer_size - 1
        # layers with only a single node should put that node in the center
        if denominator[layer] == 0:
            denominator[layer] = 2
    # now we're ready to compute the actual factors
    _layer_factor = [0] * number_of_layers
    for layer in range(number_of_layers):
        _layer_factor[layer] = 1.0 / denominator[layer]


# In what follows a constraint is a tuple of the form
#  (left, relational_operator, right)
# where left is a list of strings,
#       relational_operator is one of '>=', '<=', '=', etc.
#       and right is a string representing a number.
# each string in left is a +, -, a +c or -c (where c is a constant)
# followed by a variable name.

# variables are added to these sets when they arise in constraints
_binary_variables = []
_integer_variables = []
_continuous_variables = []
_raw_stretch_variables = []

# @return a list of constraints on relative position variables x_i_j, where
# x_i_j is 1 if node i precedes node j on their common layer, 0 otherwise
def triangle_constraints():
    global _binary_variables
    triangle_constraints = []
    # anti-symmetry constraints
    for i in _node_dictionary:
        i_layer = _node_dictionary[i]
        for j in _node_dictionary:
            j_layer = _node_dictionary[j]
            # add a constraint if i and j on same layer; only once per pair
            if i_layer == j_layer and i < j:
                x_i_j = "x_" + str(i) + "_" + str(j)
                x_j_i = "x_" + str(j) + "_" + str(i)
                current_constraint = (["+" + x_i_j, "+" + x_j_i], '=', '1')
                triangle_constraints.append(current_constraint)
                _binary_variables.append(x_i_j)
                _binary_variables.append(x_j_i)

    # transitivity constraints
    # x_i_j and x_j_k implies x_i_k
    # or x_i_k - x_i_j - x_j_k >= -1
    for i in _node_dictionary:
        i_layer = _node_dictionary[i]
        for j in _node_dictionary:
            j_layer = _node_dictionary[j]
            for k in _node_dictionary:
                k_layer = _node_dictionary[k]
                # add two constraints if the three nodes are on the same
                # layer; only once per triple
                if i_layer == j_layer == k_layer and i < j and j < k:
                    relop = '>='
                    right = '-1'

                    left = ["+x_" + i + "_" + k]
                    left.append("-x_" + i + "_" + j)
                    left.append("-x_" + j + "_" + k)
                    triangle_constraints.append((left, relop, right))

                    left = ["+x_" + i + "_" + j]
                    left.append("-x_" + i + "_" + k)
                    left.append("-x_" + k + "_" + j)
                    triangle_constraints.append((left, relop, right))

    # the above two variants suffice
    #  x_i_k - x_i_j - x_j_k = (1 - x_k_i) - x_i_j - (1 - x_k_j)
    #                        = x_k_j - x_k_i - x_i_j
    #                        = (1 - x_j_k) - x_k_i - (1 - x_j_i)
    #                        = x_j_i - x_j_k - x_k_i
    return triangle_constraints

# @return a list of position constraints given the node list and edge list
# the position of a node = the number of nodes before it
def position_constraints():
    global _integer_variables
    position_constraints = []
    # p_i_j represents the position of node i in layer j
    relop = '='
    right = '0'
    for node_id in _node_dictionary:
        left = []
        layer = _node_dictionary[node_id]
        position_variable = "p_" + node_id + "_" + layer
        _integer_variables.append(position_variable)
        left.append("+" + position_variable)
        for other_node_id in _node_dictionary:
            other_layer = _node_dictionary[other_node_id]
            if layer == other_layer and node_id != other_node_id:
                left.append("-x_" + other_node_id + "_" + node_id)
        position_constraints.append((left, relop, right))

    return position_constraints

# @return a list of trivial constraints of the form "d_i_j_i_j = 0", one for
# each edge ij, so that each edge is guaranteed to be included when the
# solution is parsed by sol2sgf.py
def edges_for_output():
    global _binary_variables
    edges = []
    for edge in _edge_list:
        source = edge[0]
        target = edge[1]
        edge_variable = "d_" + source + "_" + target + "_"  + source + "_" + target
        edges.append((["+" + edge_variable], "=", "0"))
        _binary_variables.append(edge_variable)
    return edges

# @return a list of crossing constraints given node list and edge list the
# variable d_i_j_k_l = 1 iff edge ij crosses edge kl, where i,k are on one
# layer and j,l are on the next layer; a crossing occurs if
#      i precedes k and l precedes j
#   or k precedes i and j precedes l
def crossing_constraints():
    global _binary_variables
    global _crossing_variables
    _crossing_variables = []
    crossing_constraints = []
    relop = '>='
    right = '-1'
    # for every pair of edges edge_1 and edge_2, where the two edges are in
    # the same channel and do not have any common endpoints, generate the two
    # relevant crossing constraints
    for index_1, edge_1 in enumerate(_edge_list):
        source_1 = edge_1[0]
        target_1 = edge_1[1]
        channel_1 = _node_dictionary[target_1]
        for index_2, edge_2 in enumerate(_edge_list):
            source_2 = edge_2[0]
            target_2 = edge_2[1]
            channel_2 = _node_dictionary[target_2]
            # check if two edges in the same channel without common node
            if channel_1 == channel_2 \
                    and index_1 < index_2 \
                    and source_1 != source_2 and target_1 != target_2:
                crossing_variable = "d_" + source_1 + "_" + target_1 \
                    + "_" + source_2 + "_" + target_2
                _binary_variables.append(crossing_variable)
                _crossing_variables.append(crossing_variable)

                left = ["+" + crossing_variable]
                # wrong order in the first layer
                left.append("-x_" + str(source_2) + "_" + str(source_1))
                # but correct on second
                left.append("-x_" + str(target_1) + "_" + str(target_2))
                crossing_constraints.append((left, relop, right))

                left = ["+" + crossing_variable]
                # wrong order in the second layer
                left.append("-x_" + str(target_2) + "_" + str(target_1))
                # but correct on first
                left.append("-x_" + str(source_1) + "_" + str(source_2))
                crossing_constraints.append((left, relop, right))

    return crossing_constraints
    

# @return a list of bottleneck crossing constraints
# a bottleneck constraint has the form
#    bottleneck - sum of all crossing variables for a particular edge >= 0
def bottleneck_constraints():
    global _integer_variables
    _integer_variables.append("bottleneck")
    relop = '>='
    right = '0'
    bottleneck_constraints = []
    for index_1, edge_1 in enumerate(_edge_list):
        source_1 = edge_1[0]
        target_1 = edge_1[1]
        channel_1 = _node_dictionary[target_1]
        # compile the left hand side for edges crossing this one
        left = ["+bottleneck"]
        for index_2, edge_2 in enumerate(_edge_list):
            source_2 = edge_2[0]
            target_2 = edge_2[1]
            channel_2 = _node_dictionary[target_2]
            # check if two edges in the same channel without common node
            if channel_1 == channel_2 \
                    and source_1 != source_2 and target_1 != target_2:
                if index_1 < index_2:
                    crossing_variable = "d_" + str(source_1) + "_" + str(target_1) \
                        + "_" + str(source_2) + "_" + str(target_2)
                else:
                    crossing_variable = "d_" + str(source_2) + "_" + str(target_2) \
                        + "_" + str(source_1) + "_" + str(target_1)
                left.append("-" + crossing_variable)
        # add bottleneck constraint if it there's at least one potentially
        # crossing edge in the channel
        if len(left) > 1:
            bottleneck_constraints.append((left, relop, right))

    return bottleneck_constraints
    
# @return a single constraint that captures the fact that the total >= the
# sum of all crossing variables
def total_constraint():
    relop = '>='
    right = '0'
    left = ["+total"]
    for crossing_variable in _crossing_variables:
        left.append("-" + crossing_variable)
    return (left, relop, right)
    
# @return a list of stretch constraints, each constraint says, essentially,
# that the s_i_j = abs((1/|V_k|) * p_i_k - (1/|V_{k+1}) * p_j_{k+1}), where ij is an
# edge and k is the layer of node i
def stretch_constraints():
    global _continuous_variables
    global _stretch_variables
    _stretch_variables = []
    stretch_constraints = []

    # generate stretch constraints
    relop = '>='
    for edge in _edge_list:
        right = str(-TOLERANCE)
        source = edge[0]
        target = edge[1]
        stretch_variable = "s_" + source + "_" + target
        raw_variable = "z_" + source + "_" + target
        _continuous_variables.append(stretch_variable)
        _stretch_variables.append(stretch_variable)
        # standard tricks for absolute value
        # s_i_j >= z_i_j
        left = ["+" + stretch_variable]
        left.append("-" + raw_variable)
        stretch_constraints.append((left, relop, right))
        # s_i_j >= -z_i_j
        left = ["+" + stretch_variable]
        left.append("+" + raw_variable)
        stretch_constraints.append((left, relop, right))
        # introduce binary indicator variable: b_i_j = 0 if z_i_j is positive
        # and 1 if z_i_j is negative
        indicator_variable = "b_" + source + "_" + target
        _binary_variables.append(indicator_variable)
        # ensure s_i_j <= z_i_j if b_i_j = 0
        left = ["+" + raw_variable]
        left.append("+2 " + indicator_variable)
        left.append("-" + stretch_variable)
        stretch_constraints.append((left, relop, right))
        # ensure s_i_j <= -z_i_j if b_i_j = 1
        left = ["-" + raw_variable]
        left.append("-2 " + indicator_variable)
        left.append("-" + stretch_variable)
        right = str(-2 - TOLERANCE)
        stretch_constraints.append((left, relop, right))

    return stretch_constraints
    
# @return a list of constraints that give value to variables whose absolute
# values define stretch and whose squares are used in the quadratic
# objective. Each constraint says, essentially, that
# z_i_j = abs((1/|V_k|) * p_i_k - (1/|V_{k+1}) * p_j_{k+1}), where ij is an
# edge and k is the layer of node i; these are also used to get absolute
# value equality constraints for the regular stretch variables
def raw_stretch_constraints():
    global _raw_stretch_variables
    raw_constraints = []

    # generate raw stretch constraints
    relop = '='
    right = '0'
    for edge in _edge_list:
        source = edge[0]
        target = edge[1]
        raw_stretch_variable = "z_" + source + "_" + target
        _raw_stretch_variables.append(raw_stretch_variable)
        source_layer = int(_node_dictionary[source])
        target_layer = int(_node_dictionary[target])
        source_position_variable = "p_" + source + "_" + str(source_layer)
        target_position_variable = "p_" + target + "_" + str(target_layer)
        left = ["+" + raw_stretch_variable]
        left.append("+" + str(_layer_factor[source_layer])
                    + " " + source_position_variable)
        left.append("-" + str(_layer_factor[target_layer])
                    + " " + target_position_variable)
        raw_constraints.append((left, relop, right))
    return raw_constraints
    
# @return a single constraint for total stretch, i.e., stretch >= sum of
# stretch variables
def total_stretch_constraint():
    relop = '>='
    right = '0'
    _continuous_variables.append("stretch")
    left = ["+stretch"]
    for stretch_variable in _stretch_variables:
        left.append("-" + stretch_variable)
    return (left, relop, right)

# @return a list of bottleneck stretch constraints, i.e., bn_stretch is >=
# each stretch variable.
def bottleneck_stretch_constraints():
    global _continuous_variables
    relop = '>='
    right = '0'
    _continuous_variables.append("bn_stretch")
    bottleneck_stretch_constraints = []
    for stretch_variable in _stretch_variables:
        left = ["+bn_stretch", "-" + stretch_variable]
        bottleneck_stretch_constraints.append((left, relop, right))
    return bottleneck_stretch_constraints

# permutes the left hand side of each constraint as well as the order of the
# constraints in the list
def permute_constraints(constraints):
    random.shuffle(constraints)
    for constraint in constraints:
        random.shuffle(constraint[0])

# @return a string consisting of the elements of L separated by blanks and
# with a line break (and indentation) inserted between sublists of length
# max_length
def split_list(L, max_length):
    number_of_segments = int(math.ceil(len(L) / float(max_length)))
    output = ""
    for i in range(number_of_segments):
        segment = L[i * max_length:(i + 1) * max_length]
        output += ' '.join(segment)
        if i < number_of_segments - 1:
            output += '\n' + INDENT + INDENT
    return output

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M:%S"' )
    return date_pipe.readlines()[0].strip()

def print_header():
    print "\\ " + ' '.join(sys.argv)
    print "\\ " + date()

def print_comments():
    for comment in _comments:
        print "\\ " + comment

def print_quadratic_objective():
    quadratic_variables_squared = map(lambda x: "+" + x + "^2", _raw_stretch_variables)
    print INDENT + "[ " +  split_list(quadratic_variables_squared, MAX_TERMS_IN_LINE) + " ]/2"

def print_constraint(constraint):
    (left, relop, right) = constraint
    print INDENT + split_list(left, MAX_TERMS_IN_LINE), relop, right

def print_constraints(constraints):
    for constraint in constraints:
        print_constraint(constraint)

# need to make the lower bound on raw_stretch variables less than 0; -1 will work
def print_bounds_on_raw_stretch_variables():
    for variable in _raw_stretch_variables:
        print INDENT + "-1 <= " + variable + " <= 1"

def print_variables():
    print "Binary"
    print INDENT + split_list(list(_binary_variables), MAX_TERMS_IN_LINE)
    print "General"
    print INDENT + split_list(list(_integer_variables), MAX_TERMS_IN_LINE)
    if _continuous_variables != []:
        print "Semi"
        print INDENT + split_list(list(_continuous_variables), MAX_TERMS_IN_LINE)

def main():
    read_sgf(sys.stdin)
    constraints = triangle_constraints()
    # always need to print values of position variables to allow translation
    # back to an sgf file that captures the optimum order
    constraints.extend(position_constraints())
    # always need to print at least one variable for each edge so that the
    # sgf file gets the edges right; in this case it's a silly constraint
    # that looks like an edge crossing itself - that's what sol2sgf.py expects
    constraints.extend(edges_for_output())
    if args.objective == 'total' or args.objective == 'bottleneck' \
            or args.total != None or args.bottleneck != None:
        constraints.extend(crossing_constraints())
    if args.objective == 'stretch' or args.objective == 'bn_stretch' \
            or args.stretch != None or args.bn_stretch != None \
            or args.objective == 'quadratic':
        compute_layer_factors()
        constraints.extend(raw_stretch_constraints())
        raw_stretch_constraints_added = True
    if args.objective == 'stretch' or args.objective == 'bn_stretch' \
            or args.stretch != None or args.bn_stretch != None:
        constraints.extend(stretch_constraints())
    if args.objective == 'total' or args.total != None:
        constraints.append(total_constraint())
    if args.objective == 'bottleneck' or args.bottleneck != None:
        constraints.extend(bottleneck_constraints())
    if args.objective == 'stretch' or args.stretch != None:
        constraints.append(total_stretch_constraint())
    if args.objective == 'bn_stretch' or args.bn_stretch != None:
        constraints.extend(bottleneck_stretch_constraints())
    if args.objective == 'quadratic' and not raw_stretch_constraints_added:
        constraints.extend(raw_stretch_constraints())

    # add specific constraints for each objective if appropriate
    if args.total != None:
        constraints.append((["+total"], "<=", str(args.total)))
    if args.bottleneck != None:
        constraints.append((["+bottleneck"], "<=", str(args.bottleneck)))
    if args.stretch != None:
        constraints.append((["+stretch"], "<=", str(args.stretch)))
    if args.bn_stretch != None:
        constraints.append((["+bn_stretch"], "<=", str(args.bn_stretch)))

    if args.seed != None:
        random.seed(args.seed)
        permute_constraints(constraints)

    print_header()
    print_comments()

    print "Min";
    if args.objective == 'quadratic':
        print_quadratic_objective()
    else:
        print INDENT + args.objective
    print "st"
    print_constraints(constraints)
    if _raw_stretch_variables != []:
        print "Bounds"
        print_bounds_on_raw_stretch_variables()
    print_variables()
    print "End"

main()

#  [Last modified: 2016 06 07 at 19:19:00 GMT]
