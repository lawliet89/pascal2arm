/*******************************************************************************
 * 
 * Operation functions for the compiler. Not for the internal workings of Bison or Flex
 * 
 ******************************************************************************/
#ifndef Op_H
#define Op_H

#include <ostream>
#include <string>
#include <sstream>
#include <cstdio>

/*
 * Shortcut macros
 * */
#define OUTPUT_FILE (*Flags.Output)
#define OUTPUT OutputString
#define INPUT Flags.Input

/**
 * 	Global flags
 * */
struct Flags_T{
	//File output stream & name
	std::ostream *Output;
	std::string OutputPath;
	
	//File input stream and name
	FILE *Input;		//Required by yylex() to be FILE *
	std::string InputPath;
	
	bool ShowHints;
	
	//Skeleton Assembly Files location
	std::string AsmHeaderPath;
	std::string AsmStdLibPath;	//Standard libary ASM file
	
};	//There will be one object - Flags - defined in utility.cpp

//Parse arguments and initialise flags
void ParseArg(int argc, char **argv);

/**
 * 	Compile Error handling
 * 
 * */

enum ErrorClass_T{
	E_SYNTAX, E_PARSE, E_GENERIC		//Where Error is a generic
};

enum ErrorLevel_T{
	E_NOTICE, E_WARNING, E_ERROR, E_FATAL
};

void HandleError(const char* message, ErrorClass_T ErrorClass=E_GENERIC, ErrorLevel_T level=E_NOTICE, unsigned line = 0, unsigned chararacter=0);

/**
 * Handle SIGABRT
 * 
 * */

void HandleAbort(int param);



#endif /*Op_H*/