#include <iostream>
#include "utility.h"

//Flags - declared in utility.cpp
extern Flags_T Flags;

int main(int argc, char **argv){
	ParseArg(argc, argv);	//Parse command line arguments and set Flags
	
	OUTPUT << "Test!" << std::endl;
}