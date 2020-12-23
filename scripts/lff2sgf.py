#! /usr/bin/env python

"""
 Translates from (l)ayer (f)irst (f)ormat (for lack of a better name),
 used in recent papers, such as
 Martí, R., Campos, V., Hoff, A., Peiró, J.,
  "Heuristics for the min–max arc crossing problem in graphs."
  in Expert Systems with Applications 109, 100 – 113.
 and files downloaded from
 https://grafo.etsii.urjc.es/optsicom/mmacp/mmacpfiles/MinMaxGDPlib.zip

The lff format begins with a line of the form
  NUMBER_NODES NUMBER_OF_EDGES NUMBER_OF_LAYERS
The second line lists the number of nodes on each layer, starting with the "bottom"
(or "left" based on figures in the Martí et al. paper).
All edges are directed from earlier layers to later ones, not that this matters.

Each subsequent line has two integers, the endpoints of an edge.
Node numbers for these edges are implicit from the number of nodes on the layers.
For example, if the second line in the file is.
 2 3 4
then nodes 1 and 2 are on layer 1, nodes 3-5 on layer 2 and nodes 6-9 on layer 3. 

sgf format is described in, among others, mlcm2sgf.py
IMPORTANT: The minimization program in src assumes layers are numbered starting at 0.

Usage: lff2sgf INPUT_FILE > OUTPUT_FILE
"""

import sys
import os

def usage(program_name):
    print("Usage:", program_name, "INPUT_FILE > OUTPUT_FILE")
    print("Takes the lff file INPUT_FILE and converts to sgf,")
    print("which is printed on standard output.")

import sys
import os

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M"' )
    return date_pipe.readlines()[0].split()[0]

"""
@return a tuple of the form
 (NODES, EDGES)
 where NODES is a list of the form
 [(id_1 layer_1 position_1) ... (id_n layer_n position_n)]
 and EDGES is a list of the form
 [(source_1 dest_1) ... (source_m dest_m)]    
"""
def read_lff(stream):
    global _number_of_layers
    first_line = stream.readline()
    _number_of_layers = int(first_line.strip().split()[2])
    # note: don't need number of nodes and edges to accomplish the process
    second_line = stream.readline()
    second_line_list = second_line.strip().split()
    node_list = get_nodes_from_layers(second_line_list)
    edge_list = []
    while ( True ):
        edge_pair_as_strings = stream.readline().strip().split()
        if not edge_pair_as_strings:
            break
        source = int(edge_pair_as_strings[0])
        target = int(edge_pair_as_strings[1])
        edge_list.append((source, target))
    return (node_list, edge_list)

"""
@return a list of (id, layer, position) tuples
@param layer_list a list of strings, each represents the number of nodes on a layer
     nodes are numbered consecutively on each layer, starting with
     1 + the last number on previous layer
"""
def get_nodes_from_layers(layer_list):
    node_list = []
    node_id = 1
    layer_number = 0
    for string in layer_list:
        layer_size = int(string)
        position_number = 0
        for k in range(layer_size):
            node_list.append((node_id, layer_number, position_number))
            node_id += 1
            position_number += 1
        layer_number += 1
    return node_list


def write_sgf(file_stream, graph, name):
    node_list = graph[0]
    edge_list = graph[1]
    file_stream.write("c generated %s\n" % date())
    file_stream.write("c  by lff2sgf.py\n")
    file_stream.write("c %d nodes %d edges %d layers\n"
                      % (len(node_list), len(edge_list), _number_of_layers))
    file_stream.write( "t %s\n" % name )
    for node in node_list:
        file_stream.write("n %d %d %d\n" % tuple(node))
    for edge in edge_list:
        file_stream.write("e %d %d\n" % tuple(edge))

if __name__ == '__main__':
    if len(sys.argv) != 2:
        usage(sys.argv[0])
        sys.exit()
    input_file_name = sys.argv[1]
    graph_name = os.popen('basename ' + input_file_name + ' .lff').read().strip()
    input_stream = open(input_file_name, 'r')
    internal_graph = read_lff(input_stream)
    write_sgf(sys.stdout, internal_graph, graph_name)

#  [Last modified: 2020 12 22 at 17:43:07 GMT]
