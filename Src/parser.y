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
%left '+' '-' OP_OR OP_XOR
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
	//yydebug = 1;
     };

%%

Sentence: Program Y_EOF { CurrentToken.reset(); 
			if (ParseError){
				HandleError("There are parse error(s). Compilation cannot proceed.", E_PARSE, E_FATAL);
			}
			OUTPUT << Program.GenerateCode();
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

BlockLabelDeclaration: LabelDeclaration { HandleError("Labels are unsupported and are discarded.", E_GENERIC, E_WARNING,LexerLineCount, LexerCharCount); }
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

LabelDeclaration: LabelDeclaration ',' K_LABEL V_INT	//Ignored
		| K_LABEL V_INT ';' //Ignored 
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

VarDeclaration:	IdentifierList ':' Type '=' Expression ';'{
			//Handle expression
			//TODO
			
			Program.CreateVarSymbolsFromList(std::dynamic_pointer_cast<Token_IDList>($1),std::dynamic_pointer_cast<Token_Type>($3));
		}
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
			$$.reset(new Token_Int(GetValue<int>(yylval), V_Int));
		}
	| '-' V_INT { 
			$$.reset(new Token_Int(GetValue<int>(yylval) * -1, V_Int));   
		}
	;
	
Signed_Real: '+' V_REAL {
			$$.reset(new Token_Int(GetValue<double>(yylval), V_Real));
			}
	| '-' V_REAL{
			$$.reset(new Token_Int(GetValue<double>(yylval)*-1, V_Real));
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
Expression:  Expression SimpleOp SimpleExpression {
			std::shared_ptr<Token_Expression> LHS(std::dynamic_pointer_cast<Token_Expression>($1));
			std::shared_ptr<Token_SimExpression> RHS(std::dynamic_pointer_cast<Token_SimExpression>($3));
			//Operator
			Op_T Op = (Op_T) GetValue<int>($2);
			try{
				$$.reset(new Token_Expression(RHS, Op, LHS));
			}
			catch (AsmCode e){
				std::stringstream msg;
				if (e == TypeIncompatible){
					msg << "Incompatible Types: left hand side of expression \n\thas type '" << LHS -> GetType() -> TypeToString();
					msg << "' and right hand side of expression has type '" << RHS -> GetType() -> TypeToString() << "'"; 
					
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $3 -> GetLine(), $3 -> GetColumn());
				}
				else if (e == OperatorIncompatible){
					msg <<	"Operator is incompatible with type '"	<< LHS -> GetType() -> TypeToString() << "'";	//TODO Operator string
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
				}
				else{
					msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
				}
				YYERROR;
			}
		}
	| SimpleExpression { $$.reset(new Token_Expression(std::dynamic_pointer_cast<Token_SimExpression>($1)));}
	;

SimpleOp: '<'
	| OP_LE
	| '>'
	| OP_GE
	| '='
	| OP_NOTEQUAL
	| OP_IN
	; 

SimpleExpression: SimpleExpression TermOP Term {
			std::shared_ptr<Token_SimExpression> LHS(std::dynamic_pointer_cast<Token_SimExpression>($1));
			std::shared_ptr<Token_Term> RHS(std::dynamic_pointer_cast<Token_Term>($3));
			//Operator
			Op_T Op = (Op_T) GetValue<int>($2);
			try{
				$$.reset(new Token_SimExpression(RHS, Op, LHS));
			}
			catch (AsmCode e){
				std::stringstream msg;
				if (e == TypeIncompatible){
					msg << "Incompatible Types: left hand side of simple expression \n\thas type '" << LHS -> GetType() -> TypeToString();
					msg << "' and right hand side of simple expression has type '" << RHS -> GetType() -> TypeToString() << "'"; 
					
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $3 -> GetLine(), $3 -> GetColumn());
				}
				else if (e == OperatorIncompatible){
					msg <<	"Operator is incompatible with type '"	<< LHS -> GetType() -> TypeToString() << "'";	//TODO Operator string
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
				}
				else{
					msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
				}
				YYERROR;
			}
		}
		| Term { $$.reset(new Token_SimExpression(std::dynamic_pointer_cast<Token_Term>($1)));	}
		;

TermOP: '+' { $$.reset(new Token_Int((int) Op_T::Add, T_Type::Operator)); }
	| '-' { $$.reset(new Token_Int((int) Op_T::Subtract, T_Type::Operator)); }
	| OP_OR { $$.reset(new Token_Int((int) Op_T::Or, T_Type::Operator)); }
	| OP_XOR { $$.reset(new Token_Int((int) Op_T::Xor, T_Type::Operator)); }
	;

