/*******************************************************************************
 * 
 * Utility functions for the compiler. Not for the internal workings of Bison or Flex
 * 
 ******************************************************************************/
#ifndef Utility_H
#define Utility_H

#include <ostream>
#include <string>
#include <sstream>
#include <cstdio>

/*
 * Shortcut macros
 * */
#define OUTPUT (*Flags.Output)
#define INPUT Flags.Input

/**
 * 	Command Arguments Parsing
 * */
struct Flags_T{
	//File output stream & name
	std::ostream *Output;
	std::string OutputPath;
	
	//File input stream and name
	FILE *Input;		//Required by yylex() to be FILE *
	std::string InputPath;
	
	bool ShowHints;
	
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

/**
 * Utility functions
 * 
 * */

template<typename T> std::string ToString(T input){
	std::stringstream output;
	output << input;
	
	return output.str();
}

template<typename T> T FromString(std::string input){
	T output;
	std::stringstream stream(input);
	stream >> output;
	
	return output;
}

#endif /*Utility_H*/