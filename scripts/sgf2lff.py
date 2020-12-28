#! /usr/bin/env python3

"""
Translates from sgf format to lff format.
These formats are described in mlcm2sgf.py and lff2sgf.py, respectively.
"""

import sys

def usage(program_name):
    print("Usage:", program_name, "INPUT_FILE > OUTPUT_FILE")
    print("Takes the lff file INPUT_FILE and converts to sgf,")
    print("which is printed on standard output.")

"""
Algorithm
  1. Read the sgf file, ignoring comments and the 't' line and ...
      a. create a list nodes in each layer
      b. make a list of edges
  2. Assign node numbers layer by layer, consecutively from 1 to number of nodes
  3. Output the lff format
      a. number of nodes, edges, and layers are easy to deduce from the data structures 
      b. number of nodes in each layer can be deduced from length of _layer
      c. endpoints of edges are looked up in _node_number
"""
    
"""
dictionary mapping a layer number (as int) to a list of nodes (as strings) in it
"""
global _layer

"""
maximum layer number in sgf file
"""
global _max_layer_number

"""
dictionary mapping a node id (as string) from the sgf graph
to a node number in the lff format
"""
global _node_number

"""
list of edges; each item is of the form (source, target), where each is a string
"""
global _edges

def read_sgf(input):
    global _layer, _max_layer_number, _node_number, _edges
    _layer = {}
    _node_number = {}
    _max_layer_number = 0
    _edges = []
    line = skip_comments(input)
    # for now, assume next line is the one that begins with 't' and gives the graph name,
    # it can be ignored.
    line = read_nonblank( input )
    while (line):
        type = line.split()[0]
        if type == 'n':
            process_node(line)
        elif type == 'e':
            process_edge(line)
        line = read_nonblank( input )

def process_node(line):
    global _layer, _max_layer_number
    line_fields = line.split()
    node_id = line_fields[1]
    layer_number = int(line_fields[2])
    position_in_layer = line_fields[3]
    if not layer_number in _layer:
        _layer[layer_number] = []
    _layer[layer_number].append(node_id)
    if layer_number > _max_layer_number:
        _max_layer_number = layer_number

def process_edge(line):
    global _edges
    line_fields = line.split()
    source = line_fields[1]
    target = line_fields[2]
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
reads and skips lines that begin with 'c'
@return the first nonblank line that is not a comment
"""
def skip_comments(input):
    line = read_nonblank(input)
    while ( line.split()[0] == 'c' ):
        line = read_nonblank(input)
    return line

"""
maps each node id to a number (_node_number) so that
- node numbers are consecutive from 1 to number of nodes
- number of each node = # of nodes on previous layers + position of node on its layer
"""
def assign_node_numbers():
    global _node_number
    current_node_number = 1
    for layer_number in range(0, _max_layer_number + 1):
        for node_id in _layer[layer_number]:
            _node_number[node_id] = current_node_number
            current_node_number += 1

"""
writes the graph in lff format to the output_stream
"""
def write_lff(output_stream):
    num_nodes = len(_node_number)
    num_edges = len(_edges)
    num_layers = len(_layer)
    output_stream.write("{} {} {}\n".format(num_nodes, num_edges, num_layers))
    output_stream.write("{}".format(len(_layer[0])))
    for layer_num in range(1, _max_layer_number + 1):
        output_stream.write(" {}".format(len(_layer[layer_num])))
    output_stream.write("\n")
    for edge in _edges:
        source = _node_number[edge[0]]
        target = _node_number[edge[1]]
        output_stream.write("{} {}\n".format(source, target))

if __name__ == '__main__':
    if len(sys.argv) != 2:
        usage(sys.argv[0])
        sys.exit()
    input_file_name = sys.argv[1]
    input_stream = open(input_file_name, 'r')
    read_sgf(input_stream)
    assign_node_numbers()
    write_lff(sys.stdout)

#  [Last modified: 2020 12 28 at 20:21:53 GMT]
