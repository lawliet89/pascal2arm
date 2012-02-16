/*
	Naming conventions
	Tokens/Terminals are in CAPS. 
		K_ - keywords
		I_ - predefined identifiers
		C_ - characters and operators
		T_ - Compound tokens 
		
		Y_ - Internal Tokens
	Non Terminals are CamelCased.
*/

%{
#include <iostream>
#define IN_BISON
#include "functions.h"
#include "utility.h"

extern Flags_T Flags;		//In utility.cpp
extern YYSTYPE CurrentToken;	//In lexer.l

%}
/* Compound Tokens */
%token T_IDENTIFIER

/* Characters/operators tokens */


/* Keywords */
%token K_AND K_ARRAY K_BEGIN K_CASE K_CONST K_DIV K_DO K_DOWNTO K_ELSE
%token K_END K_FILE K_FOR K_FUNCTION K_GOTO K_IF K_IN K_LABEL K_MOD
%token K_NIL K_NOT K_OF K_OR K_PACKED K_PROCEDURE K_PROGRAM K_RECORD K_REPEAT
%token K_SET K_THEN K_TO K_TYPE K_UNTIL K_VAR K_WHILE K_WITH

/* Pre-defined identifiers */
%token I_TRUE I_FALSE I_MAXINT
%token I_BOOLEAN I_CHAR I_INTEGER I_REAL I_TEXT
%token I_INPUT I_OUTPUT
%token I_FORWARD

/* Internal Use Tokens */
%token Y_SYNTAX_ERROR
%%

Program: K_PROGRAM;   

%%

void yyerror(const char * msg){
	std::cout << msg << std::endl;
}