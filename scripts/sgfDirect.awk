## sgfDirect.awk - directs edges of an sgf file using node order
# Usage: awk -f sgfDirect.awk input.sgf > output.sgf
#
# $Id: sgfDirect.awk 107 2015-04-20 19:00:18Z mfms $

# preserve title and comments
$1 == "t" || $1 == "c" { print; }

# found a node: give it the next sequence number and print it.
$1 == "n" { seq[$2] = ++seq_cnt; print; }

# found an edge: print it in the correct sequence
$1 == "e" {
  if ( seq[$2] < seq[$3] )
     print "e", $2, $3;
  else  print "e", $3, $2;
}

#  [Last modified: 2011 06 16 at 19:20:36 GMT]
