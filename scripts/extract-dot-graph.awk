## extract-dot-graph.awk - extracts edges from a dot file and converts to the
# 'e' lines in an sgf file
# Output format has lines for nodes in the form:
#     n node_number -1 -1
# The -1's indicate that neither the layer nor the node's position in it have
# been assigned.
# The edges have the form:
#     e source_node_number target_node_number
# at this point there may be cycles.
#
# each edge in the dot file looks like
#     source -> target;
# $Id: extract-dot-graph.awk 107 2015-04-20 19:00:18Z mfms $

BEGIN {
    number_of_edges = 0;
    number_of_nodes = 0;
}

# maintains the following lists (arrays)
#  node_list[string] is the numerical id of the node whose name is string
#  edge_list[index] is a string of the form "e source_id target_id"
/->/ {
    source = $1;
    # need to remove ; from target
    target_with_semicolon = $3;
    index_of_semicolon = index( target_with_semicolon, ";" );
    target = substr( target_with_semicolon, 1, index_of_semicolon - 1 )

    if ( node_list[source] == 0 )
        # source node has not been seen
        node_list[source] = ++number_of_nodes;
    source_id = node_list[source]
    if ( node_list[target] == 0 )
        # target node has not been seen
        node_list[target] = ++number_of_nodes;
    target_id = node_list[target]

    edge_list[++number_of_edges] = "e" " " source_id " " target_id
}

END {
    for ( i = 1; i <= number_of_nodes; i++ )
        printf "n %d -1 -1\n", i
    for ( i = 1; i <= number_of_edges; i++ )
        print edge_list[i]
}

#  [Last modified: 2011 06 16 at 18:43:54 GMT]
