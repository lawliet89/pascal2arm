/*******************************************************************************
 * 
 * Define some basic macros and functions for bison and flex interface
 * 
 ******************************************************************************/

#ifndef CompilerH
#define CompilerH

#include <memory>
#define YYSTYPE std::shared_ptr<Token>
#define YY_DECL int yylex(void)

/*
 * Function Prototypes for Flex and Bison for everyone
 */
int yyparse();	//Bison
YY_DECL; 	//Flex

//Initialise Lexer
void LexerInit();	//Defined in lexer.l


#endif /* CompilerH */
