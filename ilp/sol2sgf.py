#! /usr/bin/env python

# Converts solution of a linear integer program to a graph in .sgf format
# This program translate from standard input to standard output.

import sys

filename = ""

def usage( program_name ):
    print "Usage:", program_name, " < INPUT_FILE > OUTPUT_FILE"
    print "Takes an cplex solution from standard input and converts to graph in sgf."
  
def read_sol( input ):
    global filename
    filename = input.readline().split()[1][:-3]
    solution = strip_comments( input )
    node_list = []
    edge_list = []
    for line in solution:
        type = line[0]
        if type == 'p':
            node = process_node(line)
            if node not in node_list:
                node_list.append(node)
        elif type == 'd':
            edge_a, edge_b = process_edge(line)
            if edge_a not in edge_list:
                edge_list.append(edge_a)
            if edge_b not in edge_list:
                edge_list.append(edge_b)
        # otherwise ignored
        
    return (node_list, edge_list)
    
    
def process_node ( line ):
    list = line.replace('_', ' ').split()
    position_in_layer = list[3]
    id = list[1]
    layer = list[2]
    return (id, layer, position_in_layer)
    
def process_edge ( line ):
    list = line.replace('_', ' ').split()
    i = list[1]
    j = list[2]
    k = list[3]
    l = list[4]
    edge_a = (i, j)
    edge_b = (k, l)
    return (edge_a, edge_b)

# @return lines from BeginSolution to EndSolution
def strip_comments( input ):
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
    if len( sys.argv ) != 1:
        usage( sys.argv[0] )
        sys.exit()
    
    node_list, edge_list = read_sol(sys.stdin)
        
    print 't ', filename 
    for n in node_list:
        print 'n ', n[0], n[1], n[2]
    for e in edge_list:
        print 'e ', e[0], e[1]

main()