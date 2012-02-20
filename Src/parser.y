/*
	Naming conventions
	Tokens/Terminals are in CAPS. 
		K_ - keywords
		I_ - predefined identifiers 
		OP_ - Operator Tokens (if operators are single character, will not be defined as a separate enum token)
		V_ - Literal Values tokens from Flex
		VAR_ Variables or constants
		
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
/* Note that V_INT is unsigned and will be turned signed via a non terminal */
%token V_IDENTIFIER V_INT V_REAL V_STRING V_CHAR V_NIL


/* Keywords */
%token K_ARRAY K_BEGIN K_CASE K_CONST K_DO K_DOWNTO K_ELSE
%token K_END K_FILE K_FOR K_FUNCTION K_GOTO K_IF K_LABEL 
%token K_OF K_PACKED K_PROCEDURE K_PROGRAM K_RECORD K_REPEAT
%token K_SET K_THEN K_TO K_TYPE K_UNTIL K_VAR K_WHILE K_WITH

/* Pre-defined identifiers */
%token I_TRUE I_FALSE I_MAXINT
%token I_BOOLEAN I_CHAR I_INTEGER I_REAL I_STRING I_TEXT
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
%token Y_SYNTAX_ERROR Y_FATAL_ERROR Y_EOF
%%

Sentence: Program  Y_EOF { YYACCEPT; }
/* 	| Unit	*/	/* For probable implementation? */
	;

Program: Block '.'
	| ProgramHeader ';' Block '.'
	/* UsesBlock if we are implementing units */
	;
	
/* Program Headers */
ProgramHeader:	K_PROGRAM Identifier 
		| K_PROGRAM Identifier '('')'
		| K_PROGRAM Identifier '(' FileIdentifierList ')'
		;

FileIdentifierList: FileIdentifier ',' FileIdentifierList
		| FileIdentifier
		;

FileIdentifier: I_INPUT
		| I_OUTPUT
		| Identifier
		;

/* Block */	
Block: BlockDeclaration CompoundStatement
	;

BlockDeclaration: BlockLabelDeclaration BlockConstantDeclaration BlockTypeDeclaration BlockVarDeclaration BlockProcFuncDeclaration
		|;

BlockLabelDeclaration: LabelDeclaration
			|;
BlockConstantDeclaration: K_CONST ConstantList
			|;
BlockTypeDeclaration: K_TYPE TypeList
			|;
BlockVarDeclaration: K_VAR VarList
			|;
BlockProcFuncDeclaration: ProcList
			| FuncList
			|;

/* Generic Stuff */
Identifier: V_IDENTIFIER
	;

IdentifierList: Identifier ',' IdentifierList
		| Identifier
		;

LabelDeclaration: K_LABEL V_INT ',' LabelDeclaration
		| K_LABEL V_INT ';'
		;

ConstantList: 	ConstantList ConstantDeclaration
		| ConstantDeclaration;
		
		
ConstantDeclaration: Identifier '=' Expression ';' 
		;

TypeList:	TypeList TypeDeclaration
		| TypeDeclaration
		;
		
TypeDeclaration: Identifier '=' Type ';' 
		;

VarList:	VarList VarDeclaration
		| VarDeclaration
		;

VarDeclaration:	Identifier ':' Type '=' Expression ';'
		| Identifier ':' Type ';'
		;

ProcList: ProcList ProcDeclaration
	| ProcDeclaration
	;

FuncList: FuncList FuncDeclaration
	| FuncDeclaration
	;

/* Types */
Type: SimpleType
	| StringType
/*	| StructuredType 
	| PointerType   */
	| TypeIdentifier
	;
	
SimpleType: OrdinalType
	| RealType
	;

OrdinalType: I_INTEGER
	| I_CHAR
	| I_BOOLEAN
	| EnumType
	| SubrangeType
	;

EnumType:'(' EnumTypeList ')'
	;

EnumTypeList: EnumTypeList ',' IdentifierList
	/* | EnumTypeList ',' Identifier OP_ASSIGNMENT Expression */
	| IdentifierList
	/* | Identifier OP_ASSIGNMENT Expression */
	;

SubrangeType: SubrangeValue OP_DOTDOT SubrangeValue
		;
SubrangeValue: Identifier
		| V_INT
		| '-' V_INT
		| '+' V_INT
		;

RealType: I_REAL;

StringType: I_STRING
	| I_STRING '[' V_INT ']'
	;
	
