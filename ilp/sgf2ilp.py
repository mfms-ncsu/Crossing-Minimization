#! /usr/bin/env python

# Converts a graph in .sgf format to a linear integer program
# This program translate from standard input to standard output.

import sys
import argparse
import math
from sets import Set

MAX_TERMS_IN_LINE = 100

parser = argparse.ArgumentParser('Creates an ILP to minimize some quantity based on an sgf representation of a layered graph')
parser.add_argument('--objective', choices={'total','bottleneck','stretch','bn_stretch'},
                    default='total',
                    required=True,
                    help='minimize ...'
                    + ' total/bottleneck (total/bottleneck crossings)'
                    + ' stretch/bn_stretch (total/bottleneck edge length)')
parser.add_argument('--total', type=int,
                    help='constraint on the total number of crossings (default: none)')
parser.add_argument('--bottleneck', type=int,
                    help='constraint on the maximum number of crossings of any edge (default: none)')
parser.add_argument('--stretch', type=int,
                    help='constraint on the total edge length (default: none)')
parser.add_argument('--bn_stretch', type=int,
                    help='constraint on the maximum length of any edge (default: none)')
parser.add_argument('--seed', type=int,
                    help='a random seed if ILP constraints are to be permuted (default: none)')

args = parser.parse_args()

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
# each string in left is a + or - followed by a variable name.

# variables are added to these sets when they arise in constraints
global _binary_variables = Set([])
global _integer_variables = Set([])
global _continuous_variables = Set([])

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
                _binary_variables.add(x_i_j)
                _binary_variables.add(x_j_i)

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
        _integer_variables.add(position_variable)
        left.append("+" + position_variable)
        for other_node in _node_list:
            other_id = str(other_node[0])
            other_layer = str(other_node[1])
            if layer == other_layer and id != other_id:
                left.append("-x_" + other_id + "_" id)
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
        _binary_variables.add(edge_variable)
    return edges
    
# @return a list of crossing constraints given node list and edge list the
# variable d_i_j_k_l = 1 iff edge ij crosses edge kl, where i,k are on one
# layer and j,l are on the next layer; a crossing occurs if
#      i precedes k and l precedes j
#   or k precedes i and j precedes l
def crossing_constraints():
    global _binary_variables
    crossing_constraints = []
    relop = '>='
    right = '-1'
    # for every pair of edges edge_1 and edge_2, where the two edges are in
    # the same channel and do not have any common endpoints, generate the two
    # relevant crossing constraints
    for index_1, edge_1 in enumerate(_edge_list):
        source_1 = edge_1[0]
        target_1 = edge_1[1]
        # actually, it should not be necessary to consider the max of two
        # numbers here; the channel should be the layer number of the target
        channel_1 = max(_node_list[source_1][1],_node_list[target_1][1])
        for index_2, edge_2 in enumerate(_edge_list):
            source_2 = edge_2[0]
            target_2 = edge_2[1]
            # check if two edges in the same channel without common node
            channel_2 = max(_node_list[source_2][1],_node_list[target_2][1])
            # being paranoid about layers, sources and targets agaon
            if channel_1 == channel_2 \
                    and index_1 < index_2 \
                    and source_1 != source_2 and target_1 != target_2 \
                    and source_1 != target_2 and target_1 != source_2:
                crossing_variable = "d_" + source_1 + "_" + target_1 \
                    + "_" + source_2 + "_" + target_2
                _binary_variables.add(crossing_variable)

                left = ["+" + crossing_variable]
                # wrong order in the first layer
                left.append("-x_" + str(source_2) + "_" + str(source_1))
                # but correct on second
                left.append("-x_" + str(target_1) + "_" + str(target_2))
                crossing_constraints.append("-x_" + str(c) + "_" + str(a) + " -x_" + str(b) + "_" + str(d) + " +" + d_i_j + " >= -1")
                # wrong order in the second layer
                crossing_constraints.append("-x_" + str(a) + "_" + str(c) + " -x_" + str(d) + "_" + str(b) + " +" + d_i_j + " >= -1")
                constraint_generated = True
        if not constraint_generated:
            crossing_constraints.append("d_" + str(a) + "_" + str(b) + "_" + str(a) + "_" + str(b) + " >= 0")
    return crossing_constraints
    
