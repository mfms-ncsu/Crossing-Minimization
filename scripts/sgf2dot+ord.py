#! /usr/bin/env python

"""
translates from sgf format, described in the script, into a .dot file
and an .ord file, whose formats are described with the sbg_software at
http://people.engr.ncsu.edu/mfms/Software/SBG_Software/formats.html
"""

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

MAX_NODES_PER_LINE = 15

def usage(program_name):
    print "Usage: " + program_name + " basename"
    print "  where basename.sgf is the desired input file"
    print "  produces basename.dot and basename.ord in the same directory as basename.sgf"

# @return a tuple of the form (name, layers, edges), where name is the name
# of the graph, layers is a list of lists, each inner list containing pairs
# of the form (p,n), where p is a position index and n is the node in that
# position, and edges is a list of pairs of the form (n_1,n_2), each denoting
# an edge.
def read_sgf( input ):
    line = skip_comments( input )
    # for now, assume next line begins with 't'; do error checking later
    # since the number of nodes and edges is implicit, these can be ignored
    name = line.split()[1]
    graph = (name, [], [])
    line = read_nonblank( input )
    while ( line ):
        type = line.split()[0]
        if type == 'n':
            add_node(line, graph[1])
        elif type == 'e':
            add_edge(line, graph[2])
            # otherwise error (ignore for now)
        line = read_nonblank( input )
    return graph

# adds the node descibed on the sgf input line to the list for its layer
def add_node(line, layers):
    line_fields = line.split()
    id = line_fields[1]
    layer = int(line_fields[2])
    position_in_layer = int(line_fields[3])
    # ensure there are enough layers
    for i in range(len(layers), layer+1):
        layers.append([])
    layers[layer].append((position_in_layer, id))

# adds the edge described on the sgf input line to the list of edges
def add_edge(line, edges):
    line_fields = line.split()
    source = line_fields[1]
    target = line_fields[2]
    edges.append((source, target))

# @return the first non-blank line in the input
def read_nonblank( input ):
    line = input.readline()
    while ( line and line.strip() == "" ):
        line = input.readline()
    return line

# reads and skips lines that begin with 'c' and collects them into the global
# list of strings _comments, one element per comment line
# @return the first line that is not a comment line
def skip_comments( input ):
    global _comments
    _comments = []
    line = read_nonblank( input )
    while ( line.split()[0] == 'c' ):
        _comments.append( line.strip().lstrip( "c" ) )
        line = read_nonblank( input )
    return line

# writes a dot format version of the graph on the stream
def write_dot(stream, base_name, graph):
    name = graph[0]
    edges = graph[2]
    write_dot_comments(stream, base_name)
    write_dot_heading(stream, name)
    write_edges(stream, edges)
    write_dot_end(stream)


def write_dot_comments(stream, base_name):
    stream.write('/*\n * generated from %s by sgf2dot+ord.py, %s\n' % (base_name + '.sgf', date()))
    for comment in _comments:
        stream.write(' * ' + comment + '\n')
    stream.write(' */\n')

def write_dot_heading(stream, name):
    stream.write('digraph ' + name + ' {\n')

def write_edges(stream, edges):
    for edge in edges:
        write_edge(stream, edge)

def write_edge(stream, edge):
    stream.write(' n_%s -> n_%s;\n' % edge)

def write_dot_end(stream):
    stream.write('}\n')

def write_ord(stream, base_name, graph):
    name = graph[0]
    layers = graph[1]
    write_ord_comments(stream, base_name, name)
    write_layer_information(stream, layers)

def write_ord_comments(stream, base_name, name):
    stream.write('## generated from %s by sgf2dot+ord.py, %s\n' % ((base_name + '.sgf', date())))
    for comment in _comments:
        stream.write('# ' + comment + '\n\n')
    stream.write('# Graph name: ' + name + '\n')

def write_layer_information(stream, layers):
    for i in range(len(layers)):
        stream.write('\n# Order for layer %d based on sgf file.\n %d { ' % (i,i))
        write_layer(stream, layers[i])
        stream.write('} # end of layer %d\n' % i)

def get_key(item):
    return item[0]

def write_layer(stream, layer):
    # sort the layer using position, the first item in each tuple, as key
    layer.sort(key=get_key)
    for i in range(len(layer)):
        stream.write('n_%s ' % layer[i][1])
        if i < len(layer) - 1 and (i+1) % MAX_NODES_PER_LINE == 0:
            stream.write('\n')

def date():
    date_pipe = os.popen( 'date -u "+%Y/%m/%d %H:%M"' )
    return date_pipe.readlines()[0].split()[0]

def main():
    if ( len(sys.argv) != 2 ):
        usage( sys.argv[0] )
        sys.exit()
    base_name = sys.argv[1]
    input_stream = open(base_name + '.sgf', 'r')
    graph = read_sgf(input_stream)
    dot_stream = open(base_name + '.dot', 'w')
    write_dot(dot_stream, base_name, graph)
    dot_stream.close()
    ord_stream = open(base_name + '.ord', 'w')
    write_ord(ord_stream, base_name, graph)
    ord_stream.close()
    print "Files %s.dot and %s.ord have been created" % (base_name, base_name)

main()

#  [Last modified: 2020 12 21 at 17:01:56 GMT]
