#! /usr/bin/env python3

"""
Randomly permutes nodes and edges of an sgf file;
order in which nodes appear and their position in a layer are permuted independently 
"""

"""
Algorithm.

1. Read the input
    a. save a list of comments
    b. create a list of nodes, tuples of the form (id, layer, position); integers all
    c. create a list of edges, tuples of the form (source, target)
2. Permute using the seed
    a. permute the list of nodes
    b. create a dictionary that maps a layer to a list of node id's (integers)
        - nodes are added in order of appearance in the permuted list
    c. create a dictionary that maps each node to a (layer, position) tuple
        - position is determined by position in the list for the layer
    d. permute the list of nodes again to determine their order in the output
    e. permute the list of edges
3. Output the permuted sgf graph
"""

import sys
import os
import random

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M"' )
    return date_pipe.readlines()[0].split()[0]

def usage(program_name):
    sys.stderr.write("Usage: {} INPUT_FILE SEED > OUTPUT_FILE\n".format(program_name))
    sys.stderr.write("Takes an sgf file and a seed and outputs a permuted version of the graph.\n")
    sys.stderr.write("Output (also sgf) goes to standard output.\n")

def read_sgf(input):
    global _nodes, _edges, _name, _num_nodes, _num_edges, _num_layers
    _nodes = []
    _edges = []
    line = skip_comments(input)
    # the current line is assumed to be the tag line,
    # of the form 't NAME NODES EDGES LAYERS'
    (tag, _name, _num_nodes, _num_edges, _num_layers) = line.strip().split()
    line = read_nonblank(input)
    while ( line ):
        type = line.split()[0]
        if type == 'n':
            process_node(line)
        elif type == 'e':
            process_edge(line)
        line = read_nonblank(input)

def process_node(line):
    global _nodes
    line_fields = line.split()
    node_id = int(line_fields[1])
    layer_number = int(line_fields[2])
    position_in_layer = int(line_fields[3])
    _nodes.append((node_id, layer_number, position_in_layer))

def process_edge(line):
    global _edges
    line_fields = line.split()
    source = int(line_fields[1])
    target = int(line_fields[2])
    _edges.append((source, target))

"""
@return the first non-blank line in the input
"""
def read_nonblank(input):
    line = input.readline()
    while ( line and line.strip() == "" ):
        line = input.readline()
    return line

"""
reads and skips lines that begin with 'c' and collects them into the global
list of strings _comments, one element per comment line
@return the first line that is not a comment line
"""
def skip_comments(input):
    global _comments
    _comments = []
    line = read_nonblank(input)
    while ( line.split()[0] == 'c' ):
        _comments.append( line.strip().lstrip( "c" ) )
        line = read_nonblank( input )
    return line

"""
permutes nodes and edges randomly using the seed supplied on the command line;
see step 2 of the algorithm
"""
def permute_graph():
    global _nodes, _edges
    random.shuffle(_nodes)
    layer_map = add_nodes_to_layers()
    node_map = set_node_information(layer_map)
    random.shuffle(_nodes)
    random.shuffle(_edges)
    return node_map

"""
@return a map that maps a layer number to a list of nodes on the layer;
the order of nodes on each layer is based on order in the global _node_list;
see step 2(b) in the algorithm
"""
def add_nodes_to_layers():
    layer_map = {}
    for node in _nodes:
        node_id = node[0]
        layer_number = node[1]
        if not layer_number in layer_map:
            layer_map[layer_number] = []
        layer_map[layer_number].append(node_id)
    return layer_map

"""
@return a map that maps each node id to a (new) layer and position;
position is based on the order of appearance in the layer map;
see step 2(c)
"""
def set_node_information(layer_map):
    node_map = {}
    for layer_number in layer_map:
        node_list = layer_map[layer_number]
        position = 0
        for node in node_list:
            node_map[node] = (layer_number, position)
            position += 1
    return node_map

"""
write comments from the input graph plus a comment about how the permuted graph was generated
"""
def write_preamble(output_stream, seed):
    for comment in _comments:
        output_stream.write("c {}\n".format(comment))
    output_stream.write("c Permuted, scrambleSgf, seed = {}, date = {}\n"
                        .format(seed, date()))
    output_stream.write("t {} {} {} {}\n"
                        .format(_name, _num_nodes, _num_edges, _num_layers))

"""
write shuffled graph in sgf format to the output stream
@param node_map maps each node to a (new) layer and position;
 the position is independent of the order in which the node appears in _node_list 
@param seed used in a comment about how the output was generated
"""
def write_sgf(output_stream, node_map, seed):
    write_preamble(output_stream, seed)
    for node in _nodes:
        node_id = node[0]
        (layer, position) = node_map[node_id]
        output_stream.write("n {} {} {}\n".format(node_id, layer, position))
    for edge in _edges:
        output_stream.write("e {} {}\n".format(edge[0], edge[1]))

if __name__ == '__main__':
    if len( sys.argv ) != 3:
        usage(sys.argv[0])
        sys.exit()
    input_file_name = sys.argv[1]
    seed = int(sys.argv[2])
    input_stream = open(input_file_name, 'r')
    read_sgf(input_stream)
    random.seed(seed)
    node_map = permute_graph()
    write_sgf(sys.stdout, node_map, seed)

#  [Last modified: 2020 12 29 at 23:41:12 GMT]