Term:  Term FactorOP Factor{
			std::shared_ptr<Token_Term> LHS(std::dynamic_pointer_cast<Token_Term>($1));
			std::shared_ptr<Token_Factor> RHS(std::dynamic_pointer_cast<Token_Factor>($3));
			//Operator
			Op_T Op = (Op_T) GetValue<int>($2);
			try{
				$$.reset(new Token_Term(RHS, Op, LHS));
			}
			catch (AsmCode e){
				std::stringstream msg;
				if (e == TypeIncompatible){
					msg << "Incompatible Types: left hand side of term has type '" << LHS -> GetType() -> TypeToString();
					msg << "'\n\tand right hand side of term has type '" << RHS -> GetType() -> TypeToString() << "'"; 
					
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $3 -> GetLine(), $3 -> GetColumn());
				}
				else if (e == OperatorIncompatible){
					msg <<	"Operator is incompatible with type '"	<< LHS -> GetType() -> TypeToString() << "'";	//TODO Operator string
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
				}
				else{
					msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
				}
				YYERROR;
			}
		}
	| Factor {
			$$.reset(new Token_Term(std::dynamic_pointer_cast<Token_Factor>($1)));
		}
	;

FactorOP: '*' { $$.reset(new Token_Int((int) Op_T::Multiply, T_Type::Operator)); }
	| '/' { $$.reset(new Token_Int((int) Op_T::Divide, T_Type::Operator)); }
	| OP_DIV { $$.reset(new Token_Int((int) Op_T::Div, T_Type::Operator)); }
	| OP_MOD { $$.reset(new Token_Int((int) Op_T::Mod, T_Type::Operator)); }
	| OP_AND { $$.reset(new Token_Int((int) Op_T::And, T_Type::Operator)); }
	;

/* Factor can be shifted into Identifier in multiple paths. Such paths have been commented out */
Factor: '(' Expression ')' {
						$$.reset(new Token_Factor(Token_Factor::Expression, $2)); 
					}

	| VarRef		{
						//TODO with qualifier
					}

	| FuncCall		//TODO
	| UnsignedConstant { 
				std::shared_ptr<Token_Type> FactorType;
				//Determine type
				switch($1 -> GetTokenType()){
					case V_Character:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("char").first->GetValue());
						break;
					case V_Int:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("integer").first->GetValue());
						break;
					case V_Real:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("real").first->GetValue());
						break;
					case V_Boolean:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("boolean").first->GetValue());
						break;	
					case V_String:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("string").first->GetValue());
						break;	
					//V_Nil
					default:
						FactorType = nullptr;	//Recipe for segmentation fault
				}
				
				$$.reset(new Token_Factor(Token_Factor::Constant, $1, FactorType)); 
			}

	| OP_NOT Factor { 
					//You can only NOT a boolean
					$$ = $2; 
					std::shared_ptr<Token_Factor> RHS = std::dynamic_pointer_cast<Token_Factor>($$);
					if (RHS -> GetType() != std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("boolean").first->GetValue())){
						std::stringstream msg;
						msg << "You can only NOT a boolean. Factor is of type '" << RHS -> GetType() -> TypeToString() << "'";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2->GetLine(), $2 -> GetColumn());
					}
					RHS -> SetNegate();	
					
	}

	| SignedConstant{ 
				std::shared_ptr<Token_Type> FactorType;
				//Determine type
				switch($1 -> GetTokenType()){
					case V_Int:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("integer").first->GetValue());
						break;
					case V_Real:
						FactorType = std::dynamic_pointer_cast<Token_Type>(Program.GetTypeSymbol("real").first->GetValue());
						break;
					default:
						FactorType = nullptr;	//Recipe for segmentation fault
				}
				
				$$.reset(new Token_Factor(Token_Factor::Constant, $1, FactorType)); 
			}

	| SetConstructors		//TODO

	| Identifier	{ 
				//Check for symbols with identifier and then set the type of the factor accordingly
				try{
					std::pair<std::shared_ptr<Symbol>, AsmCode> sym(Program.GetSymbol($1 -> GetStrValue()));
					//Type and form of symbol
					Symbol::Type_T SymType = sym.first -> GetType();
					Token_Factor::Form_T Form;
					std::shared_ptr<Token_Type> FactorType;
					
					switch (SymType){
						case Symbol::Function:
						case Symbol::Procedure:		//For the purpose of this, we regard func and proc as the same. We will do type check later
							Form = Token_Factor::FuncCall; 
							FactorType = sym.first->GetTokenDerived<Token_Func>()->GetReturnType();
							break;
						case Symbol::Constant:		//Ditto
						case Symbol::Variable:
							Form = Token_Factor::VarRef; 
							FactorType = sym.first->GetTokenDerived<Token_Var>()->GetVarType();
							break;
						case Symbol::Typename:	//Shouldn't happen
							std::stringstream msg;
							msg << "Unexpected type '" << $1 -> GetStrValue() << "'.";
							HandleError(msg.str().c_str(), E_PARSE, E_FATAL, LexerLineCount, LexerCharCount);
							YYERROR;
					}
					$$.reset(new Token_Factor(Form, sym.first -> GetValue(), FactorType));
				}
				catch (AsmCode e){
					if (e == SymbolNotExists){
						std::stringstream msg;
						msg << "Unknown identifier '" << $1 -> GetStrValue() << "'.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, LexerLineCount, LexerCharCount);
					}
					else{
						std::stringstream msg;
						msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
					}
					YYERROR;
				}
				
			}
	/* Set, value typecast, address factor ?? */
	;
