## gr2sgf.awk - can be used to convert some graphs to sgf
# $Id: gr2sgf.awk 107 2015-04-20 19:00:18Z mfms $

# print comments as is
/^c/ {print;}
# title line is 'p sp nodes edges' in shortest path problems, may have other
# second fields; should probably add number of nodes and edges to title line
# of sgf format if its known
/^p/ {
    printf "t %s_%d_%d\n", $2, $3, $4;
    # print a line for each node
    nodes = $3;
    for ( i=1; i<=nodes; i++ )
        print "n" , i;
}

# in shortest path problems (directed graphs) each edge is an 'arc', hence
# the 'a'
 /^a/ {
     print "e", $2, $3;
 }

#  [Last modified: 2012 01 06 at 01:53:32 GMT]
