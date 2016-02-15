## sgf2dot.awk - creates a dot file from an sgf file in the most obvious way.
#
# Usage: awk -f sgf2dot.awk input.sgf > output.dot
#
# $Id: sgf2dot.awk 107 2015-04-20 19:00:18Z mfms $

# Note: assumes the simpler sgf format in which nodes have numbers instead of
# "names"; these are created by ogml2sgf (see documentation); the original
# gml2sgf still uses string names based on the unprocessed Rome graphs

BEGIN { # start a comment
  print "/*";
  print " * From an sgf file ...";
}

# render comments verbatim, preceded by *
$1 == "c" {
  printf " *";
  for ( i = 2; i <= NF; i++ ) printf " %s", $i;
  printf "\n";
}

# use title to make digraph name (end initial comment first)
$1 == "t" {
  print " */";
  printf "digraph %s {\n", $2;
}

# found an edge: print it in the correct sequence
$1 == "e" {
  printf " n_%d -> n_%d;\n", $2, $3;
}

END {
  printf "}\n"
}

#  [Last modified: 2014 01 29 at 14:54:25 GMT]
