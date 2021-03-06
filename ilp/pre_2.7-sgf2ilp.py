#! /usr/bin/env python

# Converts a graph in .sgf format to a linear integer program
# This program translate from standard input to standard output.

import sys
import os
#import argparse
import math
import random

MAX_TERMS_IN_LINE = 100
INDENT = "  "

def usage(prog_name):
    sys.stderr.write("Usage: " + prog_name + " TOTAL BOTTLENECK STRETCH BOTTLENECK_STRETCH [SEED]\n")
    sys.stderr.write(" the first four arguments specify bounds on each objective\n")
    sys.stderr.write(" a -1 indicates that there is no bound\n")
    sys.stderr.write(" a -2 indicates that the given objective is to be minimized\n")
    sys.stderr.write(" if present, SEED is used to randomize the order of the constraints\n")

# creates two global lists that describe the graph
# _node_list: each item is a tuple of the form:
# 	(id, layer, position)
# _edge_list: each item is a tuple of the form:
#       (source, target)
def read_sgf( input ):
    global _node_list
    global _edge_list
    global _comments
    _node_list = []
    _edge_list = []
    _comments = []
    line = read_nonblank( input )
    while ( line ):
        type = line.split()[0]
        if type == 'n':
            _node_list.append( process_node( line ) )
        elif type == 'e':
            _edge_list.append( process_edge( line ) )
        elif type == 'c':
            _comments.append(line[1:])
            
        line = read_nonblank( input )

def process_node( line ):
    line_fields = line.split()
    id = int(line_fields[1])
    layer = int(line_fields[2])
    position_in_layer = line_fields[3]
    return (id, layer, position_in_layer)

def process_edge( line ):
    line_fields = line.split()
    source = line_fields[1]
    target = line_fields[2]
    return (source, target)
	
# @return the first non-blank line in the input
def read_nonblank( input ):
    line = input.readline()
    while ( line and line.strip() == "" ):
        line = input.readline()
    return line

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

# @return a list of constraints on relative position variables x_i_j, where
# x_i_j is 1 if node i precedes node j on their common layer, 0 otherwise
def triangle_constraints():
    global _binary_variables
    triangle_constraints = []
    # anti-symmetry constraints
    for idx_i, i in enumerate(_node_list):
        for idx_j, j in enumerate(_node_list):
            if idx_i < idx_j and i[1] == j[1]: # two nodes on the same layer
                x_i_j = "x_" + str(i[0]) + "_" + str(j[0])
                x_j_i = "x_" + str(j[0]) + "_" + str(i[0])
                current_constraint = (["+" + x_i_j, "+" + x_j_i], '=', '1')
                triangle_constraints.append(current_constraint)
                _binary_variables.append(x_i_j)
                _binary_variables.append(x_j_i)

    # transitivity constraints
    # x_i_j and x_j_k implies x_i_k
    # or x_i_k - x_i_j - x_j_k >= -1
    for idx_i, i in enumerate(_node_list):
        for idx_j, j in enumerate(_node_list):
            for idx_k, k in enumerate(_node_list):
                if idx_i < idx_j and idx_j < idx_k and i[1] == j[1] == k[1]:
                    relop = '>='
                    right = '-1'

                    left = ["+x_" + str(i[0]) + "_" + str(k[0])]
                    left.append("-x_" + str(i[0]) + "_" + str(j[0]))
                    left.append("-x_" + str(j[0]) + "_" + str(k[0]))
                    triangle_constraints.append((left, relop, right))

                    left = ["+x_" + str(i[0]) + "_" + str(j[0])]
                    left.append("-x_" + str(i[0]) + "_" + str(k[0]))
                    left.append("-x_" + str(k[0]) + "_" + str(j[0]))
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
    for node in _node_list:
        left = []
        id = str(node[0])
        layer = str(node[1])
        position_variable = "p_" + id + "_" + layer
        _integer_variables.append(position_variable)
        left.append("+" + position_variable)
        for other_node in _node_list:
            other_id = str(other_node[0])
            other_layer = str(other_node[1])
            if layer == other_layer and id != other_id:
                left.append("-x_" + other_id + "_" + id)
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
        channel_1 = _node_list[int(target_1)][1]
        for index_2, edge_2 in enumerate(_edge_list):
            source_2 = edge_2[0]
            target_2 = edge_2[1]
            channel_2 = _node_list[int(target_2)][1]
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
        channel_1 = _node_list[int(target_1)][1]
        # compile the left hand side for edges crossing this one
        left = ["+bottleneck"]
        for index_2, edge_2 in enumerate(_edge_list):
            source_2 = edge_2[0]
            target_2 = edge_2[1]
            channel_2 = _node_list[int(target_2)][1]
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
    
# @return a list of stretch constraints
def stretch_constraints():
    global _continuous_variables
    global _stretch_variables
    _stretch_variables = []
    stretch_constraints = []

    # create a list for number of nodes in layers
    # the size is based on the number of layers
    num_layer = 1 + max(_node_list,key=lambda item:item[1])[1]
    nodes_in_layer = [0] * num_layer
    # record number of nodes in that layer
    for node in _node_list:
        nodes_in_layer[node[1]] += 1
 
    # generate stretch constraints
    relop = '>='
    right = '0'
    for edge in _edge_list:
        source = edge[0]
        target = edge[1]
        stretch_variable = "s_" + source + "_" + target
        _continuous_variables.append(stretch_variable)
        _stretch_variables.append(stretch_variable)
        source_layer = _node_list[int(source)][1]
        target_layer = _node_list[int(target)][1]
        source_position_variable = "p_" + source + "_" + str(source_layer)
        target_position_variable = "p_" + target + "_" + str(target_layer)
        source_layer_size = nodes_in_layer[source_layer]
        target_layer_size = nodes_in_layer[target_layer]
        if source_layer_size < 1:
            sys.exit("Error: layer " + source_layer + " has no nodes")
        if target_layer_size < 1:
            sys.exit("Error: layer " + target_layer + " has no nodes")
        source_denominator = source_layer_size - 1
        target_denominator = target_layer_size - 1
        # layers with only a single node should put that node in the center
        if source_denominator == 0:
            source_denominator = 2
        if target_denominator == 0:
            target_denominator = 2

        left = ["+" + stretch_variable]
        left.append("+" + str(1.0 / source_denominator)
                    + " " + source_position_variable)
        left.append("-" + str(1.0 / target_denominator)
                    + " " + target_position_variable)
        stretch_constraints.append((left, relop, right))

        left = ["+" + stretch_variable]
        left.append("-" + str(1.0 / source_denominator)
                    + " " + source_position_variable)
        left.append("+" + str(1.0 / target_denominator)
                    + " " + target_position_variable)
        stretch_constraints.append((left, relop, right))

    return stretch_constraints
    
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

