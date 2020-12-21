#! /usr/bin/env python

"""
collectParetoPoints.py - designed to gather multiple Pareto points into a
  single Pareto list; it's a simple filter (for now)

 input is a sequence of lines of the form (Pareto output from min_crossings)
   x1^y1;x2^y2;...
 output has one Pareto point per line - format is tab separated
"""

import sys
import copy

def main():
    pareto_list = read_points(sys.stdin)
    pareto_list = gather_pareto_points(pareto_list)
    print_points(sys.stdout, pareto_list)

# @return a list of Pareto points in the form [(x1,y1), ...]
def read_points(input):
    pareto_list = []
    line = input.readline().strip()
    while line:
        pareto_list.extend(process_line(line))
        line = input.readline().strip()
    return pareto_list

# @param pareto_input a string of the form 'x1^y1;x2^y2;...'
# @return a list of the form [(x1,y1), (x2,y2), ...]
def process_line(pareto_input):
    pareto_list = []
    points = pareto_input.split(';')
    for point in points:
        x, y = point.split('^')
        pareto_list.append((float(x), float(y)))
    return pareto_list

# @param point_list a list of the form [(x1,y1), (x2,y2), ...]

# @return point_list with points that are dominated by others removed; a
# point (y,z) is dominated if there exists another point (w,x) such that w <= y
# and x <= z
def gather_pareto_points(point_list):
    undominated_list = []
    for point in point_list:
        dominated = False
        # deep copy may not be necessay but doesn't hurt
        for undominated_point in copy.deepcopy(undominated_list):
            if dominates(undominated_point, point):
                dominated = True
                break
            elif dominates(point, undominated_point):
                undominated_list.remove(undominated_point)
        if not dominated:
            undominated_list.append(point)
    return undominated_list
                

# @return true if first_point dominates second_point
def dominates(first_point, second_point):
    if first_point[0] <= second_point[0] and first_point[1] <= second_point[1]:
        return True
    return False

# prints the pareto points, one per line, tab separated
def print_points(output_stream, pareto_list):
    for point in pareto_list:
        output_stream.write("%f\t%f\n" % point)

main()

#  [Last modified: 2020 12 21 at 15:57:36 GMT]
