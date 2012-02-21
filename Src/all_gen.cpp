#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, const char **argv){
	if (argc < 2)
		return 1;
	string list;
	ofstream output(argv[1], ios_base::out | ios_base::trunc);
	
	output << "#ifndef AllH\n#define AllH\n";
	
	while(!cin.eof()){
		getline(cin, list);
		
		if (!list.empty())
			output << "#include \"../" << list << "\"\n";
	}
	
	output << "#endif";
	
	return 0;
}