## makefile - for programs related to edge crossing minimization
#
# $Id: makefile 131 2016-01-12 01:07:39Z mfms $

CC=gcc
DFLAGS = 
OFLAGS = -O3
#MPFLAGS = -fopenmp
MPFLAGS =
# for specifying that MAX_EDGE (maximum crossings for an edge) and/or PARETO
# (create a list of Pareto optimum solutions) is to be used
EFLAGS = -DMAX_EDGE -DPARETO
# all programs are compiled with max edge objective included
CFLAGS = -c -Wall -g -std=c99 $(EFLAGS) $(MPFLAGS) $(OFLAGS) $(DFLAGS)
#CLIBS  = -lm -lgomp
CLIBS = -lm

# all programs that can be created
PROGRAMS = min_crossings create_random_dag add_edges dot_and_ord_to_sgf rand_seq

# object files common to all heuristics
OBJECTS = sifting.o dfs.o sorting.o heuristics.o barycenter.o crossings.o\
	crossing_utilities.o graph_io.o dot.o ord.o hash.o Statistics.o stats.o\
	order.o priority_edges.o swap.o median.o timing.o random.o

# object files used by programs that generate random instances
CREATION_OBJECTS = check_edge_duplication.o random_dag.o random_tree.o graph_io.o Statistics.o dot.o ord.o hash.o

# header files common to all heuristics; also make sure recompilation takes
# place if there are changes to the makefile
HEADERS = makefile defs.h crossings.h graph.h graph_io.h dot.h ord.h\
	crossing_utilities.h heuristics.h barycenter.h sorting.h dfs.h sifting.h\
	Statistics.h stats.h order.h priority_edges.h swap.h median.h timing.h\
	random.h makefile

# headers used by programs that generate random instances
CREATION_HEADERS = check_edge_duplication.h graph.h graph_io.h random_tree.h random_dag.h hash.h defs.h dot.h ord.h Statistics.h

.SUFFIXES: .c
.c.o: ; $(CC) $(CFLAGS) $*.c

all: $(PROGRAMS)

min_crossings: min_crossings.o $(OBJECTS)\
; $(CC) $(DFLAGS) min_crossings.o $(OBJECTS) $(CLIBS) -o min_crossings

create_random_dag: create_random_dag.o $(CREATION_OBJECTS)\
; $(CC) $(OFLAGS) create_random_dag.o $(CREATION_OBJECTS) -lm -o create_random_dag

add_edges: add_edges.o $(CREATION_OBJECTS)\
; $(CC) $(OFLAGS) add_edges.o $(CREATION_OBJECTS) -lm -o add_edges

dot_and_ord_to_sgf: dot_and_ord_to_sgf.o graph_io.o dot.o ord.o hash.o\
; $(CC) $(OFLAGS) dot_and_ord_to_sgf.o dot.o graph_io.o ord.o hash.o -o dot_and_ord_to_sgf 

graph_input_test: graph_input_test.o dot.o ord.o hash.o\
; $(CC) $(DFLAGS) graph_input_test.o dot.o ord.o hash.o -o graph_input_test

crossings_test: crossings_test.o sorting.o heuristics_test.o\
	 graph_io.o dot.o ord.o hash.o crossing_utilities.o\
; $(CC) $(DFLAGS) crossings_test.o sorting.o heuristics_test.o\
	 graph_io.o dot.o ord.o hash.o crossing_utilities.o -o crossings_test

rand_seq: rand_seq.c;
	$(CC) rand_seq.c -o rand_seq

# source dependencies

graph_input_test.o: graph_io.c $(HEADERS)\
; $(CC) $(CFLAGS) -DTEST graph_io.c -o graph_input_test.o

crossings_test.o: crossings.c $(HEADERS)\
; $(CC) $(CFLAGS) -DTEST crossings.c -o crossings_test.o

heuristics_test.o: heuristics.c $(HEADERS)\
; $(CC) $(CFLAGS) -DTEST heuristics.c -o heuristics_test.o

min_crossings.o: min_crossings.c $(HEADERS)

dot_and_ord_to_sgf.o: dot_and_ord_to_sgf.c $(HEADERS)

heuristics.o: heuristics.c $(HEADERS)

barycenter.o: barycenter.c $(HEADERS)

dfs.o: dfs.c $(HEADERS)

sorting.o: sorting.c $(HEADERS)

dot.o: dot.c $(HEADERS)

ord.o: ord.c $(HEADERS)

hash.o: hash.c $(HEADERS)

crossings.o: crossings.c $(HEADERS)

crossing_utilities.o: crossing_utilities.c $(HEADERS)

sifting.o: sifting.c $(HEADERS)

graph_io.o: graph_io.c $(HEADERS)

order.o: order.c $(HEADERS)

create_random_dag.o: create_random_dag.c $(CREATION_HEADERS)

add_edges.o: add_edges.c $(CREATION_HEADERS)

random_dag.o: random_dag.c $(CREATION_HEADERS)

random_tree.o: random_tree.c $(CREATION_HEADERS)

random_dag.o: random_dag.c $(CREATION_HEADERS)

check_edge_duplication.o: random_dag.c $(CREATION_HEADERS)

hash.o: hash.c $(CREATION_HEADERS)

stats.o: stats.c $(HEADERS)

Statistics.o: Statistics.c $(HEADERS)

priority_edges.o: priority_edges.c $(HEADERS)

swap.o: swap.c $(HEADERS)

median.o: median.c $(HEADERS)

timing.o: timing.c $(HEADERS)

random.o: random.c $(HEADERS)

clean: ; rm -f *.o $(PROGRAMS) *_test

# the effect of this is insert one 'x' in a comment on the last line of
# min_crossings.c in order to ensure that it's version id is updated and it
# can correctly report the latest version number
version: ; ed min_crossings.c < update_version ; svn commit
