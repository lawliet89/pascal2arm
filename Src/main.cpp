#include <iostream>
#include <signal.h>
#include <fstream>
#include "utility.h"
#include "compiler.h"
#include "op.h"


//Flags - declared in utility.cpp
extern Flags_T Flags;
extern std::stringstream OutputString;		//op.cpp
extern unsigned LexerCharCount, LexerLineCount;

int main(int argc, char **argv){	
	signal(SIGABRT, &HandleAbort);	//Clean up and stuff
	ParseArg(argc, argv);	//Parse command line arguments and set Flags
	
	//Check for integrity of input and output
	if (!INPUT){
		std::cout << "Error reading '" << Flags.InputPath << "'" << std::endl;
		return 1;
	}

	//Parse
	yyparse();
	
	if (Flags.OutputPath.compare("std::cout"))
		Flags.Output = new std::ofstream(Flags.OutputPath.c_str(), std::ios_base::out | std::ios_base::trunc);
	
	if (OUTPUT_FILE.fail()){
		std::cout << "Error writing to '" << Flags.OutputPath << "'" << std::endl;
		return 1;
	}	
	
	/*int lex;
	do{
		lex = yylex();
		std::cout << lex << " " << LexerLineCount << ":" << LexerCharCount << std::endl;
	} while(lex);
	
	std::cout << "Syntax okay" << std::endl;*/
	
	OUTPUT_FILE << OUTPUT.str();
	
	return 0;
}