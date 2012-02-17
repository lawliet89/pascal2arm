#include <iostream>
#include <signal.h>
#include "utility.h"
#include "functions.h"


//Flags - declared in utility.cpp
extern Flags_T Flags;
extern unsigned LexerCharCount, LexerLineCount;

int main(int argc, char **argv){
	signal(SIGABRT, &HandleAbort);	//Clean up and stuff
	ParseArg(argc, argv);	//Parse command line arguments and set Flags
	
	//Check for integrity of input and output
	if (!INPUT){
		std::cout << "Error reading '" << Flags.InputPath << "'" << std::endl;
		return 1;
	}
	if (OUTPUT.fail()){
		std::cout << "Error writing to '" << Flags.OutputPath << "'" << std::endl;
		return 1;
	}
	
	//Initialise Lexer
	LexerInit();
	
	yyparse();
	
	/*int lex;
	do{
		lex = yylex();
		std::cout << lex << " " << LexerLineCount << ":" << LexerCharCount << std::endl;
	} while(lex);
	
	std::cout << "Syntax okay" << std::endl;*/
	
	return 0;
}