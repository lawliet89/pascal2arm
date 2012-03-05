/*
	Naming conventions
	Tokens/Terminals are in CAPS. 
		K_ - keywords
		I_ - predefined identifiers 
		OP_ - Operator Tokens (if operators are single character, will not be defined as a separate enum token)
		V_ - Literal Values tokens from Flex
		VAR_ Variables or constants
		
		Y_ - Internal Tokens

		A_ - Compiler Action
		
	Non Terminals are CamelCased.
		L_ - Internal literals
*/

%code top{
#include <iostream>
#include <sstream>
#include <new>
#include <memory>
#define IN_BISON
#include "../compiler.h"	//See Prologue alternatives: http://www.gnu.org/software/bison/manual/bison.html#Prologue-Alternatives
#include "lexer.h"
#include "../asm.h"
#include "../utility.h"
#include "../op.h"
#include "../define.h"
#include "../token.h"
#include "all.h"	//All specialisations
#include "../define.h"
}

%code{
#define CurrentToken yylval

extern Flags_T Flags;		//In op.cpp
extern unsigned LexerCharCount, LexerLineCount;		//In lexer.l
extern std::stringstream OutputString;		//op.cpp

//ADT for program
AsmFile Program;
bool ParseError;

//Parser functions declaration
void yyerror(const char *msg);

}

/*
	Note: Might need to use code provides eventually.
*/

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

%start Sentence

/* Debug Options */
%error-verbose
/* %define lr.type ielr */  	/* Needs 2.5 */
//%define parse.lac full 
/* %define lr.default-reductions consistent */

%initial-action
     {
	//Initialise Lexer
	LexerInit();
     };

%%

Sentence: Program Y_EOF { CurrentToken.reset(); 
			if (ParseError){
				HandleError("There are parse error(s). Compilation cannot proceed.", E_PARSE, E_FATAL);
			}
			Program.GenerateCode(OUTPUT);
			YYACCEPT; }
/* 	| Unit	*/	/* For probable implementation? */
	;

Program: Block '.'
	| ProgramHeader ';' Block '.' 
	;
	
/* Program Headers */
ProgramHeader:	K_PROGRAM Identifier 	/*throw away */
		| K_PROGRAM Identifier '('')' /*throw away */
		| K_PROGRAM Identifier '(' FileIdentifierList ')' /*throw away */
		;

FileIdentifierList: FileIdentifierList ',' FileIdentifier
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
		;

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
Identifier: V_IDENTIFIER {
				$$.reset(new Token(yylval-> GetStrValue(), Identifier));
			}
	;

IdentifierList: IdentifierList ',' Identifier
			{
				$$ = $1;
				try{
					dynamic_cast<Token_IDList*>($$.get()) -> AddID($3);
				}
				catch (AsmCode e){
					if (e == SymbolExists){
						std::stringstream msg;
						msg << "Identifier '" << $3 -> GetStrValue();
						msg << "' has already been declared previously.";	
						
						if (Flags.ShowHints){
							msg << " It was probably previously declared on the same line.";
						}
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, LexerLineCount, LexerCharCount);
					}
				}
			}
		| Identifier {
				$$.reset(new Token_IDList($1));
				}
		;

LabelDeclaration: LabelDeclaration ',' K_LABEL V_INT
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

VarDeclaration:	IdentifierList ':' Type '=' Expression ';'	/* Add to Data declaration? */
		| IdentifierList ':' Type ';' {
			Program.CreateVarSymbolsFromList(std::dynamic_pointer_cast<Token_IDList>($1),std::dynamic_pointer_cast<Token_Type>($3));
		};

ProcList: ProcList ProcDeclaration
	| ProcDeclaration
	;

FuncList: FuncList FuncDeclaration
	| FuncDeclaration
	;

/* Types */
Type: SimpleType { $$ = $1; }
	| StringType 
	| StructuredType 
	| PointerType   
	| TypeIdentifier
	;
	
SimpleType: OrdinalType { $$ = $1; }
	| RealType { $$ = $1; }
	;

OrdinalType: I_INTEGER	{ $$ = Program.GetTypeSymbol("integer").first->GetValue(); }
	| I_CHAR { $$ = Program.GetTypeSymbol("char").first->GetValue(); }
	| I_BOOLEAN { $$ = Program.GetTypeSymbol("boolean").first->GetValue(); }
	| EnumType { $$ = Program.GetTypeSymbol("enum").first->GetValue(); } /* TODO more handling */
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
		| Signed_Int
		;

RealType: I_REAL { $$ = Program.GetTypeSymbol("real").first->GetValue(); }
		;

StringType: I_STRING { $$ = Program.GetTypeSymbol("string").first->GetValue(); }
	| I_STRING '[' V_INT ']' //TODO
	;
	
TypeIdentifier: Identifier {
	//Possibility that type doesn't exist
	try{
		std::pair<std::shared_ptr<Symbol>, AsmCode> result = Program.GetTypeSymbol($1 -> GetStrValue());
		$$ = result.first->GetValue(); 
	}
	catch (AsmCode e){
		std::stringstream msg;
		msg << "'" << $1 -> GetStrValue() << "' ";

		msg << "is an unknown type.";
		if (Flags.ShowHints)
			msg << " Has the type been declared before?";
		HandleError(msg.str().c_str(), E_PARSE, E_ERROR, LexerLineCount, LexerCharCount);
	}
};

