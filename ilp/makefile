# makefile for programs that convert among from sgf to lp and from cplex
# output to sgf for an optimal solution, both total and bottleneck crossings

all:
	g++ -g -std=c++0x Graph_to_LP.cpp -o Graph_to_LP
	g++ -g -std=c++0x Sol_to_Graph.cpp -o Sol_to_Graph

clean:
	rm -f Graph_to_LP Sol_to_Graph
