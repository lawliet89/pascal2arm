/*
	Naming conventions
	Tokens/Terminals are in CAPS. 
		K_ - keywords
		I_ - predefined identifiers 
		OP_ - Operator Tokens (if operators are single character, will not be defined as a separate enum token)
		V_ - Values tokens
		
		Y_ - Internal Tokens
		
	Non Terminals are CamelCased.
*/

%{
#include <iostream>
#define IN_BISON
#include "../functions.h"
#include "../utility.h"

extern Flags_T Flags;		//In utility.cpp
extern YYSTYPE CurrentToken;	//In lexer.l
extern unsigned LexerCharCount, LexerLineCount;		//In lexer.l

void yyerror(const char *msg);
%}
/* Value Tokens */
%token V_IDENTIFIER V_INT V_REAL V_STRING

/* Characters/operators tokens */


/* Keywords */
%token K_ARRAY K_BEGIN K_CASE K_CONST K_DO K_DOWNTO K_ELSE
%token K_END K_FILE K_FOR K_FUNCTION K_GOTO K_IF K_LABEL 
%token K_NIL K_OF K_PACKED K_PROCEDURE K_PROGRAM K_RECORD K_REPEAT
%token K_SET K_THEN K_TO K_TYPE K_UNTIL K_VAR K_WHILE K_WITH

/* Pre-defined identifiers */
%token I_TRUE I_FALSE I_MAXINT
%token I_BOOLEAN I_CHAR I_INTEGER I_REAL I_TEXT
%token I_INPUT I_OUTPUT
%token I_FORWARD

/* Operators
	Precedence for expression operators
*/
%right OP_NOT
%left '*' '/' OP_DIV OP_MOD OP_AND
%left '+' '-' OP_OR
%left '=' OP_GE OP_LE OP_NOTEQUAL '<' '>' OP_IN

%token OP_DOTDOT OP_STARSTAR OP_UPARROW OP_ASSIGNMENT


/* Internal Use Tokens */
%token Y_SYNTAX_ERROR Y_FATAL_ERROR
%%

Program: K_PROGRAM;   

%%

//If this is called then we have encountered an unknown parse error
void yyerror(const char * msg){
	HandleError("Unknown parse error.", E_PARSE, E_FATAL, LexerLineCount, LexerCharCount);
}