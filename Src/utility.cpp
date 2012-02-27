#include "utility.h"
#include <fstream>
#include <streambuf>


//http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
std::string ReadFile(const char* path){
	//Catch any exceptions yourself!
	std::ifstream file(path);
	std::string str;
	
	//Reserve memory for string based on length of file
	file.seekg(0, std::ios::end);
	str.reserve(file.tellg());
	file.seekg(0, std::ios::beg);
	
	//Create iterator for the file based on its current position (which is at the beginning)
	//And end off with an iterator for the EOF (which the default constructor creates)
	str.assign((std::istreambuf_iterator<char>(file)),
		   std::istreambuf_iterator<char>());
	
	return str;
}