TypeIdentifier: Identifier;

/* Values */
L_Int: '+' V_INT
	| '-' V_INT
	| V_INT
	;

/*L_Real: '+' V_REAL
	| '-' V_REAL
	| V_REAL
	;
;*/	
Constant: Identifier
	| L_Int
	;

/* Prodcedures and functions */
FormalParamList: '(' FormalParam ')'
		;

FormalParam:  FormalParam ';' ParamDeclaration 
	| ParamDeclaration
	;

ParamDeclaration: ValueParam
	/*	| VarParam
		| OutParam
		| ConstantParam */
		;

ValueParam:	IdentifierList ':' Type
	/*	| Identifier ':' Type '=' DefaultParamValue  */
		;

SubroutineBlock: Block
		| I_FORWARD
		;

ProcDeclaration: ProcHeader ';' SubroutineBlock ';'
		;

ProcHeader: K_PROCEDURE Identifier FormalParamList
	;

FuncDeclaration: FuncHeader ';' SubroutineBlock ';'
		;

FuncHeader: K_FUNCTION Identifier FormalParamList ':' Type
	;

/* Expression */
Expression: SimpleExpression SimpleOp Expression
	| SimpleExpression;

SimpleOp: '*'
	| '<'
	| OP_LE
	| '>'
	| OP_GE
	| '='
	| OP_NOTEQUAL
	| OP_IN
	; 

SimpleExpression: Term TermOP SimpleExpression
		| Term
		;

TermOP: '+'
	| '-'
	| OP_OR
	/* XOR ? */
	;

Term: Factor FactorOP Term
	| Factor
	;

FactorOP: '*'
	| '/'
	| OP_DIV
	| OP_MOD
	| OP_AND
	;

/* Factor can be shifted into Identifier in multiple paths. Need to optimise */
Factor: '(' Expression ')'
	| VarRef
	| FuncCall
	| UnsignedConstant
	| OP_NOT Factor
	| '+' Factor
	| '-' Factor
	/* Set, value typecast, address factor ?? */
	;
VarRef: Identifier
	;

UnsignedConstant: V_REAL
		| V_INT
		| V_STRING
		| V_CHAR
		/* | Identifier */
		| V_NIL
		;

FuncCall: Identifier '(' ActualParamList ')'
	/* | Identifier */
	;

ActualParamList: ActualParamList ',' Expression
		| Expression
		|
		;

/* Statements */
CompoundStatement: K_BEGIN StatementList K_END
		;

StatementList: StatementList ';' Statement
		| Statement ';'
		| Statement
		;

StatementLabel: V_INT ':' 
		| ;

Statement: StatementLabel SimpleStatement
	| StatementLabel StructuredStatement
	;

SimpleStatement: AssignmentStatement
		| ProcedureStatement
		| GotoStatement
		;

AssignmentStatement: Identifier OP_ASSIGNMENT Expression
		/* += -= /= *= */
		;

ProcedureStatement: Identifier '(' ActualParamList ')'
		| Identifier
		;

GotoStatement: K_GOTO V_INT
		;

StructuredStatement: CompoundStatement
		| ConditionalStatement
		| RepetitiveStatement
		/* | WithStatement */
		;

ConditionalStatement: CaseStatement
		| IfStatement
		;

RepetitiveStatement: ForStatement
		| RepeatStatement
		| WhileStatement
		;

CaseStatement: K_CASE Expression K_OF CaseBody K_END;

CaseBody: Cases ElsePart ';'
	| Cases ElsePart 
	| Cases  ';'
	| Cases
	;

Cases: Cases ';' Case ':' Statement
	|  Case   ':' Statement
	;

Case: Case ',' CaseConstant 
	|  CaseConstant
	;

CaseConstant: Constant
	| Constant OP_DOTDOT Constant
	;
ElsePart: K_ELSE Statement
	;

IfStatement: K_IF Expression K_THEN Statement ElsePart
	| K_IF Expression K_THEN Statement
	;

ForStatement: K_FOR Identifier OP_ASSIGNMENT Expression K_TO Expression K_DO Statement
	;

RepeatStatement: K_REPEAT StatementList K_UNTIL Expression
		;

WhileStatement: K_WHILE Expression K_DO Statement
		;

%%

//If this is called then we have encountered an unknown parse error
void yyerror(const char * msg){
	HandleError("Unknown parse error.", E_PARSE, E_FATAL, LexerLineCount, LexerCharCount);
}