def print_constraint(constraint):
    (left, relop, right) = constraint
    print INDENT + split_list(left, MAX_TERMS_IN_LINE), relop, right

def print_constraints(constraints):
    for constraint in constraints:
        print_constraint(constraint)

def print_variables():
    print "Binary"
    print INDENT + split_list(list(_binary_variables), MAX_TERMS_IN_LINE)
    print "General"
    print INDENT + split_list(list(_integer_variables), MAX_TERMS_IN_LINE)
    if _continuous_variables != []:
        print "Semi"
        print INDENT + split_list(list(_continuous_variables), MAX_TERMS_IN_LINE)

def main():
    if len(sys.argv) < 5 or len(sys.argv) > 6:
        usage(sys.argv[0])
        exit(1)
    total_constraint_arg = int(sys.argv[1])
    bottleneck_constraint_arg = int(sys.argv[2])
    stretch_constraint_arg = float(sys.argv[3])
    bottleneck_stretch_constraint_arg = float(sys.argv[4])
    seed = None
    if len(sys.argv) == 6:
        seed = int(sys.argv[5])

    # figure out which constraint corresponds to the objective to be
    # minimized (-2) and which constraints are unbounded (-1)
    total = bottleneck = stretch = bn_stretch = None
    objective = None
    if total_constraint_arg == -2:
        objective = 'total'
    elif total_constraint_arg >= 0:
        total = total_constraint_arg
    if bottleneck_constraint_arg == -2:
        if objective == None:
            objective = 'bottleneck'
        else:
            sys.stderr.write("Only one objective can be optimized:\n")
            sys.stderr.write(" you're trying to optimize both total"
                             + " and bottleneck crossings (two -2's)\n")
            usage(sys.argv[0])
            exit(1)
    elif bottleneck_constraint_arg >= 0:
        bottleneck = bottleneck_constraint_arg
    if stretch_constraint_arg == -2.0:
        if objective == None:
            objective = 'stretch'
        else:
            sys.stderr.write("Only one objective can be optimized:\n")
            sys.stderr.write(" you're trying to optimize stretch"
                             + " and something else (two -2's)\n")
            usage(sys.argv[0])
            exit(1)
    elif stretch_constraint_arg >= 0:
        stretch = stretch_constraint_arg
    if bottleneck_stretch_constraint_arg == -2.0:
        if objective == None:
            objective = 'bn_stretch'
        else:
            sys.stderr.write("Only one objective can be optimized:\n")
            sys.stderr.write(" you're trying to optimize bottleneck stretch"
                             + " and something else (two -2's)\n")
            usage(sys.argv[0])
            exit(1)
    elif bottleneck_stretch_constraint_arg >= 0:
        bn_stretch = bottleneck_stretch_constraint_arg

    if objective == None:
        sys.stderr.write("No objective specified (need at least one -2)\n")
        usage(sys.argv[0])
        exit(1)
              
    read_sgf(sys.stdin)
    constraints = triangle_constraints()
    # always need to print values of position variables to allow translation
    # back to an sgf file that captures the optimum order
    constraints.extend(position_constraints())
    # always need to print at least one variable for each edge so that the
    # sgf file gets the edges right; in this case it's a silly constraint
    # that looks like an edge crossing itself - that's what sol2sgf.py expects
    constraints.extend(edges_for_output())
    if objective == 'total' or objective == 'bottleneck' \
            or total != None or bottleneck != None:
        constraints.extend(crossing_constraints())
    if objective == 'stretch' or objective == 'bn_stretch' \
            or stretch != None or bn_stretch != None:
        constraints.extend(stretch_constraints())
    if objective == 'total' or total != None:
        constraints.append(total_constraint())
    if objective == 'bottleneck' or bottleneck != None:
        constraints.extend(bottleneck_constraints())
    if objective == 'stretch' or stretch != None:
        constraints.append(total_stretch_constraint())
    if objective == 'bn_stretch' or bn_stretch != None:
        constraints.extend(bottleneck_stretch_constraints())

    # add specific constraints for each objective if appropriate
    if total != None:
        constraints.append((["+total"], "<=", str(total)))
    if bottleneck != None:
        constraints.append((["+bottleneck"], "<=", str(bottleneck)))
    if stretch != None:
        constraints.append((["+stretch"], "<=", str(stretch)))
    if bn_stretch != None:
        constraints.append((["+bn_stretch"], "<=", str(bn_stretch)))

    if seed != None:
        random.seed(seed)
        permute_constraints(constraints)

    print_header()
    print_comments()

    print "Min"
    print INDENT + objective
    print "st"
    print_constraints(constraints)
    print_variables()
    print "End"

main()

#  [Last modified: 2016 05 13 at 01:50:52 GMT]
