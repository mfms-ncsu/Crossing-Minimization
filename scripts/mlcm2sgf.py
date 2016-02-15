#! /usr/bin/env python

## mlcm2sgf.py - translates from mlcm format, as documented at
#  http://www-lehre.informatik.uni-osnabrueck.de/theoinf/index/research/mlcm
# and described below to sgf format, also described below.
#
# $Id: mlcm2sgf.py 106 2015-04-13 17:01:05Z mfms $

# mlcm (multi-level crossing minimization) was used in the experiments
# descibed in M. Chimani, P. Hungerlaender, M. Juenger, P. Mutzel. An SDP
# Approach to Multi-level Crossing Minimization. ACM Journal of Experimental
# Algorithmics, Volume 17(3), Article 3.3, 2012. It goes as follows:
#
# <Number-of-Levels>
# <Number-of-Edges-Between-First-and-Second-Level>
# <Number-of-Edges-Between-Second-and-Third-Level>
# ...
# <Number-of-Nodes-on-First-Level>
# <Number-of-Nodes-on-Second-Level>
# ...
# <Index-of-Node-on-First-Layer> <Index-of-Node-on-Second-Level>
# <Index-of-Node-on-First-Layer> <Index-of-Node-on-Second-Level>
# ...
# <Index-of-Node-on-Second-Layer> <Index-of-Node-on-Third-Level>
# ...
#
# The node indices start at 0 on each level, i.e., each level has a node
# numbered 0, a node numbered 1, etc. Numbers determine the order, which is
# optimal in the files provided at the web site.

# sfg format is as follows:
#
#    c comment line 1
#    ...
#    c comment line k
#
#    t graph_name
#
#    n id_1 layer_1 position_1
#    n id_2 layer_2 position_2
#    ...
#    n id_n layer_n position_n
#
#    e source_1 target_1
#    ...
#    e source_m target_m

import sys
import os

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M"' )
    return date_pipe.readlines()[0].split()[0]

def version():
    return "$Id: mlcm2sgf.py 106 2015-04-13 17:01:05Z mfms $"

def usage( program_name ):
    print "Usage:", program_name, "INPUT_FILE > OUTPUT_FILE"
    print "Takes the mlcm file INPUT_FILE and converts to sgf,"
    print "which is printed on standard output."

# adds tuples for nodes with id's from start_id to start_id+number_of_nodes-1
# to the given layer within the node_list
def add_nodes_to_layer( node_list, layer, start_id, number_of_nodes ):
    position = 0
    for node_id in range( start_id, number_of_nodes ):
        node_list.append( (node_id, layer, position) )
        position = position + 1

# @return a tuple of the form
# ( NODES, EDGES )
# where NODES is a list of the form
# [ (id_1 layer_1 position_1) ... (id_n layer_n position_n) ]
# and EDGES is a list of the form
# [ (source_1 dest_1) ... (source_m dest_m) ]    
def read_mlcm( stream ):
    global _number_of_layers
    node_list = []
    _number_of_layers = int( stream.readline() )
    number_of_edges_from_layer = []

    # read number of edges for each layer
    for layer in range( _number_of_layers - 1 ):
        edges_from_layer = int( stream.readline() )
        number_of_edges_from_layer.append( edges_from_layer )

    # compile a list of nodes based on the number of nodes on each layer
    start_node_of_layer = [ 0 ]
    total_nodes = 0
    for layer in range( _number_of_layers - 1 ):
        nodes_on_layer = int( stream.readline() )
        add_nodes_to_layer( node_list, layer, total_nodes, total_nodes + nodes_on_layer )
        total_nodes = total_nodes + nodes_on_layer
        start_node_of_layer.append( total_nodes )
    nodes_on_last_layer = int( stream.readline() )
    add_nodes_to_layer( node_list, _number_of_layers - 1, total_nodes, total_nodes + nodes_on_last_layer )

    # read the edges using information about the number of edges from each layer
    edge_list = []
    for layer in range( _number_of_layers - 1 ):
        for edge_number in range( number_of_edges_from_layer[ layer ] ):
            edge_info = stream.readline().split()
            source_index = int( edge_info[0] )
            dest_index = int( edge_info[1] )
            source = start_node_of_layer[ layer ] + source_index
            destination = start_node_of_layer[ layer + 1 ] + dest_index
            edge_list.append( (source, destination ) )

    return ( node_list, edge_list )

def print_sgf( file_stream, graph, name ):
    node_list = graph[0]
    edge_list = graph[1]
    file_stream.write( "c generated %s\n" % date() )
    file_stream.write( "c %s\n" % version() )
    file_stream.write( "c %d nodes %d edges %d layers\n" % ( len(node_list), len(edge_list), _number_of_layers ) )
    file_stream.write( "t %s\n" % name )
    for node in node_list:
        file_stream.write( "n %d %d %d\n" % tuple( node ) )
    for edge in edge_list:
        file_stream.write( "e %d %d\n" % tuple( edge ) )

def main():
    if len( sys.argv ) != 2:
        usage( sys.argv[0] )
        sys.exit()
    input_file_name = sys.argv[1]
    graph_name = os.popen( 'basename ' + input_file_name + " .mlcm" ).read()
    input_stream = open( input_file_name, 'r' )
    internal_graph = read_mlcm( input_stream )
    print_sgf( sys.stdout, internal_graph, graph_name )

main()

#  [Last modified: 2015 04 07 at 00:04:19 GMT]
