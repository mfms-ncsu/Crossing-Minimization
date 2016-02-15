/// @file Graph_to_LP_ch.cpp
/// Converts a graph in .sgf format to a linear integer program that
/// minimizes crossings. With the -t flag, the program minimizes total
/// crossings, with the -b flag, it minimizes bottleneck (max edge) crossings.
///
/// @todo !!! Caution: assumes node numbers are consecutive integers and that
/// layers are contiguous.
//
/// @author Unday Sonthy, modified by Matthias Stallmann
///
/// compile as
///    g++ -std=c++0x Graph_to_LP.cpp -o Graph_to_LP_ch
/// a command that is also in the makefile.
/// This file represents a working version, cleaning up Uday's code and
/// adding constraints that can be used to find solutions that are optimum
/// with respect to both objectives.
///
/// $Id: Graph_to_LP.cpp 140 2016-02-15 16:13:56Z mfms $

#include<iostream>
#include<bitset>
#include<fstream>
#include<string>
#include<map>
#include<list>
#include <thread>
#include<chrono>
#include<sstream>
#include<limits.h>

using namespace std;

/// The output file stream
ofstream output_stream;

/// bound on the maximum number of crossings among the edges (bottleneck); if
/// less than INT_MAX, can be used to find minimum total crossings, given a
/// fixed bound on bottleneck crossings
int max_edge_bound = INT_MAX;
/// true if there is a bound on the number of bottleneck crossings
bool edge_bound_exists = false;

/// similar to max_edge_bound, but this one can be used to find minimum
/// bottleneck crossing given a bound on total crossings
int max_crossings_bound = INT_MAX;
/// true if there is a bound on the number of total crossings
bool crossings_bound_exists = false;

/// true if the objective is to minimize bottleneck crossings
bool bottleneck = false;

/// stores the set constraints as a string stream
ostringstream constraints;

/// the objective as a string stream
ostringstream objective;

/// declaration of general and binary variables as a string stream
ostringstream variable_declarations;

typedef struct e {
	e(int s, int d){
		src = s;
		dst = d;
	}
	int src, dst;
}  edge;

typedef struct n {
	n(int Pos, int Lay, string Id) {
		pos = Pos;
		lay = Lay;
		id = Id;
	}

	int pos, lay;
	string id;
}  node;

/// used to label constraints in sequence
int constraint_number = 0;
list<string> peers, con, allvars;
map <string, node*> nodes;
map <node*, list<node*>*> edges;
list<int> ids;
list<edge*> lt;
list<int> ltn;

//Function to find if an element already exists in the list.
bool find(list<string> *p, string s){

	list<string>::iterator it2;
	for (it2 = p->begin(); it2 != p->end(); ++it2){
	//for (string element : *p){
		if (*it2 == s)
			return true;
	}
	return false;
}	

