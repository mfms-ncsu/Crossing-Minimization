#! /usr/bin/env python

# Converts a graph in .sgf format to a linear integer program
# This program translate from standard input to standard output.

import sys

MAX_TOKENS_IN_LINE = 100

total_constraint = 0
bottleneck_constraint = 0
stretch_constraint = 0

def usage( program_name ):
    print "Usage:", program_name, " [Total] [Bottleneck] [Strecth] < INPUT_FILE > OUTPUT_FILE"
    print "Takes an sgf file from standard input and converts to lp."
    
# @return a tuple of the form (node_list, edge_list)
# node_list has the form:
# 		[id layer position]
# edge_list has the form:
#       [source target]
def read_sgf( input ):
    node_list = []
    edge_list = []
    line = read_nonblank( input )
    while ( line ):
        type = line.split()[0]
        if type == 'n':
            node_list.append( process_node( line ) )
        elif type == 'e':
            edge_list.append( process_edge( line ) )
            # otherwise error (ignore for now)
        line = read_nonblank( input )
    # sort node_list by id
    node_list = sorted(node_list,key=lambda x: x[0])
    return (node_list, edge_list)	

def process_node( line ):
    line_fields = line.split()
    id = int(line_fields[1])
    layer = line_fields[2]
    position_in_layer = line_fields[3]
    return (id, layer, position_in_layer)

def process_edge( line ):
    line_fields = line.split()
    source = line_fields[1]
    target = line_fields[2]
    return (source, target)
	
# @return the first non-blank line in the input
def read_nonblank( input ):
    line = input.readline()
    while ( line and line.strip() == "" ):
        line = input.readline()
    return line

# minimize different variabe as sepicified by type
# maintain a list of constraints
# prepare output
def minimize(node_list, edge_list, type):
    min = ""
    count = 1
    
    constraints = triangle_constraints(node_list, edge_list)
    constraints.extend(crossing_constraints(node_list, edge_list))
    constraints.extend(position_constraints(node_list, edge_list))
    constraints.extend(bottleneck_constraints(node_list, edge_list))
    constraints.append(total_constraints(node_list, edge_list))
    #constraints.extend(stretch_constraints(node_list, edge_list))
    
    if type == "total":
        min = "t"
        if bottleneck_constraint > 0:
            constraints.append("b <= " + str(bottleneck_constraint))
        #if stretch_constraint > 0:
    elif type == "bottleneck":
        min = "b"
        if total_constraint > 0:
            constraints.append("t <= " + str(total_constraint))
        #if stretch_constraint > 0:
    elif type == "stretch":
        min = "s"
        sys.exit("TODO: Stretch constraint")
      #  if total_constraint > 0:
      #     constraints.append("b <= " + str(total_constraint))
      #  if bottleneck_constraint > 0:
      #     constraints.append("b <= " + str(bottleneck_constraint))
    
    binary, general = variable(node_list, edge_list)
    
    # prepare output
    output = "Min\n obj: "  + min
    output += "\nst\n"
    for item in constraints:
        output += " c" + str(count) + ": " + item + "\n"
        count += 1
    output += "Binary\n"
    tokens_in_line = 0 
    for item in binary:
        output += " " + item
        tokens_in_line += 1 
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0 
    output += "\nGeneral\n"
    tokens_in_line = 0
    for item in general:
        output += " " + item
        tokens_in_line += 1
        if tokens_in_line > MAX_TOKENS_IN_LINE:
            output += "\n"
            tokens_in_line = 0
    output += "\nEnd"
    return output
    
# @return a list of triangle constraints given node list and edge list
def triangle_constraints(node_list, edge_list):
    triangle_constraints = []
    # anti-symmetry constraint
    for idx_i, i in enumerate(node_list):
        for idx_j, j in enumerate(node_list):
            if idx_i < idx_j and i[1] == j[1]: #two nodes on the same layer
                x_i_j = "x_" + str(i[0]) + "_" + str(j[0])
                x_j_i = "x_" + str(j[0]) + "_" + str(i[0])
                triangle_constraints.append("+" + x_i_j + " +" + x_j_i + " = 1" )
    # transitivity constraint
    for idx_i, i in enumerate(node_list):
        for idx_j, j in enumerate(node_list):
            for idx_k, k in enumerate(node_list):
                if idx_i < idx_j and idx_j < idx_k and i[1] == j[1] == k[1]:
                    triangle_constraints.append("-x_" + str(i[0]) + "_" + str(j[0]) + " -x_" + str(j[0]) + "_" + str(k[0]) + " +x_" + str(i[0]) + "_" + str(k[0]) + " >= -1")
    return triangle_constraints
    
