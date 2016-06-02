#! /usr/bin/env python

## channelDegrees.py - takes an sgf file and prints statistics about degrees
## of nodes relative to the upper and lower layers of each channel

import sys
import math

def main():
    if len(sys.argv) != 2:
        print "Usage: channelDegrees.py FILE"
        exit(1)

    input_stream = open(sys.argv[1])
    read_sgf(input_stream)
    print_statistics()

# creates the global data structures _node_dictionary and _nodes_on_layer,
# where
# in _node_dictionary each entry has key = a node id and value = a list
#        [layer, up_degree, down_degree]
# and _nodes_on_layer[i] is a list of (ids of) nodes on layer i

_node_dictionary = {}
_nodes_on_layer = []
 
def read_sgf(input):
    global _node_dictionary
    global _nodes_on_layer
    line = skip_comments(input)
    # for now, assume next line begins with 't'; do error checking later
    # since the number of nodes and edges is implicit, these can be ignored
    name = line.split()[1]
    graph = (name, [], [])
    line = read_nonblank(input)
    while (line):
        type = line.split()[0]
        if type == 'n':
            process_node(line)
        elif type == 'e':
            process_edge(line)
            # otherwise error (ignore for now)
        line = read_nonblank( input )
    _nodes_on_layer = layer_lists()
    return graph

# reads and skips lines that begin with 'c'
# @return the first line that is not a comment line
def skip_comments( input ):
    line = read_nonblank( input )
    while ( line.split()[0] == 'c' ):
        line = read_nonblank( input )
    return line

# @return the next non-blank line in the input
def read_nonblank(input):
    line = input.readline()
    while ( line and line.strip() == "" ):
        line = input.readline()
    return line

# creates a dictionary entry corresponding to the node information on the line
def process_node(line):
    line_fields = line.split()
    id = line_fields[1]
    layer = int(line_fields[2])
    _node_dictionary[id] = [layer, 0, 0]

# updates up- and down-degrees for the endpoints of the edge described on the line
def process_edge(line):
    line_fields = line.split()
    source = line_fields[1]
    target = line_fields[2]
    _node_dictionary[source][1] += 1
    _node_dictionary[target][2] += 1

# @return a list whose i-th entry is a list of nodes on layer i
def layer_lists():
    # first compute the maximum layer number
    max_layer = 0
    for node in _node_dictionary:
        layer = _node_dictionary[node][0]
        if layer > max_layer:
            max_layer = layer
    # since layers are numbered starting with 0, the number of layers is
    # actually max_layer + 1; also, need to be careful that there are
    # multiple copies of the empty list in 'layers'
    layers = []
    for layer in range(max_layer + 1):
        layers.append([])
    for node in _node_dictionary:
        layer = _node_dictionary[node][0]
        layers[layer].append(node)
    return layers

# -------- here ends the reading of the graph --------------- #

# ----------------------------------------------------------- #

# prints information that might be interesting with respect to our conjecture
# about crossings versus stretch:
#     - degree discrepancy in each channel (and min, median, mean, max degree)
#     - degree discrepancy for upper and lower layers separately
#     - number of edges, density, volume of each channel
#       where
#         density = number of edges / product of number of nodes on the layers
#         volume = product of the degree discrepancies of the two layers
# format for each channel is
#     allDegrees,channel#,min,med,mean,max,stdev,#nodes_on_channel
#     upperDegrees,channel#,min,med,mean,max,stdev,#nodes_on_channel
#     lowerDegrees,channel#,min,med,mean,max,stdev,#nodes_on_channel
#     channelStats,channel,#upper_nodes,#lower_nodes,#edges,density,layer_ratio,total_density,volume
def print_statistics():
    print "degreeStats,%2s,%3s,%4s,%4s,%4s,%3s,%4s,%s" % \
        ("Ch", "Min", "Med", "Avg", "Max",
         "Dev", "#", "Dis")
    print "nodesInChannel,%s,%s,%s,%s,%s,%s,%s,%s" % \
        ("Ch", "upper", "lower", "edges", "dens", "ratio", "+dis", "*dis")
    print "scaledVolumes,%s,%s,%s,%s,%s,%s,%s" % \
        ("min", "median", "mean", "max", "stdev", "#ch", "disc")
    print "---------------------------------"
    number_of_layers = len(_nodes_on_layer)
    volume_list = []
    for channel in range(1, number_of_layers):
        volume = print_channel_statistics(channel)
        volume_list.append(volume)
    scaled_volume_list = map(lambda x: float(x) / number_of_layers, volume_list)
    print_basic_statistics('allVolumes', scaled_volume_list)

