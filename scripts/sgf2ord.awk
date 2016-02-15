## sgf2ord.awk - gathers layering information from an sgf file to create an
# ord file.
# $Id: sgf2ord.awk 107 2015-04-20 19:00:18Z mfms $

BEGIN {
  MAX_NODES_PER_LINE = 10;
  seen_a_node = 0;
  print "# Ordering based on sgf information";
}

$1 == "c" {
  printf "#";
  for ( i = 2; i <= NF; i++ ) printf " %s", $i;
  printf "\n";
}

$1 == "t" {
  printf "# Graph name = %s\n", $2;
}

$1 == "n" {
  if ( ! seen_a_node ) {
    printf "\n# Order for layer %d based on sgf file.\n", layer;
    printf "%d {\n", layer;
    seen_a_node = 1;
  }
  if ( layer != $3 ) {
    # end previous layer
    printf "\n} # end of layer %d\n", layer;
    # start a new layer
    layer = $3;
    nodes_on_line = 0;
    printf "\n# Order for layer %d based on sgf file.\n", layer;
    printf "%d {\n", layer;
  }
  else {
    if ( nodes_on_line > MAX_NODES_PER_LINE ) {
      printf "\n";
      nodes_on_line = 0;
    }
  }
  printf "n_%d ", $2;          # node name
  nodes_on_line++;
}

END {
  # end last layer
  printf "\n} # end of layer %d\n", layer;
}

#  [Last modified: 2012 01 06 at 01:57:34 GMT]
