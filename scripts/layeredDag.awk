## layeredDag.awk
#
# Generates a random dag with pre-specified number of nodes per layer.
#
# Usage: awk -f layeredDags.awk input_file > output_file.dot
#
# FORMAT of input file:
#     graph_name edge_prob random_seed
#     layer_zero_num_nodes
#     layer_one_num_nodes
#     ...
#     layer_(k-1)_num_nodes
#
# ---
# edge_prob determines the probability that an edge between nodes of two
# adjacent layers is chosen; random_seed is there so that the same dag can be
# recreated if the seed is known
#
# output_file.dot is in dot format with node names of the form
#   n_x_y
# where x is the layer and y is the position within the layer.
#
# Since a .ord file does not depend on random choices of edges, it can be
# generated independently using layeredDagsOrd.awk.
#
# @author Matt Stallmann
# @date 2008/07/10
#
# $Id: layeredDag.awk 107 2015-04-20 19:00:18Z mfms $

NR == 1 { # first-line information => print header
  name = $1; edge_probability = $2; bias = $3; random_seed = $4;
  srand( random_seed );
  printf "/* Created by layeredDag.awk:";
  printf " name = %s,\n edge_probability = %f, bias = %5.1f, seed = %d */\n\n",
    name, edge_probability, bias, random_seed;
  printf "digraph %s {\n", name;
} # NR == 1

NR == 2 { # layer 0
  current_layer = 0;
  layer_size[current_layer] = $1;
} # 

NR > 2 { # layer 1, ..., k-1
  ++current_layer;
  num_nodes = $1;
  layer_size[current_layer] = num_nodes;

  # for all nodes j on the current layer ...
  for( j = 0; j < num_nodes; j++ )
  {
    num_incoming = 0;         # number of incoming nodes for j

    # for all nodes i on the previous layer
    previous_layer = current_layer - 1;
    for( i = 0; i < layer_size[previous_layer]; i++ )
    {
      # add an edge from i to j with probability edge_probability
      if( rand() < edge_probability )
      {
        printf("  n_%d_%d -> n_%d_%d;\n",
               previous_layer, i, current_layer, j);
        num_incoming++;
      }
    } # for all nodes on previous layer

    # no incoming edge for j added -- create one from a random node on the
    # previous layer
    if( num_incoming == 0 )
    {
      random_position = int( rand() * layer_size[previous_layer] / bias );
      printf("  n_%d_%d -> n_%d_%d;\n",
             previous_layer, random_position, current_layer, j);
    }
  } # for all nodes on current layer
} # NR > 2

END {
  print "}";
}

#  [Last modified: 2012 01 06 at 01:54:07 GMT]
