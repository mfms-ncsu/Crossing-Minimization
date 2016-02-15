#include<iostream>
#include<bitset>
#include<fstream>
#include<string>
#include<map>
#include<list>

using namespace std;


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
		bef = 0;
	}

	int pos, lay, bef;
	string id;
}  node;

map <string, node*> nodes;
map <node*, list<node*>*> edges;
list<int> ids;

int print =0;



// Function to remove spaces
string trim(string str) {
	while (str[0] == ' ' || str[0] == '\t') str = str.substr(1);
	if (str == "") return str;
	while (str[str.length() - 1] == ' ' || str[str.length() - 1] == '\t') str = str.substr(0, str.length() - 1);
	return str;
}

// Function to get the next word in a string
string readNextWord(string &str) {
	string ret;
	str = trim(str);
	int i = str.find_first_of(' ');
	int j = str.find_first_of('\t');
	if (i != -1){
		ret = trim(str.substr(0, i));
		str = trim(str.substr(i + 1));
	}
	else if(j != -1){
		ret = trim(str.substr(0,j));
		str = trim(str.substr(j+1));
	}
	else{
		ret = str;
		str = "";
	}
	return ret;
}

string cnt_str;

// This function gets the variables and the value from a string Eg x_1_4#1 
void get_nodes(string s,int *n1,int *n2, int *n3){
	int a, b;
	a = s.find_first_of('_');
	b = (s.substr(a + 1)).find_first_of('_');
	*n1 = stoi(s.substr(a+1,b));
	*n2 = stoi(s.substr(b + a + 2,s.length()-b-a-4)); 
	*n3 = s.at(s.length() - 1) - 48;
}

//output_sol(choice, inputfile, lpfile, cp_ip, outputfile1);

/// This function does all the work.
///
/// @param digits_to_skip number of digits in the objective if it's a
/// bottleneck optimization; this number will appear at the beginning of the
/// solution string and represents the only variable that is not binary.
void output_sol(int digits_to_skip, string inputfile, string lpfile,
                string cplex_solution_string, string outputfile) {
  int min_layer = -1, max_layer = 0;
  string st, st1, tmp;
  ifstream sgf_stream, lp_stream;
  ofstream output_stream;
  list<string> l;
	list<string> ln;
	bool flg = false;
	sgf_stream.open(inputfile);
	lp_stream.open(lpfile);
	//rfile2.open("E:\\MSQE\\test.txt");
	int no_nodes = 0, no_edges = 0;
	ids.clear();
	output_stream.open(outputfile);

	//Creating the adjecency list from the input file
	while (getline(sgf_stream, st))
	{
		if (st == "\n" || st == "") continue;

		string type = readNextWord(st);
		if (type != "n" && type != "e")
			continue;
		else if (type == "n") {
			string id;
			id = readNextWord(st);
			int pos, lay;
			lay = stoi(readNextWord(st));
			if (min_layer == -1)
				min_layer = lay;
			else if (min_layer > lay)
				min_layer = lay;
			if (max_layer < lay)
				max_layer = lay;
			pos = stoi(readNextWord(st));
			nodes[id] = new node(pos, lay, id);
			edges[nodes[id]] = new list<node*>();
			ids.push_back(stoi(id));
			no_nodes++;
		}
		else if (type == "e") {
			string from, to;
			from = readNextWord(st);
			to = readNextWord(st);
			edges[nodes[from]]->push_back(nodes[to]);
			//cout << from << " -> " << to << ",";
			no_edges++;
		}
	}



	
	ids.sort();
	sgf_stream.close();


	st = cplex_solution_string;
	//cout << "st at input is " << st << endl;
	string fin = "", pt;
	int obj = 0, cnt = 0;

	cnt = digits_to_skip;
	//cout << "\n About to Read data from the Solution \n" << endl;
	// This while loop focusses on reading the solution and concatinating it with the variables
	while (getline(lp_stream, st1))
	{
		if (st1 == "\n" || trim(st1) == "")
			continue;
		//cout << " After the first if" << endl; 
		if (flg == true){
			//cout << " st1:: " << st1 << "\t st:: "<< st << endl; 	
			while (!st1.empty()){
				pt = readNextWord(st1);
				if (pt.find_first_of('d') == 0){
					obj += (st.at(cnt++) - 48);
					continue;
				}

				fin = pt + "#" + st.at(cnt++);
				l.push_back(fin);
				l.sort();
				//cout << fin <<endl;
			}
			break;
		}
		if (readNextWord(st1) == "Binary"){
			//cout << " Found the Binary Variable" << endl; 		
			flg = true;

		}
	}


	//cout << "Adj list done.\t Now appended the Hash and solution  " << endl;
	//int layer = 0;
	cnt = 0;
	int a, b, c;
	string id1, id2;

	// The below module calculates the position of a node, relative to the positions of the other nodes in the same layer, based on the values obtained for the binary 
	// variables in the integer programming problem.  
	
	list<string>::iterator it1;
	for (it1 = l.begin(); it1 != l.end(); ++it1){
		string ele = *it1;	
	//for (auto ele : l) {
		get_nodes(ele, &a, &b, &c);
		id1 = to_string((long long int)a);
		id2 = to_string((long long int)b);
		if (c == 1)
			nodes[id2]->bef++;
		else
			nodes[id1]->bef++;
	}

	//for (auto n : nodes){
	//	if (lay_cnt.find(n.second->lay) != lay_cnt.end())
	//		lay_cnt[n.second->lay]++;
	//	else
	//		lay_cnt[n.second->lay] = 1;
	//}
	int lay = min_layer;
	list<int> lp;
	int state = 0;
	map<int, string> mp;
	
	// The below module would ensure that the graph is sorted by layer,position.
	list<int>::iterator it2;
	for (it2 = ids.begin(); it2 != ids.end(); ++it2){
		int plo = *it2;

	//for (auto plo : ids){
		node *n = nodes[to_string((long long int)plo)];

				
		if(n->lay != lay){
			lp.sort();
			while (!lp.empty()){
				string snode = mp[lp.front()];
				lp.pop_front();
				ln.push_back(snode);
			}
			mp.clear();
			lay++;
			state = 0;
		}

		if (state == 0 && n->bef == 0){
			n->pos = n->bef;
			state = 1;
		}
		else if (state == 1 && n->bef == 0){
			n->pos = 1;
			state = 2;
		}
		else{
			n->pos = n->bef;
		}

		
		lp.push_back(n->pos);
		mp[n->pos] = n->id;
		
	}
	lp.sort();
	while (!lp.empty()){
		string snode = mp[lp.front()];
		lp.pop_front();
		ln.push_back(snode);
	}
	mp.clear();

	sgf_stream.open(inputfile);
	while (getline(sgf_stream, st))
	{
		if (st == "\n" || trim(st) == "")
			continue;
		string type = readNextWord(st);
		if (type == "t") {
			output_stream << "t " << st << "\n";
		}
	}
	sgf_stream.close();
	list<string>::iterator it3;
	for (it3 = ln.begin(); it3 != ln.end(); ++it3){
		string plo = *it3;
	//for (auto plo : ln){
		node *n = nodes[plo];
		//n->pos = n->bef;
		output_stream << "n " << n->id << " " << n->lay << " " << n->pos << "\n";
	}

	sgf_stream.open(inputfile);
	while (getline(sgf_stream, st))
	{
		if (st == "\n" || trim(st) == "")
			continue;
		string type = readNextWord(st);
		if (type == "e") {
			output_stream << "e " << st << "\n";
		}
	}
	sgf_stream.close();
	lp_stream.close();
	output_stream.close();

	map<string,node*>::iterator it9;
	for (it9 = nodes.begin(); it9 != nodes.end(); ++it9){
		delete it9->second;
	//for (auto p : nodes) {
	//	delete p.second;
	}
	nodes.clear();
	map <node*, list<node*>*>::iterator it10;
	for (it10 = edges.begin(); it10 != edges.end(); ++it10){
		delete it10->second;
	//for (auto p : edges) {
	//	delete p.second;
	}
	edges.clear();

	ids.clear();
}


