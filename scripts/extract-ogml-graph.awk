## extract-ogml-graph.awk - extracts graph and ordering information from Sugiyama
# output for Rome graphs.
# Used as a filter by ogml2sgf (input format given by documentation there).
# Output format has lines for nodes in the form:
#     n n_NUMBER X-COORD Y-COORD
# and edges of the form:
#     e source_node_number target_node_number
# at this point there may be cycles.
#
# $Id: extract-ogml-graph.awk 107 2015-04-20 19:00:18Z mfms $

BEGIN {
  first_node = 1;
  first_edge = 1;
}

/^node/ {
  if ( ! first_node ) {
    # print all the information about the current node
    printf "n %d %d %d\n", id, x, y;
  }
  first_node = 0;
  id = 0;
  x = 0;
  y = 0;
}

/^id/ {
  id = $2;
}

/^x/ {
  x = $2;
}

/^y/ {
  y = $2;
}

/^edge/ {
  if ( first_edge ) {
    # first print all the information about the last node
    printf "n %d %d %d\n", id, x, y;
  }
  else {
    # print information about the current edge
    printf "e %d %d\n", source, target;
  }
  first_edge = 0;
}

/^source/ {
  source = $2;
}

/^target/ {
  target = $2;
}

END {
  # print information about the last edge
  printf "e %d %d\n", source, target;
}

#  [Last modified: 2011 05 25 at 20:06:35 GMT]