VarRef: SimpleVarReference VarQualifier{
											//TODO Qualifier
											
										}
	/*| SimpleVarReference */
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
		| V_STRING	{ /*$$.reset(new Token(yylval -> GetStrValue(), V_String));*/ }
		| V_CHAR	{ $$.reset(new Token_Char(yylval -> GetStrValue()[0])); }
		/*| Identifier	{ $$.reset(new Token(yylval -> GetStrValue(), V_Identifier)); }*/
		| V_NIL		{ $$.reset(new Token("NIL", V_Nil)); }
		| I_TRUE	{ $$.reset(new Token_Int(1, V_Boolean)); }
		| I_FALSE	{ $$.reset(new Token_Int(0, V_Boolean)); }
		| I_MAXINT	{ $$.reset(new Token_Int(2147483647, V_Int)); }
		;
SignedConstant: Signed_Int { $$ = $1; }
		| Signed_Real { $$ = $1; }
		;
		
FuncCall: Identifier '(' ActualParamList ')' {
				//TODO FuncCall Token
			}
	/* | Identifier */
	;

ActualParamList: ActualParamList ',' Expression { $$ = $1; std::dynamic_pointer_cast<Token_ExprList>($$) -> AddExpression(std::dynamic_pointer_cast<Token_Expression>($3)); }
		| Expression {$$.reset(new Token_ExprList(std::dynamic_pointer_cast<Token_Expression>($1)));}
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
		| StatementList ';' error { yyerrok; if (Flags.Pedantic) HandleError("Error in statement. Statement ignored.", E_PARSE, E_ERROR, LexerLineCount, LexerCharCount); }
		| Statement
		| error { yyerrok; if (Flags.Pedantic) HandleError("Error in statement. Statement ignored.", E_PARSE, E_ERROR, LexerLineCount, LexerCharCount); }
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
		| GotoStatement { HandleError("Labels and Gotos are unsupported.", E_PARSE, E_WARNING,LexerLineCount, LexerCharCount);  }
		;

AssignmentStatement: Identifier OP_ASSIGNMENT Expression {
			//Check for Identifier
			try{
				std::shared_ptr<Token_Type> LHS_T, RHS_T;
				std::shared_ptr<Token_Expression> RHS;

				std::pair<std::shared_ptr<Symbol>, AsmCode> sym(Program.GetSymbol($1 -> GetStrValue()));
				//Check that symbol is a variable
				if (sym.first -> GetType() != Symbol::Variable){
					std::stringstream msg;
					msg << "Identifier '" << $1 -> GetStrValue() << "' is not a variable.";
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
					throw SymbolIsNotAVariable;	//Skip rest of execution
				}					
				RHS = std::dynamic_pointer_cast<Token_Expression>($3);
				//Check that variable type matches expression
				
				LHS_T = sym.first->GetTokenDerived<Token_Var>()->GetVarType();
				RHS_T = RHS -> GetType();
				if (Program.TypeCompatibilityCheck(LHS_T, RHS_T) != TypeCompatible){
					std::stringstream msg;
					msg << "Incompatible Types: Variable '"<< $1 -> GetStrValue() << "' has type '" << LHS_T -> TypeToString();
					msg << "'\n\tand expression has type '" << RHS_T -> TypeToString() << "'"; 
					
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
					throw TypeIncompatible;
				}

				//Generate that assignment line!
				Program.CreateAssignmentLine(sym.first, RHS);
			}
			catch (AsmCode e){
				if (e == SymbolNotExists){
					std::stringstream msg;
					msg << "Unknown identifier '" << $1 -> GetStrValue() << "'.";
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
				}
				else{
					std::stringstream msg;
					msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
				}
				YYERROR;
			}
				
		}
		/* += -= /= *= */
		;

ProcedureStatement: Identifier '(' ActualParamList ')' {
					//We are going to hack in write here
					if ($1 -> GetStrValue() == "write"){
						Program.CreateWriteLine(std::dynamic_pointer_cast<Token_ExprList>($3));
					}
			}
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
	//std::stringstream text;
	//text << "Unknown parse error: " << msg;
	//text << "(" << yylloc.first_line << "-" << yylloc.last_line << ":" << yylloc.first_column;
	//text << "-" << yylloc.last_column << ")";
	HandleError(msg, E_PARSE, E_FATAL, LexerLineCount, LexerCharCount);
}
