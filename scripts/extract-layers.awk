## extract-layers.awk - turns x- and y- coordinates into layers and layer
# orderings, assuming that input format is given by the output format of
# extract-graph.awk. Output (sgf) format is described in ogml2sgf.
#
# $Id: extract-layers.awk 107 2015-04-20 19:00:18Z mfms $

BEGIN {
  first = 1;
  layer = 0;
}

$1 == "n" {
  if ( first ) y = $4;
  first = 0;
  if ( y != $4 ) {
    # new layer
    y = $4;
    layer++;
    position = 0;
  }
  else position++;
  print "n", $2, layer, position;
}

$1 != "n" {
  print;
}

#  [Last modified: 2012 01 06 at 01:52:17 GMT]
