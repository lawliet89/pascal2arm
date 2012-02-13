/*******************************************************************************
 * 
 * Utility functions for the compiler. Not related to Flex or Bison
 * 
 ******************************************************************************/
#ifndef Utility_H
#define Utility_H

#include <ostream>
#include <string>
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
};	//There will be one object - Flags - defined in utility.cpp

//Parse arguments and initialise flags
void ParseArg(int argc, char **argv);

#endif /*Utility_H*/