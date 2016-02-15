#! /bin/bash

# @file pareto_opt
# finds the minimum number of total crossings given the minimum number of bottleneck crossings
# and the minimum number of bottleneck crossings given the minimum number of total crossings
# Uses sgf2opt, which is assumed to be in the same directory
#
# $Id: pareto_opt 140 2016-02-15 16:13:56Z mfms $

if [ $# -ne 1 ]; then
    echo "Usage: pareto_opt BASE.sgf"
    echo " where BASE.sgf is an sgf file"
    echo " creates 12 output files, three each for TAG = b, t, bx, te"
    echo "  b and t are for minimum bottleneck and total crossings, respectively"
    echo "  bx and te are for min bottleneck given min total and min total given min bottleneck, respectively"
    echo "  The files are:"
    echo "   BASE_TAG.lp - ilp_formulation of the appropriate minimization problem"
    echo "   BASE_TAG.out - CPLEX output from solving BASE_TAG.lp"
    echo "   BASE_optimum_TAG.sfg - sgf file giving the ordering that leads to the minimum solution"
    echo "  The value of the minimum can be retrieved via"
    echo "    fgrep Objective BASE_TAG.out | cut -f 2"
fi

graph=$1
graph_base=${graph%.sgf}

script_dir=${0%/*}
sgf2opt=$script_dir/sgf2opt

$sgf2opt -t $graph
min_total=`fgrep Objective ${graph_base}_t.out | cut -f 2`
echo "Minimum total crossings = $min_total"

$sgf2opt -b $graph
min_bottleneck=`fgrep Objective ${graph_base}_b.out | cut -f 2`
echo "Minimum bottleneck crossings = $min_bottleneck"

$sgf2opt -t -e $min_bottleneck $graph
$sgf2opt -b -x $min_total $graph

#  [Last modified: 2016 01 18 at 20:26:51 GMT]