# @return a list of crossing constraints given node list and edge list
def crossing_constraints(node_list, edge_list):
    crossing_constraints = []
    # edge crosses if wrong order on the first or second layer
    for idx_i, i in enumerate(edge_list):
        constraint_generated = 0;
        a = int(i[0])
        b = int(i[1])
        channel_i = max(node_list[a][1],node_list[b][1])
        for idx_j, j in enumerate(edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(node_list[c][1],node_list[d][1])
            if channel_i == channel_j and idx_i < idx_j and a != c and a != d and b != c and b != d:
                d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                # wrong order in the first layer
                crossing_constraints.append("-x_" + str(c) + "_" + str(a) + " -x_" + str(b) + "_" + str(d) + " +" + d_i_j + " >= -1")
                # wrong order in the second layer
                crossing_constraints.append("-x_" + str(a) + "_" + str(c) + " -x_" + str(d) + "_" + str(b) + " +" + d_i_j + " >= -1")
                constraint_generated = 1
        if constraint_generated != 1:
            crossing_constraints.append("d_" + str(a) + "_" + str(b) + "_" + str(a) + "_" + str(b) + " >= 0")
    return crossing_constraints
    
# @return a list of position constraints given node list and edge list
def position_constraints(node_list, edge_list):
    position_constraints = []
    # p_i represets the position of node i in its layer 
    for i in node_list:
        p = "p_" + str(i[0]) + "_" + i[1]
        p_i = " -" + p + " = 0"
        for j in node_list:
            if i[1] == j[1] and str(i[0]) != str(j[0]): # distinguish node in same layer
                p_i = " +x_" + str(j[0]) + "_" + str(i[0]) + p_i
        position_constraints.append( p_i )

    return position_constraints
    
# @return a list of bottleneck constraints given node list and edge list
def bottleneck_constraints(node_list, edge_list):
    bottleneck_constraints = []
    for idx_i, i in enumerate(edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(node_list[a][1],node_list[b][1])
        single_edge_crossing = ""
        for idx_j, j in enumerate(edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(node_list[c][1],node_list[d][1])
            if channel_i == channel_j and a != c and a != d and b != c and b != d:
                if idx_i < idx_j:
                    d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                else:
                    d_i_j = "d_" + str(c) + "_" + str(d) + "_" + str(a) + "_" + str(b)
                # sum of crossing for each edge
                single_edge_crossing += " +" + d_i_j
        # bottleneck constraint
        single_edge_crossing += " -b <= 0"
        # ignore standalone edges
        if single_edge_crossing != " -b <= 0":
            bottleneck_constraints.append(single_edge_crossing)
        single_edge_crossing = ""
    return bottleneck_constraints
    
# @return total constraints given node list and edge list
def total_constraints(node_list, edge_list):
    tokens_in_line = 0
    total_constraints = ""
    for idx_i, i in enumerate(edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(node_list[a][1],node_list[b][1])
        for idx_j, j in enumerate(edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(node_list[c][1],node_list[d][1])
            if channel_i == channel_j and idx_i < idx_j and a != c and a != d and b != c and b != d:
                d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                total_constraints += " +" + d_i_j
                tokens_in_line += 1
                if tokens_in_line > MAX_TOKENS_IN_LINE:
                    total_constraints += "\n"
                    tokens_in_line = 0
    total_constraints += " -t <= 0"
    return total_constraints
    
# @retun a list of stretch constraints given node list and edge list
def stretch_constraints(node_list, edge_list):
    sys.exit("TODO: Stretch Constraints")
 
# @return a list of binary variables and a list of general variable 
def variable(node_list,edge_list):
    binary = []
    general = []
    # binary node order variables
    for idx_i, i in enumerate(node_list):
        for idx_j, j in enumerate(node_list):
            if idx_i < idx_j and i[1] == j[1]: #two nodes on the same layer
                x_i_j = "x_" + str(i[0]) + "_" + str(j[0])
                x_j_i = "x_" + str(j[0]) + "_" + str(i[0])
                binary.append(x_i_j)
                binary.append(x_j_i)
    # binary edge crossing variables
    for idx_i, i in enumerate(edge_list):
        a = int(i[0])
        b = int(i[1])
        channel_i = max(node_list[a][1],node_list[b][1])
        for idx_j, j in enumerate(edge_list):
            c = int(j[0])
            d = int(j[1])
            # check if two edges in the same channel without common node
            channel_j = max(node_list[c][1],node_list[d][1])
            if channel_i == channel_j and idx_i < idx_j and a != c and a != d and b != c and b != d:
                d_i_j = "d_" + str(a) + "_" + str(b) + "_" + str(c) + "_" + str(d)
                binary.append(d_i_j)
    # general variables
    for i in node_list:
        p = "p_" + str(i[0]) + "_" + i[1]
        general.append(p)
    general.append("t")
    general.append("b")
    #general.append("s")
    return binary, general

def main():
    if len( sys.argv ) != 4:
        usage( sys.argv[0] )
        sys.exit()
    global total_constraint 
    global bottleneck_constraint
    global stretch_constraint
    total_constraint = int(sys.argv[1])
    bottleneck_constraint = int(sys.argv[2])
    stretch_constraint = int(sys.argv[3])
    
    # constraint can not less than -1
    if total_constraint < -1 or bottleneck_constraint < -1 or stretch_constraint < -1:
        sys.exit("Invalid input: constraint can not less than -1")
    # minimize exact one parameter
    if sum([total_constraint == 0,bottleneck_constraint == 0,stretch_constraint == 0]) != 1:
        sys.exit("Invalid input: excatly one constraint should be 0")
        
    node_list, edge_list = read_sgf( sys.stdin )
    
    # use function minimize do actual work
    if total_constraint == 0:
        output = minimize(node_list, edge_list, "total")
    elif bottleneck_constraint == 0:
        output = minimize(node_list, edge_list, "bottleneck")
    elif stretch_constraint == 0:
        output = minimize(node_list, edge_list, "strecth")
        
    print output
    
main()

#  [Last modified: 2016 03 28 at 18:29:53 GMT]
