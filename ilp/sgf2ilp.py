#! /usr/bin/env python

# Converts a graph in .sgf format to a linear integer program
# This program translate from standard input to standard output.

import sys
import argparse

MAX_TOKENS_IN_LINE = 100

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
    
# @return a list of constraints on relative position variables x_i_j, where
# x_i_j is 1 if node i precedes node j on their common layer, 0 otherwise
def triangle_constraints():
    triangle_constraints = []
    # anti-symmetry constraints
    for idx_i, i in enumerate(_node_list):
        for idx_j, j in enumerate(_node_list):
            if idx_i < idx_j and i[1] == j[1]: # two nodes on the same layer
                x_i_j = "x_" + str(i[0]) + "_" + str(j[0])
                x_j_i = "x_" + str(j[0]) + "_" + str(i[0])
                current_constraint = (["+" + x_i_j, "+" + x_j_i], '=', '1')
                triangle_constraints.append(current_constraint)
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
                    # not clear how many we need
                    
                    triangle_constraints.append("-x_" + str(i[0]) + "_" + str(k[0]) + " -x_" + str(k[0]) + "_" + str(j[0]) + " +x_" + str(i[0]) + "_" + str(j[0]) + " >= -1")
                    triangle_constraints.append("-x_" + str(j[0]) + "_" + str(k[0]) + " -x_" + str(k[0]) + "_" + str(i[0]) + " +x_" + str(j[0]) + "_" + str(i[0]) + " >= -1")
                    triangle_constraints.append("-x_" + str(i[0]) + "_" + str(j[0]) + " -x_" + str(j[0]) + "_" + str(k[0]) + " +x_" + str(i[0]) + "_" + str(k[0]) + " >= -1")
                    triangle_constraints.append("-x_" + str(k[0]) + "_" + str(j[0]) + " -x_" + str(j[0]) + "_" + str(i[0]) + " +x_" + str(k[0]) + "_" + str(i[0]) + " >= -1")
                    triangle_constraints.append("-x_" + str(j[0]) + "_" + str(i[0]) + " -x_" + str(i[0]) + "_" + str(k[0]) + " +x_" + str(j[0]) + "_" + str(k[0]) + " >= -1")
                    triangle_constraints.append("-x_" + str(k[0]) + "_" + str(i[0]) + " -x_" + str(i[0]) + "_" + str(j[0]) + " +x_" + str(k[0]) + "_" + str(j[0]) + " >= -1")
    return triangle_constraints
    
# @return a list of crossing constraints given node list and edge list
def crossing_constraints(node_list, _edge_list):
    crossing_constraints = []
    # edge crosses if wrong order on the first or second layer
    for idx_i, i in enumerate(_edge_list):
        constraint_generated = False
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
                # wrong order in the first layer
                crossing_constraints.append("-x_" + str(c) + "_" + str(a) + " -x_" + str(b) + "_" + str(d) + " +" + d_i_j + " >= -1")
                # wrong order in the second layer
                crossing_constraints.append("-x_" + str(a) + "_" + str(c) + " -x_" + str(d) + "_" + str(b) + " +" + d_i_j + " >= -1")
                constraint_generated = True
        if not constraint_generated:
            crossing_constraints.append("d_" + str(a) + "_" + str(b) + "_" + str(a) + "_" + str(b) + " >= 0")
    return crossing_constraints
    
# @return a list of position constraints given node list and edge list
def position_constraints(_node_list, _edge_list):
    position_constraints = []
    # p_i represets the position of node i in its layer 
    for i in _node_list:
        p = "p_" + str(i[0]) + "_" + str(i[1])
        p_i = " -" + p + " = 0"
        for j in _node_list:
            if i[1] == j[1] and str(i[0]) != str(j[0]): # distinguish node in same layer
                p_i = " +x_" + str(j[0]) + "_" + str(i[0]) + p_i
        position_constraints.append( p_i )

    return position_constraints
    
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

main()

#  [Last modified: 2016 05 11 at 00:14:50 GMT]
