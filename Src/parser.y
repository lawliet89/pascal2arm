/*
	Naming conventions
	Tokens/Terminals are in CAPS. 
		K_ - keywords
		I_ - predefined identifiers 
		OP_ - Operator Tokens (if operators are single character, will not be defined as a separate enum token)
		V_ - Values tokens from Flex
		
		Y_ - Internal Tokens
		
	Non Terminals are CamelCased.
		L_ - Internal literals
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
%token V_IDENTIFIER V_INT V_REAL V_STRING V_BOOLEAN V_NIL

/* Literal Tokens */
/* After determining signed or unsigned etc.*/
%token L_Int L_Real


/* Keywords */
%token K_ARRAY K_BEGIN K_CASE K_CONST K_DO K_DOWNTO K_ELSE
%token K_END K_FILE K_FOR K_FUNCTION K_GOTO K_IF K_LABEL 
%token K_OF K_PACKED K_PROCEDURE K_PROGRAM K_RECORD K_REPEAT
%token K_SET K_THEN K_TO K_TYPE K_UNTIL K_VAR K_WHILE K_WITH

/* Pre-defined identifiers */
%token I_TRUE I_FALSE I_MAXINT
%token I_BOOLEAN I_CHAR I_INTEGER I_REAL I_TEXT
%token I_INPUT I_OUTPUT
%token I_FORWARD

/* Operators
	Precedence for expression operators
*/
/* Unary Ops, includes +- but these are handled separately */
%right OP_NOT
/* Multiplicative */
%left '*' '/' OP_DIV OP_MOD OP_AND
/* Additive */
%left '+' '-' OP_OR
/* Relational */
%left '=' OP_GE OP_LE OP_NOTEQUAL '<' '>' OP_IN

%token OP_DOTDOT OP_STARSTAR OP_UPARROW OP_ASSIGNMENT


/* Internal Use Tokens */
%token Y_SYNTAX_ERROR Y_FATAL_ERROR
%%

Sentence: Program 
/* 	| Unit	*/	/* For probable implementation? */
	;

Program: Block '.'
	| ProgramHeader ';' Block '.'
	/* Uses block if we are implementing units */
	;
	
/* Program Headers */
ProgramHeader:	K_PROGRAM V_IDENTIFIER 
		| K_PROGRAM V_IDENTIFIER '('')'
		| K_PROGRAM V_IDENTIFIER '(' IdentifierList ')'
		;

/* Block */	
Block: BlockDeclaration CompoudStatement
	;

BlockDeclaration: BlockLabelDeclaration
		|;

BlockLabelDeclaration: LabelDeclaration BlockConstantDeclaration;
BlockConstantDeclaration: ConstantDeclaration BlockTypeDeclaration;
BlockTypeDeclaration: TypeDeclaration BlockVarDeclaration;
BlockVarDeclaration: VarDeclaration BlockProcFuncDeclaration;
BlockProcFuncDeclaration: ProcFuncDeclaration
			|;

/* Generic Stuff */
IdentifierList: V_IDENTIFIER ',' IdentifierList
		| V_IDENTIFIER
		;

LabelDeclaration: K_LABEL V_INT ',' LabelDeclaration
		| K_LABEL V_INT ';'
		;
%%

//If this is called then we have encountered an unknown parse error
void yyerror(const char * msg){
	HandleError("Unknown parse error.", E_PARSE, E_FATAL, LexerLineCount, LexerCharCount);
}