# prints all statistics for the current channel
# @return the volume of the current channel
def print_channel_statistics(channel):
    number_of_layers = len(_nodes_on_layer)
    upper_layer = channel
    lower_layer = channel - 1
    # get the up degrees of nodes on the lower layer
    lower_degrees = map(lambda node: _node_dictionary[node][1],
                        _nodes_on_layer[lower_layer])
    # get the down degrees of nodes on the upper layer
    upper_degrees = map(lambda node: _node_dictionary[node][2],
                        _nodes_on_layer[upper_layer])
    all_degrees = list(upper_degrees)
    all_degrees.extend(lower_degrees)
    print_basic_statistics('allDegrees,' + str(channel), all_degrees)
    print_basic_statistics('lowerDegrees,' + str(channel), lower_degrees)
    print_basic_statistics('upperDegrees,' + str(channel), upper_degrees)
    number_of_edges = reduce(lambda x,y: x+y, upper_degrees)
    number_of_upper_nodes = len(upper_degrees)
    number_of_lower_nodes = len(lower_degrees)
    density = float(number_of_edges) / \
        (float(number_of_upper_nodes) * float(number_of_lower_nodes))
    layer_ratio = max(float(number_of_upper_nodes) / number_of_lower_nodes,
                      float(number_of_lower_nodes) / number_of_upper_nodes)
    upper_discrepancy = discrepancy(upper_degrees)
    lower_discrepancy = discrepancy(lower_degrees)
    total_discrepancy = upper_discrepancy + lower_discrepancy
    volume = upper_discrepancy * lower_discrepancy
    print "channelStats,%d,%d,%d,%d,%4.3f,%4.2f,%4.2f,%5.2f" % \
        (channel, number_of_upper_nodes, number_of_lower_nodes, number_of_edges,
         density, layer_ratio, total_discrepancy, volume)
    return volume

# takes a list of numbers and prints 
#   label, min, median, mean, max, stdev, length, discrepancy
#    where discrepancy = max / median
# for puposes of discrepancy, the median is the lower median
# the list must be nonempty
def print_basic_statistics(label, list):
    total = reduce(lambda x,y: x+y, list)
    squares = map(lambda x: x*x, list)
    sum_of_squares = reduce(lambda x,y: x+y, squares)
    length = len(list)
    mean = float(total) / length
    sorted_list = sorted(list)
    if length % 2 == 1:
        median = sorted_list[length / 2]
    else:
        median = float(sorted_list[length / 2 - 1] + sorted_list[length / 2]) \
            / 2.0
    stdev = math.sqrt( float(sum_of_squares) / length - mean * mean )
    print "%s,%3.1f,%3.1f,%4.2f,%4.2f,%4.2f,%d,%4.2f" % \
        (label, min(list), median, mean, max(list), stdev,
         length, discrepancy(list))

# @return max / median of the list (in this case is always the lower value
# for an even length list)
def discrepancy(list):
    working_list = filter(lambda x: x != 0, list)
    working_list = sorted(working_list)
    median = working_list[(len(working_list) - 1)/ 2]
    return float(max(working_list)) / median

main()



#  [Last modified: 2016 06 02 at 14:46:07 GMT]
