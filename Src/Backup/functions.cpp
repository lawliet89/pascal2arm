#include <iostream>
#include <set>
#include <string>
#include <cstring>
#include "functions.h"
#include "utility.h"
#include "lexer.h"
#include "parser.h"


using namespace std;

extern Flags_T Flags;			//In utility.cpp
extern unsigned LexerCharCount, LexerLineCount;		//In lexer.l

/*
 * 	Bison Functions
 * 					*/


/*
 * 	Flex Functions
 * 
 * */




/*
 * 	Useful data constructs. Use extern if you want to access 
 * 
 * 
 * */
//A set for easily searching for keywords. Use set::count to find.