# @return a list of bottleneck constraints given node list and edge list
def bottleneck_constraints(_node_list, _edge_list):
    bottleneck_constraints = []
    for idx_i, i in enumerate(_edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(_node_list[a][1],_node_list[b][1])
        single_edge_crossing = ""
        for idx_j, j in enumerate(_edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(_node_list[c][1],_node_list[d][1])
            if channel_i == channel_j and a != c and a != d and b != c and b != d:
                if idx_i < idx_j:
                    d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                else:
                    d_i_j = "d_" + str(c) + "_" + str(d) + "_" + str(a) + "_" + str(b)
                # sum of crossing for each edge
                single_edge_crossing += " +" + d_i_j
        # bottleneck constraint
        single_edge_crossing += " -b <= 0"
        # ignore standalone edges
        if single_edge_crossing != " -b <= 0":
            bottleneck_constraints.append(single_edge_crossing)
        single_edge_crossing = ""
    return bottleneck_constraints
    
# @return total constraints given node list and edge list
def total_constraints(_node_list, _edge_list):
    tokens_in_line = 0
    total_constraints = ""
    for idx_i, i in enumerate(_edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(_node_list[a][1],_node_list[b][1])
        for idx_j, j in enumerate(_edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(_node_list[c][1],_node_list[d][1])
            if channel_i == channel_j and idx_i < idx_j and a != c and a != d and b != c and b != d:
                d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                total_constraints += " +" + d_i_j
                tokens_in_line += 1
                if tokens_in_line > MAX_TOKENS_IN_LINE:
                    total_constraints += "\n"
                    tokens_in_line = 0
    total_constraints += " -t <= 0"
    return total_constraints
    
# @retun a list of stretch constraints given node list and edge list
def stretch_constraints(_node_list, _edge_list):
    stretch_constraints = []

    # create a list for number of nodes in layers
    # the size is based on the number of layers
    num_layer = 1 + max(_node_list,key=lambda item:item[1])[1]
    nodes_in_layer = [0] * num_layer
    # record number of nodes in that layer
    for node in _node_list:
        nodes_in_layer[node[1]] += 1
 
    # generate stretch constraint
    for edge in _edge_list:
        i = edge[0]
        j = edge[1]
        s_i_j = "s_" + i + "_" + j
        i_layer = _node_list[int(i)][1]
        j_layer = _node_list[int(j)][1]
        p_i = "p_" + i + "_" + str(i_layer)
        p_j = "p_" + j + "_" + str(j_layer)
        L_i = nodes_in_layer[i_layer]
        L_j = nodes_in_layer[j_layer]
        if L_i < 1:
            sys.exit("Error: layer " + i_layer + " has no nodes")
        if L_j < 1:
            sys.exit("Error: layer " + j_layer + " has no nodes")
        deno_i = L_i - 1
        deno_j = L_j - 1
        if L_i == 1:
            deno_i = 2
        if L_j == 1:
            deno_j = 2
        dec_i = 1.0/deno_i
        dec_j = 1.0/deno_j
        stretch_constraints.append(s_i_j + " +" + str(dec_i) + " " + p_i + " -" + str(dec_j) + " " + p_j + " >= 0" )
        stretch_constraints.append(s_i_j + " -" + str(dec_i) + " " + p_i + " +" + str(dec_j) + " " + p_j + " >= 0" )
    
    # total stretch
    tokens_in_line = 0
    total_stretch = ""
    for edge in _edge_list:
        i = edge[0]
        j = edge[1]
        s_i_j = "s_" + i + "_" + j
        total_stretch += " +" + s_i_j
        tokens_in_line += 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            total_stretch += "\n"
            tokens_in_line = 0
    total_stretch += " -s <= 0"
    stretch_constraints.append(total_stretch)
    
    return stretch_constraints
    
# @return a list of binary variables, a list of general variables and a list
# of semi-continuous variables
def variables():
    binary = []
    general = []
    semi = []
    # binary node order variables
    for idx_i, i in enumerate(_node_list):
        for idx_j, j in enumerate(_node_list):
            if idx_i < idx_j and i[1] == j[1]: #two nodes on the same layer
                x_i_j = "x_" + str(i[0]) + "_" + str(j[0])
                x_j_i = "x_" + str(j[0]) + "_" + str(i[0])
                binary.append(x_i_j)
                binary.append(x_j_i)
    # binary edge crossing variables
    for idx_i, i in enumerate(_edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(_node_list[a][1],_node_list[b][1])
        for idx_j, j in enumerate(_edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(_node_list[c][1],_node_list[d][1])
            if channel_i == channel_j and idx_i < idx_j and a != c and a != d and b != c and b != d:
                d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                binary.append(d_i_j)
    # general variables
    for i in _node_list:
        p = "p_" + str(i[0]) + "_" + str(i[1])
        general.append(p)
    general.append("t")
    general.append("b")
    
    # semi- continus variables
    for i in _edge_list:
        s = "s_" + i[0] + "_" + i[1]
        semi.append(s)
    semi.append("s")
    return binary, general,semi

# @return a string consisting of the elements of L separated by blanks and
# with a line break inserted between sublists of length max_length
def split_list(L, max_length):
    number_of_segments = int(math.ceil(len(L) / float(max_list_length)))
    output = ""
    for i in range(number_of_segments):
    segment = L[i * max_list_length:(i + 1) * max_list_length]
    output += ' '.join(segment)
    if i < number_of_segments - 1:
        output += '\n'
    return output

def main():
    read_sgf( sys.stdin )
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

    if args.seed != None:
        constraints = permute_constraints(constraints)

    print_header()
    print_comments()

    print "Min"
    print "   ", args.objective
    print "st"
    print_constraints(constraints)
    print_variables(variables())


    
    # prepare output
    output = "Min\n obj: "  + min
    output += "\nst\n"
    for item in constraints:
        output += " c" + str(count) + ": " + item + "\n"
        count += 1
    output += "Binary\n"
    tokens_in_line = 0 
    for item in binary:
        output += " " + item
        tokens_in_line += 1 
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0 
    output += "\nGeneral\n"
    tokens_in_line = 0
    for item in general:
        output += " " + item
        tokens_in_line += 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0
    output += "\nSemi\n"
    tokens_in_line = 0
    for item in semi:
        output += " " + item
        tokens_in_line += 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0    
    output += "\nEnd"
    return output
    
main()

#  [Last modified: 2016 05 11 at 20:24:39 GMT]
