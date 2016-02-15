## extract-graphml-graph.awk - extracts graph and ordering information from
# standard graphml formal (see graphml2sgf for description).
# Output format has lines for nodes in the form:
#     n n_NUMBER -1 -1
# The -1's indicate that neither the layer nor the node's position in it have
# been assigned.
# The edges have the form:
#     e source_node_number target_node_number
# at this point there may be cycles.
#
# @todo now it's possible to extract layer and position information for nodes
# as well; need a python script, probably, to do this right

# $Id: extract-graphml-graph.awk 71 2014-07-15 14:45:53Z mfms $

BEGIN {
    FS = "\""
}

/\<node/ {
    id = substr( $2, 2 );
    printf "n %d -1 -1\n", id;
}

/\<edge/ {
    source = substr( $4, 2 );
    target = substr( $6, 2 );
    printf "e %d %d\n", source, target;
}

#  [Last modified: 2014 06 12 at 19:48:15 GMT]
