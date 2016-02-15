
#include<iostream>
#include<bitset>
#include<fstream>
#include<string>


using namespace std;

// Function to remove spaces
string trim(string str) {
	while (str[0] == ' ' || str[0] == ',' ) str = str.substr(1);
	if (str == "") return str;
	while (str[str.length() - 1] == ' ' || str[str.length() - 1] == ',') str = str.substr(0, str.length() - 1);
	return str;
}

// Function to get the next word in a string
string readNextWord(string &str) {
	string ret;
	str = trim(str);
	int i = str.find_first_of(',');
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


int main(int argc, char* argv[]){
	int i = 0 , j = 0, k = 0,min_layer= -1,max_layer=0;
	ofstream myfile;
	ifstream rfile;
	string st,temp,temp2 ;


		if (argc != 2 ){
			std::cout << "usage: " << argv[0] << " <filename>.txt\n";
			return 0;
		}
		string inputfilename = argv[1];
		rfile.open(inputfilename, ifstream::in);

		if (inputfilename.substr(inputfilename.size() - 4) != ".txt"){
			std::cout << "File  " << inputfilename << " not a .txt file" << endl;
			return 0;
		}
		else if (!rfile.good()){
			std::cout << "Cant read from the file " << inputfilename << endl;
			return 0;
		}
		rfile.close();
		string outputfilename = inputfilename.substr(0, inputfilename.size() - 3) + "csv";


	
	temp2 = "Input File, No_of_Layers, No_of_Nodes, Isolated_Nodes, Effective_Nodes, No_of_Edges, Edge_Density, MinDegree, MaxDegree, MeanDegree, No of Components, Size of largest component, Preprocessor, Heuristic, Iterations, Runtime, StartCrossings, PreCrossings, HeuristicCrossings, Iteration, FinalCrossings, Iteration, StartEdgeCrossings, PreEdgeCrosings, HeuristicEdgeCrossings, Iteration, FinalEdgeCrossings";

	rfile.open(inputfilename);
	myfile.open(outputfilename);
	int begin=0;
	myfile << temp2 ;
	while (getline(rfile, st))
	{
		if (st == "\n" || st == "" ) continue;
		
		if(st.find_first_of(',') == string::npos)
		{
			if(st.substr(0,3) == "***")
				{
				int cnt = 1;
				while( !st.empty() ){
					temp = readNextWord(st);
				if(cnt == 6 || cnt == 8 || cnt == 10 || cnt == 12)
					myfile << temp << ",";
				}

				
				}
		continue;	
		}
			 
		int com = st.find_first_of(',');
		string type = st.substr(0,com);
		string next = st.substr(com+1);
		//cout << type;
		
		if (type == "GraphName"){
			myfile <<"\n" << next << ",";
		}
		else if (type == "")
			myfile << next << ",";
		else if (type == "NumberOfLayers")
			myfile << next << ",";
		else if (type == "NumberOfNodes")
			myfile << next << ",";
		else if (type == "IsolatedNodes")
			myfile << next << ",";
		else if (type == "EffectiveNodes")
			myfile << next << ",";
		else if (type == "NumberOfEdges")
			myfile << next << ",";
		else if (type == "EdgeDensity")
			myfile << next << ",";
		else if (type == "MinDegree")
			myfile << next << ",";
		else if (type == "MaxDegree")
			myfile << next << ",";
		else if (type == "MeanDegree")
			myfile << next << ",";
		else if (type == "dfs done"){
			string stp = readNextWord(next);
			int eq = stp.find_first_of('=');
			myfile << stp.substr(eq+2) << ",";
			myfile << next.substr(eq + 7) << ",";
		}
		else if (type == "Preprocessor")
			myfile << next << ",";
		else if (type == "Heuristic")
			myfile << next << ",";
		else if (type == "Iterations")
			myfile << next << ",";
		else if (type == "Runtime")
			myfile << next << ",";
		else if (type == "StartCrossings")
			myfile << next << ",";
		else if (type == "PreCrossings")
			myfile << next << ",";
		else if (type == "HeuristicCrossings"){
			myfile << readNextWord(next) << ",";
			temp = readNextWord(next);
			myfile << readNextWord(next) << ",";
			}
		else if (type == "FinalCrossings"){
			myfile << readNextWord(next) << ",";
			temp = readNextWord(next);
			myfile << readNextWord(next) << ",";
			}
		else if (type == "StartEdgeCrossings")
			myfile << next << ",";
		else if (type == "PreEdgeCrossings")
			myfile << next << ",";
		else if (type == "HeuristicEdgeCrossings"){
			myfile << readNextWord(next) << ",";
			temp = readNextWord(next);
			myfile << readNextWord(next) << ",";
			}
		else if (type == "FinalEdgeCrossings"){
			myfile << readNextWord(next);
			temp = readNextWord(next);
			myfile << readNextWord(next);
			}
	}

	myfile.close();
	rfile.close();
	
	//std::cout << "\nProgram Done";
	return 0;

}

