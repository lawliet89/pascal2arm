/*******************************************************************************
 * 
 * Utility functions for the compiler. Not related to Flex or Bison
 * 
 ******************************************************************************/
#ifndef Utility_H
#define Utility_H

#include <ostream>

/*
 * Shortcut macros
 * */
#define OUTPUT (*Flags.Output)


/**
 * 	Command Arguments Parsing
 * */
struct Flags_T{
	//File output stream
	std::ostream *Output;
};	//There will be one object - Flags - defined in utility.cpp

void ParseArg(int argc, char **argv);

#endif /*Utility_H*/