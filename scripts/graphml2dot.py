#! /usr/bin/env python

"""
converts a simple dialect of graphml (the one currently produced by Galant)
 to the dot notation used for GraphViz.
See https://github.com/mfms-ncsu/galant for more information about
Galant (Graph Algorithm Animation Tool)
"""
# input comes from stdin and output goes to stdout so it can be used as a filter
#
# graphml2dot &lt; file.graphml &gt; file.dot and then open file.dot in GraphViz
# will use default layout for GraphViz.
#
# graphml2dot &lt; file.graphml | neato -Tpdf &gt; file.pdf
# to get the graph in pdf format using spring layout; programs other than
# neato for different layouts are described on the GraphViz home page.
#
# if positions should not change from those specified in the graphml file,
# use a -p flag; this will add the right attribute to the dot file; this will
# only work for the fdp and neato programs

# Notes to myself:

# everything has to be scaled appropriately: points are coordinates in
# inches from bottom left, need to experiment with font sizes, etc.

# need to use pin=true to guarantee points won't move as a result of
# the layout strategy

import sys;
import argparse;

PIXELS_PER_INCH = 100.0;
is_directed = False;

# @param input_lines a list of lines (strings); of the input graphml
# description with each line a description of a node or edge (it's assumed
# that each is on a single line)

# @return a list; the first element is either 'directed' or undirected' and
# each subsequent sublist takes the form
# [type [attr value] ... [attr value]]
# where type is either node or edge
def parse_graph( input_lines ):
    internal_representation = [];
    for i in range( 0, len(input_lines) ):
        current_line = input_lines[i].strip().strip('</>').split();
        if current_line[0] == 'graph':
            break;

    # current_line starts the graph description
    graph_type = current_line[1].split('=')[1].strip('"');
    internal_representation.append( graph_type );

    for j in range( i + 1, len(input_lines) ):
        current_line = input_lines[j].strip().strip('</>').split()
        if current_line[0] == 'node':
            internal_representation.append( parse_node(current_line) );
        elif current_line[0] == 'edge':
            internal_representation.append( parse_edge(current_line) );

    # parsing takes place here: uses parse_node and parse_edge
    return internal_representation;

# @param node_line a single input line for a node with the '<node' and the
# '/>' stripped away
# @return a list of lists of the form [[attr value] ... [attr value]]
def parse_node( node_line ):
    node_attributes = ['node'];
    # pretty simple, just collect the attributes into a list
    for i in range( 1, len( node_line )):
        attribute_pair = node_line[i].split('=');
        if len(attribute_pair) < 2:
            continue;
        attribute_pair[1] = attribute_pair[1].strip('"');
        if attribute_pair[1] != '':
            node_attributes.append( attribute_pair );
    return node_attributes;

# @param edge_line a single input line for an edge with the '<edge' and the
# '/>' stripped away
# @return a list of lists of the form [[attr value] ... [attr value]]
def parse_edge( edge_line ):
    # pretty simple, just collect the attributes into a list, except that
    # source and target should come first (for convenience)
    edge_attributes = ['edge'];
    for i in range( 1, len( edge_line )):
        attribute_pair = edge_line[i].split('=');
        if len(attribute_pair) < 2:
            continue;
        attribute_pair[1] = attribute_pair[1].strip('"');
        if attribute_pair[1] != '':
            edge_attributes.append( attribute_pair );
    return edge_attributes;

def read_input( istream ):
    input_lines = [];
    line = istream.readline();

# prints attributes of a node in dot format (unless the only attribute is id)
def print_node_attributes( node_item ):
    # assume if there's only one item other than 'node' it must be an id
    if len( node_item ) == 2:
        return;

    # need to print the id, assumed to be the first attribute, before the
    # remaining properties (output_string accumulates the line)
    id = node_item[1][1];
    output_string = "  " + id + " [";

    # properties (starting with the list item at index 2) are handled as
    # follows:
    #
    # - x and y (originally pixels) are saved and translated to inches
    #   before output (the result will be upside down); for now the points will
    #   be pinned, but a command line option will specify otherwise later
    #
    # - since dot does not handle both labels and weights. the former takes
    #   precedence if both are there (command-line option to be added later,
    #   perhaps)
    #
    # - other options have a direct correspondence, mostly
 
    has_pos = False;
    my_label = '';
    for index in range( 2, len( node_item ) ):
        prop_pair = node_item[ index ];
        prop = prop_pair[0];
        value = prop_pair[1];
        if prop == 'x': x_pos = int(value) / PIXELS_PER_INCH; has_pos = True;
        elif prop == 'y': y_pos = int(value) / PIXELS_PER_INCH; has_pos = True;
        elif prop == 'label': my_label = value;
        elif prop == 'weight':
            if value != '0' and my_label == '': my_label = value;
        else: output_string = output_string + " " + prop + ' = "' + value + '"';

    if has_pos:
        output_string = output_string + ' pos = "' + str(x_pos) + "," + str(y_pos) + '"';
    if my_label != '':
        output_string = output_string + ' label = "' + id + ':' + my_label + '"';

    output_string = output_string + " ];"
    print output_string;

def print_edge_attributes( attribute_list ):
#    global is_directed;

    # omit the word 'edge'
    my_label = '';
    output_string = '['
    for index in range( 1, len( attribute_list ) ):
        prop_pair = attribute_list[ index ];
        prop = prop_pair[0];
        value = prop_pair[1];
        if prop == 'source': source = value;
        elif prop == 'target': target = value;
        elif prop == 'label': my_label = value;
        elif prop == 'weight':
            if my_label == '' and value != '0': my_label = value;
        else: output_string = output_string + " " + prop + ' = "' + value + '"';

    output_string = output_string + ' label = "' + my_label + '"';
    output_string = output_string + " ];"
    connector = " -- ";
    if is_directed: connector = " -> ";
    output_string = "  " + source + connector + target + " " + output_string;
    print output_string;

def node_filter( sublist ):
    return sublist[0] == 'node';

def edge_filter( sublist ):
    return sublist[0] == 'edge';

def print_general_attributes():
    print "  node [ pin = true ]"; # puts nodes where specified by graphml file

# prints the internal representation in dot format, doing whatever
# translations are necessary
def output_dot( internal_representation ):
    global is_directed;

    if internal_representation[0] == 'directed':
        is_directed = True;
        print "digraph my_graph {"
    elif internal_representation[0] == 'undirected':
        is_directed = False;
        print "graph my_graph {";
    else:
        print "Error - unrecognized graph type: ", internal_representation[0];

    # insert general attributes here
    print_general_attributes();

    # indivudual node attributes
    node_items = filter( node_filter, internal_representation );
    map( print_node_attributes, node_items );

    # edge information
    edge_items = filter( edge_filter, internal_representation );
    map( print_edge_attributes, edge_items );

    print '}';
    return;

def main():
    
    istream = sys.stdin;
    input_lines = istream.readlines();
    internal_representation = parse_graph( input_lines );
    output_dot( internal_representation );

main();
    

#  [Last modified: 2020 12 21 at 16:34:22 GMT]