//A condition to maintain linear orderings between nodes in a layer. 
void triangle_condition(int a, int b, int p){

	//cout << "\n TCP "<< a << " " << b << " " << p ;
	//std::this_thread::sleep_for(std::chrono::seconds(1));
	constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
	string s = "";
	s = "+x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)b) + " +x_" + std::to_string((long long int)b) + "_" + std::to_string((long long int)p)
		+ " -x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)p) + " >=" + " 0\n";
	constraints << s;
	constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
	s = "-x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)b) + " -x_" + std::to_string((long long int)b) + "_" + std::to_string((long long int)p)
		+ " +x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)p) + " >=" + " -1\n";
	constraints << s;
	if (!find(&peers, "x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)b)))
		peers.push_back("x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)b));
	if (!find(&peers, "x_" + std::to_string((long long int)b) + "_" + std::to_string((long long int)p)))
		peers.push_back("x_" + std::to_string((long long int)b) + "_" + std::to_string((long long int)p));
	if (!find(&peers, "x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)p)))
		peers.push_back("x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)p));
	if (!find(&allvars, "x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)b)))
		allvars.push_back("x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)b));
	if (!find(&allvars, "x_" + std::to_string((long long int)b) + "_" + std::to_string((long long int)p)))
		allvars.push_back("x_" + std::to_string((long long int)b) + "_" + std::to_string((long long int)p));
	if (!find(&allvars, "x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)p)))
		allvars.push_back("x_" + std::to_string((long long int)a) + "_" + std::to_string((long long int)p));
}

// These are the constraints which charecterize the crossing variables
void crossing_condition(edge e1, edge e2){
	string s = "";
	int s1, s2, d1, d2;
	constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
	s1 = e1.src;
	s2 = e2.src;
	if (e1.dst < e2.dst){
		d1 = e1.dst;
		d2 = e2.dst;
		s = "+x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2) + " -x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2)
			+ " +d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst) + " >=" + " 0\n";
		constraints << s;
		constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
		s = "-x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2) + " +x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2)
			+ " +d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst) + " >=" + " 0\n";
		constraints << s;
		s = "";

	}
	else{
		d1 = e2.dst;
		d2 = e1.dst;
		s = "+x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2) + " +x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2)
			+ " +d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst) + " >=" + " 1\n";
		constraints << s;
		constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
		s = "-x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2) + " -x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2)
			+ " +d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst) + " >=" + " -1\n";
		constraints << s;
		s = "";

	}
	if (!find(&peers, "x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2)))
		peers.push_back("x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2));
	if (!find(&peers, "x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2)))
		peers.push_back("x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2));
	if (!find(&con, "d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst)))
		con.push_back("d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst));
	if (!find(&allvars, "x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2)))
		allvars.push_back("x_" + std::to_string((long long int)d1) + "_" + std::to_string((long long int)d2));
	if (!find(&allvars, "x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2)))
		allvars.push_back("x_" + std::to_string((long long int)s1) + "_" + std::to_string((long long int)s2));
	if (!find(&allvars, "d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst)))
		allvars.push_back("d_" + std::to_string((long long int)e1.src) + "_" + std::to_string((long long int)e1.dst) + "_" + std::to_string((long long int)e2.src) + "_" + std::to_string((long long int)e2.dst));

}

// This function adds the constraint representing the bounds for the crossings on each of the edges. 
void bottleneck_condition(list<string>* lstr){
	string s = "";
	constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
	list<string>::iterator itp;
	for (itp = lstr->begin(); itp != lstr->end(); itp++){
		s += *itp + " ";
	}
	s += "+b >= 0\n";
	constraints << s;
}

// Function to remove spaces
string trim(string str) {
	while (str[0] == ' ') str = str.substr(1);
	if (str == "") return str;
	while (str[str.length() - 1] == ' ') str = str.substr(0, str.length() - 1);
	return str;
}

// Function to get the next word in a string by considering a space as a delimiter
string readNextWord(string &str) {
	string ret;
	str = trim(str);
	int i = str.find_first_of(' ');
	if (i != -1){
		ret = trim(str.substr(0, i));
		str = trim(str.substr(i + 1));
	}
	else{
		ret = str;
		str = "";
	}
	return ret;
}

string cnt_str;

// This is a function mainly prepares the integer programming file from the input
void create(string ip, string op){
	int i = 0 , j = 0, k = 0,min_layer= -1,max_layer=0;
	ifstream rfile;

	rfile.open(ip);
	int no_nodes=0;
    int no_edges=0;
	ids.clear();

    string current_line;
	//Creating the adjacency list from the input file
	while (getline(rfile, current_line))
	{
		if ( current_line == "\n" || current_line == "") continue;

		string type = readNextWord(current_line);
		if (type != "n" && type != "e")
			continue;
		else if (type == "n") {
			string id;
			id = readNextWord(current_line);
			int pos, lay;
			lay = stoi(readNextWord(current_line));
			if (min_layer == -1)
				min_layer = lay;
			else if (min_layer > lay)
				min_layer = lay;
			if (max_layer < lay)
				max_layer = lay;
			pos = stoi(readNextWord(current_line));
			nodes[id] = new node(pos, lay, id);
			edges[nodes[id]] = new list<node*>();
			ids.push_back(stoi(id));
			no_nodes++;
		}
		else if (type == "e") {
			string from, to;
			from = readNextWord(current_line);
			to = readNextWord(current_line);
			edges[nodes[from]]->push_back(nodes[to]);
			//cout << from << " -> " << to << ",";
			no_edges++;
		}
	}
	ids.sort();
	cnt_str = "\t node :: " + std::to_string((long long int)no_nodes) + "\t edges :: " + std::to_string((long long int)no_edges) + "\t layers :: " + std::to_string((long long int)max_layer - min_layer);
	//cout << cnt_str << endl;
	int layer = min_layer;
	edge* edp;
	node *n1, *n2;
	list<int>::iterator it2, it3;
	int it1, cnt = 0,firsttc=1;
	//cout << "\n Size Before starting " << lt.size();
	while(!ids.empty()) {
		node *n = nodes[std::to_string((long long int)*ids.begin())];
		ids.pop_front();
		pair<node*, list<node*>*> edg(n,edges[n]);

		if (edg.first->lay != layer) {

			while (!ltn.empty()){
				if (ltn.size() >= 3)	{
					if (firsttc){
						//cout << "\n FC it1, .next, .next.next " << (int)*(ltn.begin()) << " " << *ltn.begin() << " " << *ltn.begin();
						firsttc = 0;
					}

					it1 = *ltn.begin();
					for (it2 = ltn.begin(); it2 != ltn.end(); ++it2)
					for (it3 = it2; it3 != ltn.end(); ++it3)
					if ((it1 != *it2) && (*it2 != *it3) && (*it3 != it1)){
						triangle_condition(it1, *it2, *it3);

					}
						

				}
				ltn.pop_front();
			}


			list<edge*>::iterator it5;
			for (it5 = lt.begin(); it5 != lt.end(); ++it5){
			//edge* edp = new edge(it5->src,it5->dst);
				edge* edp= *it5;
			//for (auto edp : lt){ 
				list<string> lstr;
				string ts;
				list<edge*>::iterator it6;
				for (it6 = lt.begin(); it6 != lt.end(); ++it6){
					edge* lte= *it6;					
					if (edp->src != lte->src && edp->dst != lte->dst){
						if (edp->src < lte->src)
							ts = "d_" + std::to_string((long long int)edp->src) + "_" + std::to_string((long long int)edp->dst) + "_" + std::to_string((long long int)lte->src) + "_" + std::to_string((long long int)lte->dst);
						else
							ts = "d_" + std::to_string((long long int)lte->src) + "_" + std::to_string((long long int)lte->dst) + "_" + std::to_string((long long int)edp->src) + "_" + std::to_string((long long int)edp->dst);
						lstr.push_back("-" + ts);
						if (!find(&con, ts))
							con.push_back(ts);
						if (!find(&allvars, ts))
							allvars.push_back(ts);
					}
				}
				if ( ! lstr.empty() && (bottleneck || edge_bound_exists) )
					bottleneck_condition(&lstr);

			}

			while (!lt.empty()){
				edp = lt.front();
				list<edge*>::iterator it6;
				for (it6 = lt.begin(); it6 != lt.end(); ++it6){
					edge* lte = *it6;
				//for (auto lte : lt){
					if (edp->src != lte->src && edp->dst != lte->dst){
						crossing_condition(*edp, *lte);
					}
				}
				lt.pop_front();
			}

			layer = edg.first->lay;
			cnt = 0;
			//cout << __TIMESTAMP__<< endl;
		}

		

		cnt++;
		ltn.push_back(stoi(edg.first->id));
		
		
		list<node*>::iterator it7;
		for (it7 = (edg.second)->begin(); it7 != (edg.second)->end(); ++it7){
			node* nd= *it7;
		//for (auto nd : *edg.second){
			lt.push_back(new edge(stoi(edg.first->id), stoi(nd->id)));
		}
	}	
	while (!ltn.empty()){
		if (ltn.size() >= 3)	{
			it1 = *ltn.begin();
			for (it2 = ltn.begin(); it2 != ltn.end(); ++it2)
			for (it3 = it2; it3 != ltn.end(); ++it3)
			if ((it1 != *it2) && (*it2 != *it3) && (*it3 != it1)){
				triangle_condition(it1, *it2, *it3);
			}
		}
		ltn.pop_front();
	}

    // string containing the sum of all the binary variables that represent
    // the existence of crossings
	string sum_of_crossings = "";

	list<string>::iterator it8;
	
	for ( it8 = con.begin(); it8 != con.end(); ++it8 )
		sum_of_crossings += " +" + *it8;		

	if ( bottleneck || edge_bound_exists )
      variable_declarations << "Generals" << endl << " b" << endl;
	variable_declarations << endl << "Binary" << endl;
	
	if ( bottleneck || edge_bound_exists ) {
		for (it8 = allvars.begin(); it8 != allvars.end(); ++it8)
			variable_declarations << " " + *it8;
	}
	else{
		for (it8 = con.begin(); it8 != con.end(); ++it8)
			variable_declarations << " " + *it8;
		for (it8 = peers.begin(); it8 != peers.end(); ++it8)
			variable_declarations << " " + *it8;
	}

	objective << "Min" << endl << "\t obj:";
	if (bottleneck)
		objective << " b";
	else
		objective << sum_of_crossings;

    // add the two constraints related to bounding bottleneck and total
    // crossings if appropriate
    if ( edge_bound_exists ) {
      constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
      constraints << " b <= " << max_edge_bound << endl;
    }
    if ( crossings_bound_exists ) {
      constraints << "\t c" + std::to_string((long long int)++constraint_number) + ": ";
      constraints << sum_of_crossings << " <= " <<  max_crossings_bound << endl;
    } 

    output_stream.open(op);

	output_stream << objective.str() << endl << "st" << endl
                  << constraints.str() << endl
                  << variable_declarations.str() << endl << "End" << endl;

	output_stream.close();
	rfile.close();

	peers.clear();
	con.clear();
	allvars.clear();
	map<string,node*>::iterator it9;
	for (it9 = nodes.begin(); it9 != nodes.end(); ++it9){
		delete it9->second;	
	}
	nodes.clear();
	map <node*, list<node*>*>::iterator it10;
	for (it10 = edges.begin(); it10 != edges.end(); ++it10){
		delete it10->second;	
	}
	edges.clear();
	list<edge*>::iterator it11;
	for (it11 = lt.begin(); it11 != lt.end(); ++it11){
		delete *it11;
	}
	lt.clear();
	ltn.clear();
}

void print_usage(const string program_name) {
  std::cout << "Usage: " << program_name << " -t|-b [-e EBOUND | -x TBOUND] FILE.sgf"
            << endl;
  std::cout << " where -t means minimize total crossings," << endl;
  std::cout << "       -b means minimize bottleneck crossings," << endl;
  std::cout << "       EBOUND is a bound on the crossings for any edge, i.e., bottleneck crossings" << endl;
  std::cout << "       TBOUND is a bound on the total number of crossings" << endl;
  std::cout << " EBOUND and TBOUND can be used to find out if" << endl;
  std::cout << " the minimum bottleneck and total crossings can be achieved simultaneously" << endl;
}

int main(int argc, char* argv[]){

  string program_name = argv[0];

  if ( argc < 3 ) {
    print_usage(program_name);
    return EXIT_FAILURE;
  }

  int arg_number = 0;

  arg_number++;
    
  // minimize total crossings
  if (argv[arg_number][1] == 't'){
    bottleneck = false;
  }
  // minimize bottleneck crossings
  else if (argv[arg_number][1] == 'b'){
    bottleneck = true;
  }
  else {
    std::cout << "expected -b or -t as first flag" << endl;
    print_usage(program_name);
    return EXIT_FAILURE;
  }
    
  arg_number++;

  // put an upper bound on the maximum number of crossings on any edge
  if ( argv[arg_number][0] == '-' && argv[arg_number][1] == 'e' ) {
    arg_number++;
    max_edge_bound = atoi(argv[arg_number]);
    edge_bound_exists = true;
    arg_number++;
  }
  // put an upper bound on the maximum number of total crossings
  else if ( argv[arg_number][0] == '-' && argv[arg_number][1] == 'x' ) {
    arg_number++;
    max_crossings_bound = atoi(argv[arg_number]);
    crossings_bound_exists = true;
    arg_number++;
  }
  else if ( argv[arg_number][0] == '-' ) {
    cout << "expected -e or -x as second flag" << endl;
    print_usage(program_name);
    return EXIT_FAILURE;
  }

  string inputfilename = argv[arg_number];

  ifstream readf;
  readf.open(inputfilename, ifstream::in);

  if ( inputfilename.substr(inputfilename.size() - 3) != "sgf" ) {
    std::cout << "File  " << inputfilename << " not a .sgf file" << endl;
    return EXIT_FAILURE;
  }
  else if ( ! readf.good() ) {
    std::cout << "Can't read from the file " << inputfilename << endl;
    return EXIT_FAILURE;
  }
  readf.close();
  string outputfilename
    = inputfilename.substr(0, inputfilename.size() - 3) + "lp";

    create(inputfilename, outputfilename);

	return EXIT_SUCCESS;
}

//  [Last modified: 2016 02 11 at 19:24:06 GMT]
