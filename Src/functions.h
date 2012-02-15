/*******************************************************************************
 * 
 * Utility functions for Flex and Bison
 * 
 ******************************************************************************/

#ifndef FunctionsH
#define FunctionsH

#define YY_DECL int yylex(void)
#define YYSTYPE int

/*
 * Function Prototypes for Flex and Bison
 */
int yyparse();	//Bison
YY_DECL; 	//Flex

/*
	Bison Functions
*/

void yyerror(const char *msg);

/*
 * 
 * 	Flex Functions - defined in lexer.l
 * 
 */

//Initialise Lexer
void LexerInit();

void LexerConsumeComments(const char *delimiter);
void LexerConsumeInvalid();

//Inline functions
inline void LexerAddCharCount();
#endif