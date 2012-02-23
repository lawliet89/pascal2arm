/*******************************************************************************
 * 
 * Utility functions for Flex and Bison
 * 
 * When used in Bison/Flex provides the definition of Macros and utility functions 
 * 
 * Otherwise, it give the prototype to use the lexer and parser
 * 
 ******************************************************************************/

#ifndef FunctionsH
#define FunctionsH

// FLEX and Bison Specific
//#if defined IN_BISON || defined IN_FLEX
#include <memory>
#include "token.h"	//Declares YYTYPE and includes "parser.h"
//#endif

//Bison specific
#ifdef IN_BISON
#include "Gen/lexer.h"
#endif

#ifdef IN_FLEX
#include "Gen/parser.h"
#endif

#define YY_DECL int yylex(void)

/*
 * Function Prototypes for Flex and Bison for everyone
 */
int yyparse();	//Bison
YY_DECL; 	//Flex

//Initialise Lexer
void LexerInit();


#endif /* FunctionsH */