#! /bin/bash

# constrained_stretch - starts with an sgf file and finds an ordering that
# minimizes stretch, using the constraint that the stretch of any individual
# edge is never greater than one (this gets better results with CPLEX)
# This version works with python 2.x and the corresponding sgf2ilp.py
# version.
# assumes sgf2ilp.py and sol2sgf.py are in this directory

# @todo update to sgf2ilp.py for python 2.7 and later (with argparse)
                                                                                   
TIME_LIMIT=1200                 # number of seconds until timeout
if [ $# -ne 1 ]; then
    echo "Usage: $0 FILE.sgf"                 
    echo " produces files of the form, in the same directory as the input"
    echo "  FILE-s.lp (ILP for minimizing stretch - constrained)"
    echo "  FILE-s.out (the cplex output when the ILP is solved)"
    echo "  FILE-s.sgf, (the sgf file with the optimum order)"
    exit 1
fi

# python scripts are in same directory as this one
script_dir=${0%/*}

input_sgf_file=$1
input_base=${input_sgf_file%.sgf}

# minimize stretch subject to no edge having stretch more than 1
cplex_input_file=${input_base}-s.lp
cplex_output_file=${input_base}-s.out
sgf_output_file=${input_base}-s.sgf

echo "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
echo "creating ILP, objective is $first_objective, input file is $input_sgf_file"
$script_dir/sgf2ilp.py -1 -1 -2 1 < $input_sgf_file > $cplex_input_file
echo -n "solving $cplex_input_file,      "
date -u
cplex_ilp -time=$TIME_LIMIT -solution $cplex_input_file > $cplex_output_file
echo -n "done solving $cplex_input_file, "
date -u

echo "converting solution to sgf for optimal order"
$script_dir/sol2sgf.py < $cplex_output_file > $sgf_output_file

minimum_stretch=`fgrep Objective $cplex_output_file | cut -f 2`
echo "minimum strech is $minimum_stretch"

echo "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
echo

#  [Last modified: 2016 05 17 at 14:06:07 GMT]