int main(int argc, char* argv[]){

	ifstream readf;
	if (argc != 5 || (argv[1][0] != '-' || (argv[1][1] != 't' && argv[1][1] != 'b'))){
		std::cout << "usage: " << argv[0] << " -t|-b <filename>.sgf <cplex_input>.lp <cplex_output>.txt \n ";
		return 0;
	}
	string inputfilename = argv[2];
	readf.open(inputfilename, ifstream::in);

	if (inputfilename.substr(inputfilename.size() - 3) != "sgf"){
		std::cout << "File  " << inputfilename << " not a .sgf file" << endl;
		return 0;
	}
	else if (!readf.good()){
		std::cout << "Cant read from the file " << inputfilename << endl;
		return 0;
	}
	readf.close();

	string lpfilename = argv[3];
	readf.open(lpfilename, ifstream::in);
	if (lpfilename.substr(lpfilename.size() - 3) != ".lp"){
		std::cout << "File  " << lpfilename << " not a .lp file" << endl;
		return 0;
	}
	else if (!readf.good()){
		std::cout << "Cant read from the file " << lpfilename << endl;
		return 0;
	}
	readf.close();

	string solution = argv[4];
	//cout << solution << " is solution the text file " << endl ;
	readf.open(solution, ifstream::in);
	if (solution.substr(solution.size() - 4) != ".txt"){
		cout << "File  " << solution << " not a .txt file" << endl;
		return 0;
	}
	else if (!readf.good()){
		std::cout << "Cant read from the file " << solution << endl;
		return 0;
	}
	readf.close();
	
	string outputfilename = inputfilename.substr(0, inputfilename.size() - 4) + "_o.sgf";

	string tempstr,cplex_sol;
	readf.open(solution, ifstream::in);

    // holds the number of the objective function
    string objective;
    // number of digits in the objective function; need to skip past these if
    // objective is bottleneck
    int objective_digits = 0;
	while (getline(readf, tempstr)){
		if (tempstr == "\n" || tempstr == "") continue;
		string plo = readNextWord(tempstr);		
		//if (print)
		//cout << "plo :: " << plo << "\ttempstr " <<tempstr << endl;		
		if ( plo == "Objective" ) {
          objective = readNextWord(tempstr);
          objective_digits = objective.length();
          cout << "objective = " << objective << ", digits = " << objective_digits << endl;
        }
		if(plo == "Solution"){	
			cplex_sol = readNextWord(tempstr);
			//cout << "cplex_sol :: " << cplex_sol << endl;
			break;
		}
		
	}
	readf.close();
	
	if (argv[1][1] == 't'){
		output_sol(0, inputfilename, lpfilename, cplex_sol, outputfilename);
	}
	else if (argv[1][1] == 'b'){
		output_sol(objective_digits, inputfilename, lpfilename, cplex_sol, outputfilename);
	}

	//cout << "\nProgram Done\n";
	return 0;
}



//  [Last modified: 2016 01 28 at 21:44:40 GMT]