StructuredType: ArrayType
		| RecordType
		| SetType
		| FileType
		;
ArrayType: Packness K_ARRAY ArraySize K_OF Type
	;

Packness: K_PACKED		/* Maybe not implementing at all */
		| 
		;

ArraySize: '[' SubRangeList ']'
	|
	;

SubRangeList: SubRangeList ',' SubrangeType
		| SubrangeType
		;

RecordType: Packness K_RECORD FieldList K_END
		;

FieldList: VarList;

SetType: K_SET K_OF OrdinalType;

FileType: K_FILE 
	| K_FILE K_OF FileTypes
	;

FileTypes: Type
	| I_TEXT
	;

PointerType: '^' Type
	;

/* Values */
Signed_Int: '+' V_INT {  
			$$.reset(new Token_Int(GetValue<long>(yylval), Signed_Int));
		}
	| '-' V_INT { 
			$$.reset(new Token_Int(GetValue<long>(yylval) * -1, Signed_Int));   
		}
	;
	
Signed_Real: '+' V_REAL {
			$$.reset(new Token_Int(GetValue<double>(yylval), Signed_Real));
			}
	| '-' V_REAL{
			$$.reset(new Token_Int(GetValue<double>(yylval)*-1, Signed_Real));
			}
	;
;
Constant: Identifier
	| V_INT
	| Signed_Int
	;

/* Prodcedures and functions */
FormalParamList: '(' FormalParam ')'
				|
		;

FormalParam:  FormalParam ';' ParamDeclaration 
	| ParamDeclaration
	;

ParamDeclaration: ValueParam
		| VarParam
	/*	| OutParam
		| ConstantParam */
		;

ValueParam:	IdentifierList ':' Type
	/*	| Identifier ':' Type '=' DefaultParamValue  */
		;
VarParam: K_VAR IdentifierList ':' Type
		;
		
SubroutineBlock: Block
		| I_FORWARD
		;

ProcDeclaration: ProcHeader ';' SubroutineBlock ';'{
				//Pop Block
				Program.PopBlock();
			}
		;

ProcHeader: K_PROCEDURE Identifier FormalParamList{
			//Create the token for the procedure
			//TODO
			Program.CreateProcSymbol($2 -> GetStrValue());
		};

FuncDeclaration: FuncHeader ';' SubroutineBlock ';'
		;

FuncHeader: K_FUNCTION Identifier FormalParamList ':' Type
			;

/* Expression */
Expression:  Expression SimpleOp SimpleExpression
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

SimpleExpression: SimpleExpression TermOP Term
		| Term
		;

TermOP: '+'
	| '-'
	| OP_OR
	/* XOR ? */
	;

Term:  Term FactorOP Factor
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
	| SignedConstant
	| SetConstructors
	/* Set, value typecast, address factor ?? */
	;
VarRef: SimpleVarReference VarQualifier
	| SimpleVarReference
	;

SimpleVarReference: Identifier
		;

VarQualifier: FieldSpecifier
		| Index
		;

FieldSpecifier: '.' Identifier
		;

Index: '[' IndexList ']'
	;

IndexList: IndexList ',' Expression
	| Expression
	;

UnsignedConstant: V_REAL	{ $$.reset(new Token_Real(GetValue<double>(yylval), V_Real)); }
		| V_INT		{ $$.reset(new Token_Int(GetValue<int>(yylval), V_Int)); }
		| V_STRING	{ $$.reset(new Token(yylval -> GetStrValue(), V_String)); }
		| V_CHAR	{ $$.reset(new Token(yylval -> GetStrValue(), V_Character)); }
		/* | Identifier */
		| V_NIL		{ $$.reset(new Token("NIL", V_Nil)); }
		| I_TRUE	{ $$.reset(new Token("TRUE", I_True)); }
		| I_FALSE	{ $$.reset(new Token("FALSE", I_False)); }
		| I_MAXINT	{ $$.reset(new Token_Int(2147483647, V_Int)); }
		;
SignedConstant: Signed_Int
		| Signed_Real
		;
		
FuncCall: Identifier '(' ActualParamList ')'
	/* | Identifier */
	;

ActualParamList: ActualParamList ',' Expression
		| Expression
		|
		;
		
SetConstructors: '[' SetGroupList ']'
		| '[' ']'
		;

SetGroupList: SetGroupList ',' SetGroup
		| SetGroup
		;
		
SetGroup: Expression OP_DOTDOT Expression
	| Expression
	;

/* Statements */
CompoundStatement: K_BEGIN StatementList K_END
		;

StatementList:   StatementList ';' Statement
		| Statement
		;

StatementLabel: V_INT ':' {
			HandleError("Labels are unsupported and are discarded.", E_GENERIC, E_WARNING,LexerLineCount, LexerCharCount); 
		}
		| ;

Statement: StatementLabel SimpleStatement
	| StatementLabel StructuredStatement
	| /* A statement can be empty - this is to allow for that pesky optional ';' at the end of the last statement */
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
	std::stringstream text;
	text << "Unknown parse error: " << msg;
	//text << "(" << yylloc.first_line << "-" << yylloc.last_line << ":" << yylloc.first_column;
	//text << "-" << yylloc.last_column << ")";
	HandleError(text.str().c_str(), E_PARSE, E_FATAL, LexerLineCount, LexerCharCount);
}
