
#include<iostream>
#include<bitset>
#include<fstream>
#include<string>


using namespace std;



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

int main(int argc, char* argv[]){
	int i = 0 , j = 0, k = 0,min_layer= -1,max_layer=0;
	ofstream myfile,myfile1;
	ifstream rfile;
	string st,temp2;

	temp2 = "Input File,Timeout,Variables,Constraints,NonZeros,Runtime,TimedOut,SolutionFound,ProvedOptimal,StatusCode,Nodes,Objective,Iterations";

	if (argc != 2 ){
		std::cout << "usage: " << argv[0] << " <filename>.txt\n";
		return 0;
	}
	string inputfilename = argv[1];
	rfile.open(inputfilename, std::ifstream::in);

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
	string out_fl;


	rfile.open(inputfilename);
	myfile.open(outputfilename);

	myfile << temp2 ;
	int begin=0;
	string tp,top;
	while (getline(rfile, st))
	{
		//cout << " inwhile loop " + st + "\n";
		if (st == "\n" || st == "" ) continue;
		
		string type = readNextWord(st);
		//cout << type<<endl;
		
		if (type == "input_file"){
			top=readNextWord(st);
			tp =top.substr(0,top.length()-3);
			myfile <<"\n" << tp << ",";
		}
		else if (type == "Timeout")
			myfile << readNextWord(st) << ",";
		else if (type == "Variables")
			myfile << readNextWord(st) << ",";
		else if (type == "Constraints")
			myfile << readNextWord(st) << ",";
		else if (type == "NonZeros")
			myfile << readNextWord(st) << ",";
		else if (type == "Runtime")
			myfile << readNextWord(st) << ",";
		else if (type == "TimedOut")
			myfile << readNextWord(st) << ",";
		else if (type == "SolutionFound")
			myfile << readNextWord(st) << ",";
		else if (type == "ProvedOptimal")
			myfile << readNextWord(st) << ",";
		else if (type == "StatusCode")
			myfile << readNextWord(st) << ",";
		else if (type == "Nodes")
			myfile << readNextWord(st) << ",";
		else if (type == "Objective")
			myfile << readNextWord(st) << ",";
		else if (type == "Iterations")
			myfile << readNextWord(st);
		else if (type == "Solution"){
			out_fl = tp + "_solution.txt";
			
			myfile1.open(out_fl);
			myfile1 << "Solution " + readNextWord(st);
			myfile1.close();
			//cout << "closing file \n";
		}
	//cout << " Still in WHile " + type;	
	}
	myfile.close();
	rfile.close();
	return 0;	
}

