#! /usr/bin/env python

# Converts a graph in .sgf format to a linear integer program
# This program translate from standard input to standard output.

import sys

MAX_TOKENS_IN_LINE = 100

def usage( program_name ):
    print "Usage:", program_name, " < INPUT_FILE > OUTPUT_FILE"
    print "Takes an sgf file from standard input and converts to lp."
    
# @return a tuple of the form (node_list, edge_list)
# node_list has the form:
# 		[id layer position]
# edge_list has the form:
#       [source target]
def read_sgf( input ):
    node_list = []
    edge_list = []
    line = read_nonblank( input )
    while ( line ):
        type = line.split()[0]
        if type == 'n':
            node_list.append( process_node( line ) )
        elif type == 'e':
            edge_list.append( process_edge( line ) )
            # otherwise error (ignore for now)
        line = read_nonblank( input )
    # sort node_list by id
    node_list = sorted(node_list,key=lambda x: x[0])
    return (node_list, edge_list)	

def process_node( line ):
    line_fields = line.split()
    id = int(line_fields[1])
    layer = line_fields[2]
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
	
# @return string represented constraints
# @return list of binary for nodes
# @return list of binary for edges
def constraints(node_list, edge_list):
    count = 1
    b_n = []
    b_e = []
    general = []
    constraint = "st\n"
    # anti-symmetry constraint
    for idx_i, i in enumerate(node_list):
        for idx_j, j in enumerate(node_list):
            if idx_i < idx_j and i[1] == j[1]: #two nodes on the same layer
                x_i_j = "x_" + str(i[0]) + "_" + str(j[0])
                x_j_i = "x_" + str(j[0]) + "_" + str(i[0])
                constraint += " c" + str(count) + ": +" + x_i_j + " +" + x_j_i + " = 1\n" 
                b_n.append(x_i_j)
                b_n.append(x_j_i)
                count += 1
    # transitivity constraint
    for idx_i, i in enumerate(node_list):
        for idx_j, j in enumerate(node_list):
            for idx_k, k in enumerate(node_list):
                if idx_i < idx_j and idx_j < idx_k and i[1] == j[1] == k[1]:
                    constraint += " c" + str(count) + ": -x_" + str(i[0]) + "_" + str(j[0]) + " -x_" + str(j[0]) + "_" + str(k[0]) + " +x_" + str(i[0]) + "_" + str(k[0]) + " >= -1\n"
                    count += 1
    # edge crosses if wrong order on the first or second layer
    for idx_i, i in enumerate(edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(node_list[a][1],node_list[b][1])
        for idx_j, j in enumerate(edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(node_list[c][1],node_list[d][1])
            if channel_i == channel_j and idx_i < idx_j and a != c and a != d and b != c and b != d:
                d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                b_e.append(d_i_j)
                # wrong order in the first layer
                constraint += " c" + str(count) + ": -x_" + str(c) + "_" + str(a) + " -x_" + str(b) + "_" + str(d) + " +" + d_i_j + " >= -1\n"
                count += 1
                # wrong order in the second layer
                constraint += " c" + str(count) + ": -x_" + str(a) + "_" + str(c) + " -x_" + str(d) + "_" + str(b) + " +" + d_i_j + " >= -1\n"
                count += 1
                
    # p_i represets the position of node i in its layer 
    for i in node_list:
        p = "p_" + str(i[0]) + "_" + i[1]
        p_i = " -" + p + " = 0\n"
        for j in node_list:
            if i[1] == j[1] and str(i[0]) != str(j[0]): # distinguish node in same layer
                p_i = " +x_" + str(j[0]) + "_" + str(i[0]) + p_i
        constraint += " c" + str(count) + ":" + p_i
        general.append(p)
        count += 1
    return (constraint, b_n, b_e, general)

def main():
    if len( sys.argv ) != 1:
        usage( sys.argv[0] )
        sys.exit()
    node_list, edge_list = read_sgf( sys.stdin )
    constraint, b_n, b_e, general = constraints(node_list, edge_list)
    output =  "Min\n obj:"
    tokens_in_line = 0
    for item in b_e:
        output += " +" + item
        tokens_in_line = tokens_in_line + 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0
    output += "\n" + constraint
    output += "Binary\n"
    tokens_in_line = 0
    for item in b_e:
        output += " " + item
        tokens_in_line = tokens_in_line + 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0 
    for item in b_n:
        output += " " + item
        tokens_in_line = tokens_in_line + 1 
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0 
    output += "\nGeneral\n"
    tokens_in_line = 0
    for item in general:
        output += " " + item
        tokens_in_line = tokens_in_line + 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0
    output += "\nEnd"
    print output
    
main()
