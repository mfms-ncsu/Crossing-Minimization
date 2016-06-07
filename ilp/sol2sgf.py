#! /usr/bin/env python

# Converts solution of a linear integer program to a graph in .sgf format
# This program translate from standard input to standard output.

import sys

_graph_name = ""

def usage( program_name ):
    print "Usage:", program_name, " < INPUT_FILE > OUTPUT_FILE"
    print "Takes an cplex solution from standard input and converts to graph in sgf."

# @return a tuple of the form (node_list, edge_list)
# node_list is a list of nodes, each a tuple (id, layer, position)
# edge_list is a list of edges, each a tuple (source, target)  
def read_sol(input):
    global _graph_name
    # first line of cplex_ilp output has input file name, drop the .lp to get
    # graph name
    _graph_name = input.readline().split()[1][:-3]
    solution = get_solution(input)
    node_list = []
    edge_list = []
    for line in solution:
        type = line[0]
        if type == 'p':
            # a position variable has the form p_i_j, where i is a node id, j
            # is its layer; the value is the position
            node = process_node(line)
            node_list.append(node)
        elif type == 'd':
            # edges are deduced from crossing variables of the form
            # d_i_j_k_l, where ij and kl are edges
            edge_a, edge_b = process_edge(line)
            if edge_a not in edge_list:
                edge_list.append(edge_a)
            if edge_b not in edge_list:
                edge_list.append(edge_b)
        # otherwise ignored
        
    return (node_list, edge_list)
    
# @ return a tuple of the form (i, L, P) for a line of the form
# p_i_L P
def process_node (line):
    list = line.replace('_', ' ').split()
    # now the line looks like p i L P
    id = list[1]
    layer = list[2]
    # make sure position is an integer (round if it is not)
    position_in_layer = float(list[3])
    if (int(position_in_layer) != position_in_layer):
        position_in_layer = int(position_in_layer + 0.5)
    else:
        position_in_layer = int(position_in_layer)
    return (id, layer, position_in_layer)
    
# @return a tuple of the form ((i,j), (k,l)) for a line of the form
# d_i_j_k_l x, where x can be 0 or 1
def process_edge ( line ):
    list = line.replace('_', ' ').split()
    i = list[1]
    j = list[2]
    k = list[3]
    l = list[4]
    edge_a = (i, j)
    edge_b = (k, l)
    return (edge_a, edge_b)

# @return lines between BeginSolution and EndSolution
def get_solution(input):
    solution = []
    # Skips text before the beginning of the interesting block:
    for line in input:
        if line.strip() == 'BeginSolution':
            break
    # Reads text until the end of the block:
    for line in input:  
        if line.strip() == 'EndSolution':
            break
        if line.strip() != "":
            solution.append(line) 
    return solution
    
def main():
    if len(sys.argv) != 1:
        usage(sys.argv[0])
        sys.exit()
    
    node_list, edge_list = read_sol(sys.stdin)
        
    print 't ', _graph_name 
    for n in node_list:
        print 'n ', n[0], n[1], n[2]
    for e in edge_list:
        print 'e ', e[0], e[1]

main()

#  [Last modified: 2016 06 07 at 18:47:43 GMT]
