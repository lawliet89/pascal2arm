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
			
/* 
BlockProcFuncDeclaration: ProcList | FuncList ;
*/
BlockProcFuncDeclaration: ProcFuncList
			|;

ProcFuncList: ProcFuncList ProcDeclaration
			| ProcFuncList FuncDeclaration
			| ProcDeclaration
			| FuncDeclaration
			;

/* Generic Stuff */
Identifier: V_IDENTIFIER {
				$$.reset(new Token(yylval-> GetStrValue(), Identifier));
			}
	;

IdentifierList: IdentifierList ',' Identifier
			{
				$$ = $1;
				try{
					std::static_pointer_cast<Token_IDList>($$) -> AddID($3);
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
		
TypeDeclaration: Identifier '=' Type ';' {
					std::shared_ptr<Token_Type> type = std::static_pointer_cast<Token_Type>($3);
					std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.CreateTypeSymbol($1 -> GetStrValue(), type -> GetPrimary(), type -> GetSecondary());
				}
		;

VarList:	VarList VarDeclaration
		| VarDeclaration
		| VarList error { if (Flags.Pedantic) HandleError("Error in variable declaration. Variable ignored.", E_PARSE, E_ERROR, LexerLineCount, LexerCharCount); }
		| error { if (Flags.Pedantic) HandleError("Error in variable declaration. Variable ignored.", E_PARSE, E_ERROR, LexerLineCount, LexerCharCount); }
		;

VarDeclaration:	IdentifierList ':' Type '=' Expression ';'{
			//Handle expression
			//Check expression Type
			std::shared_ptr<Token_Expression> expr = std::static_pointer_cast<Token_Expression>($5);
			std::shared_ptr<Token_Type> type = std::static_pointer_cast<Token_Type>($3);
			
			
			if (Program.TypeCompatibilityCheck(expr->GetType(), type) != TypeCompatible){
				std::stringstream msg;
				msg << "Variable(s) of type '" << expr->GetType()-> TypeToString();
				msg << "' cannot be initialised with expressions of type '" << type -> TypeToString() << "'"; 
				
				HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
				YYERROR;
			}
			//Try to flatten expression
			std::shared_ptr<AsmLine> line = Program.FlattenExpression(expr);
			//Check if Rd is an immediate
			if (line -> GetRd() -> GetType() != AsmOp::Immediate){
				std::stringstream msg;
				msg << "Variable(s) cannot be initialised with non-constant expressions.";
				
				HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $5 -> GetLine(), $5 -> GetColumn());
				YYERROR;
			}
			
			Program.CreateVarSymbolsFromList(std::static_pointer_cast<Token_IDList>($1), type, line -> GetRd()->GetImmediate());
		}
		| IdentifierList ':' Type ';' {
			Program.CreateVarSymbolsFromList(std::static_pointer_cast<Token_IDList>($1),std::static_pointer_cast<Token_Type>($3));
		};

/* Types */
Type: SimpleType { $$ = $1; }
	| StringType 
	| StructuredType 
	| PointerType   { $$ = $1; }
	| TypeIdentifier
	;
	
SimpleType: OrdinalType { $$ = $1; }
	| RealType { $$ = $1; }
	;

OrdinalType: I_INTEGER	{ $$ = Program.GetTypeSymbol("integer").first->GetValue(); }
	| I_CHAR { $$ = Program.GetTypeSymbol("char").first->GetValue(); }
	| I_BOOLEAN { $$ = Program.GetTypeSymbol("boolean").first->GetValue(); }
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

SubrangeType: SubrangeValue OP_DOTDOT SubrangeValue{
				std::shared_ptr<Token_Type> type(new Token_Type("subrange", Token_Type::Integer, Token_Type::Subrange));
				type -> SetLowerRange(GetValue<int>($1));
				type -> SetUpperRange(GetValue<int>($3));
				
				$$ = std::static_pointer_cast<Token>(type);
			}
		;
SubrangeValue: V_INT { $$ = $1; }
		| Signed_Int { $$ = $1; }
//		| Identifier
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

PointerType: '^' Type {
				//Copy type
				std::shared_ptr<Token_Type> type = std::static_pointer_cast<Token_Type>($2);
				//Clone it
				type.reset(new Token_Type(*type));
				$$ = type;
				//Set secondary flag
				type -> SetPointer();
			}
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
FormalParamList: '(' FormalParam ')' { $$ = $2; }
				| '(' ')'{ $$.reset(new Token_FormalParam()); }
				| { $$.reset(new Token_FormalParam()); }
		;

FormalParam:  FormalParam ';' ParamDeclaration { $$ = $1; std::static_pointer_cast<Token_FormalParam>($$)->Merge( std::static_pointer_cast<Token_FormalParam>($3) ); }
	| ParamDeclaration { $$ = $1; }
	;

ParamDeclaration: ValueParam { $$ = $1; }
		| VarParam { $$ = $1; }
	/*	| OutParam
		| ConstantParam */
		;

ValueParam:	IdentifierList ':' Type {
				std::vector<std::shared_ptr<Token_Var> > vars = Program.CreateVarSymbolsFromList(std::static_pointer_cast<Token_IDList>($1), std::static_pointer_cast<Token_Type>($3));
				std::shared_ptr<Token_FormalParam> param(new Token_FormalParam());
				param->AddParams(vars);
				
				$$ = std::static_pointer_cast<Token>(param);
				
			}
	/*	| Identifier ':' Type '=' DefaultParamValue  */
		;
VarParam: K_VAR IdentifierList ':' Type {
				std::vector<std::shared_ptr<Token_Var> > vars = Program.CreateVarSymbolsFromList(std::static_pointer_cast<Token_IDList>($2), std::static_pointer_cast<Token_Type>($4));
				std::shared_ptr<Token_FormalParam> param(new Token_FormalParam());
				param->AddParams(vars, true, true);
				
				$$ = std::static_pointer_cast<Token>(param);
				
			}
		;
		
SubroutineBlock: Block
		| I_FORWARD	{		//TODO Forward declaration
		
		}
		;

ProcDeclaration: ProcHeader ';' SubroutineBlock ';'{
				std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.GetSymbol($1 -> GetStrValue());
				//std::shared_ptr<Token_Func> function = sym.first->GetTokenDerived<Token_Func>();
				
				std::shared_ptr<AsmLine> line = Program.CreateCodeLine(AsmLine::Directive, AsmLine::BLOCKPOP);
				std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
				Rd -> SetSymbol(sym.first);		
				line -> SetRd(Rd);
				Program.PopBlock();
			}
		;

ProcHeader: K_PROCEDURE Identifier {	//NOTE This is $3
					//Check that identifier is not defined in this scope
					//AsmCode sym = Program.CheckSymbol($2->GetStrValue());
					try{
						//Create a new block and a new function
						std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.CreateProcFuncSymbol($2 -> GetStrValue(),false);		//Block is also pushed
						//Then create a to push block
						std::shared_ptr<AsmLine> line = Program.CreateCodeLine(AsmLine::Directive, AsmLine::BLOCKPUSH);
						std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
						Rd -> SetSymbol(sym.first);
						line -> SetRd(Rd);
						
					}
					catch(AsmCode sym){
						if (sym == SymbolExistsInCurrentBlock){
							std::stringstream msg;
							msg << "Identifier '" << $2->GetStrValue() << "' has already been declared in this scope."	;			
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
							YYERROR;
						}
						else if (sym == SymbolReserved){
							std::stringstream msg;
							msg << "Identifier '" << $2->GetStrValue() << "' is a reserved identifier.";				
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
							YYERROR;
						}
						else if (sym == SymbolExistsInOuterBlock && Flags.Pedantic){
							std::stringstream msg;
							msg << "Identifier '" << $2->GetStrValue() << "' might occlude another symbol defined in an outer scope.";				
							HandleError(msg.str().c_str(), E_PARSE, E_WARNING, $2 -> GetLine(), $2 -> GetColumn());
						}
					}

				} FormalParamList{ //NOTE $4
					//Get function
					std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.GetSymbol($2 -> GetStrValue());
					std::shared_ptr<Token_Func> function = sym.first->GetTokenDerived<Token_Func>();
					
					//Let's deal with formal param
					std::shared_ptr<Token_FormalParam> params = std::static_pointer_cast<Token_FormalParam>($4);
					function -> SetParams(params);
					
					$$=$2;
				};

FuncDeclaration: FuncHeader ';' SubroutineBlock ';' {
				std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.GetSymbol($1 -> GetStrValue());
				std::shared_ptr<Token_Func> function = sym.first->GetTokenDerived<Token_Func>();
				
				if (!function -> GetHasReturn()){
					std::stringstream msg;
					msg << "In function '" << $1 -> GetStrValue() <<  "', no write to the function return variable was performed. Return value is undefined.";
					HandleError(msg.str().c_str(), E_PARSE, E_WARNING, $1 -> GetLine(), $1 -> GetColumn());
				}
				
				std::shared_ptr<AsmLine> line = Program.CreateCodeLine(AsmLine::Directive, AsmLine::BLOCKPOP);
				std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
				Rd -> SetSymbol(sym.first);		
				line -> SetRd(Rd);
				Program.PopBlock();
			}
		;

FuncHeader: K_FUNCTION Identifier {	//NOTE This is $3
					//Check that identifier is not defined in this scope
					//AsmCode sym = Program.CheckSymbol($2->GetStrValue());
					try{
						//Create a new block and a new function
						std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.CreateProcFuncSymbol($2 -> GetStrValue(),true);		//Block is also pushed
						//Then create a to push block
						std::shared_ptr<AsmLine> line = Program.CreateCodeLine(AsmLine::Directive, AsmLine::BLOCKPUSH);
						std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
						Rd -> SetSymbol(sym.first);
						line -> SetRd(Rd);
						
					}
					catch(AsmCode sym){
						if (sym == SymbolExistsInCurrentBlock){
							std::stringstream msg;
							msg << "Identifier '" << $2->GetStrValue() << "' has already been declared in this scope."	;			
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
							YYERROR;
						}
						else if (sym == SymbolReserved){
							std::stringstream msg;
							msg << "Identifier '" << $2->GetStrValue() << "' is a reserved identifier.";				
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2 -> GetLine(), $2 -> GetColumn());
							YYERROR;
						}
						else if (sym == SymbolExistsInOuterBlock && Flags.Pedantic){
							std::stringstream msg;
							msg << "Identifier '" << $2->GetStrValue() << "' might occlude another symbol defined in an outer scope.";				
							HandleError(msg.str().c_str(), E_PARSE, E_WARNING, $2 -> GetLine(), $2 -> GetColumn());
						}
					}

				} FormalParamList ':' Type { //NOTE $4 $5 $6
					//Get function
					std::pair<std::shared_ptr<Symbol>, AsmCode> sym = Program.GetSymbol($2 -> GetStrValue());
					std::shared_ptr<Token_Func> function = sym.first->GetTokenDerived<Token_Func>();
					function -> SetReturnType(std::static_pointer_cast<Token_Type>($6));
					
					//Let's deal with formal param
					std::shared_ptr<Token_FormalParam> params = std::static_pointer_cast<Token_FormalParam>($4);
					function -> SetParams(params);
					
					$$=$2;
				}
			;

/* Expression */
Expression:  Expression SimpleOp SimpleExpression {
			std::shared_ptr<Token_Expression> LHS(std::static_pointer_cast<Token_Expression>($1));
			std::shared_ptr<Token_SimExpression> RHS(std::static_pointer_cast<Token_SimExpression>($3));
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
	| SimpleExpression { $$.reset(new Token_Expression(std::static_pointer_cast<Token_SimExpression>($1)));}
	;

SimpleOp: '<' { $$.reset(new Token_Int((int) Op_T::LT, T_Type::Operator)); } 
	| OP_LE { $$.reset(new Token_Int((int) Op_T::LTE, T_Type::Operator)); }
	| '>' { $$.reset(new Token_Int((int) Op_T::GT, T_Type::Operator)); }
	| OP_GE { $$.reset(new Token_Int((int) Op_T::GTE, T_Type::Operator)); }
	| '=' { $$.reset(new Token_Int((int) Op_T::Equal, T_Type::Operator)); }
	| OP_NOTEQUAL { $$.reset(new Token_Int((int) Op_T::NotEqual, T_Type::Operator)); }
	| OP_IN { $$.reset(new Token_Int((int) Op_T::In, T_Type::Operator)); }
	; 

SimpleExpression: SimpleExpression TermOP Term {
			std::shared_ptr<Token_SimExpression> LHS(std::static_pointer_cast<Token_SimExpression>($1));
			std::shared_ptr<Token_Term> RHS(std::static_pointer_cast<Token_Term>($3));
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
		| Term { $$.reset(new Token_SimExpression(std::static_pointer_cast<Token_Term>($1)));	}
		;

TermOP: '+' { $$.reset(new Token_Int((int) Op_T::Add, T_Type::Operator)); }
	| '-' { $$.reset(new Token_Int((int) Op_T::Subtract, T_Type::Operator)); }
	| OP_OR { $$.reset(new Token_Int((int) Op_T::Or, T_Type::Operator)); }
	| OP_XOR { $$.reset(new Token_Int((int) Op_T::Xor, T_Type::Operator)); }
	;

Term:  Term FactorOP Factor{
			std::shared_ptr<Token_Term> LHS(std::static_pointer_cast<Token_Term>($1));
			std::shared_ptr<Token_Factor> RHS(std::static_pointer_cast<Token_Factor>($3));
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
			$$.reset(new Token_Term(std::static_pointer_cast<Token_Factor>($1)));
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
						$$ = $1;
					}

	| FuncCall		{ $$ = $1; }
	| UnsignedConstant { 
				std::shared_ptr<Token_Type> FactorType;
				//Determine type
				switch($1 -> GetTokenType()){
					case V_Character:
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("char").first->GetValue());
						break;
					case V_Int:
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("integer").first->GetValue());
						break;
					case V_Real:
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("real").first->GetValue());
						break;
					case V_Boolean:
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("boolean").first->GetValue());
						break;	
					case V_String:
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("string").first->GetValue());
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
					std::shared_ptr<Token_Factor> RHS = std::static_pointer_cast<Token_Factor>($$);
					if (RHS -> GetType() != std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("boolean").first->GetValue())){
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
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("integer").first->GetValue());
						break;
					case V_Real:
						FactorType = std::static_pointer_cast<Token_Type>(Program.GetTypeSymbol("real").first->GetValue());
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
					
					if (SymType == Symbol::Procedure){
						std::stringstream msg2;
						msg2 << "Identifier " << $1 -> GetStrValue() << "' is a procedure -- expressions must have a type. ";
						HandleError(msg2.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
						YYERROR;
					}
					else if (SymType == Symbol::Function){
						Form = Token_Factor::FuncCall; 
						FactorType = sym.first->GetTokenDerived<Token_Func>()->GetReturnType();
					}
					else if (SymType == Symbol::Constant || Symbol::Variable){
						Form = Token_Factor::VarRef; 
						FactorType = sym.first->GetTokenDerived<Token_Var>()->GetVarType();
					}
					else if (SymType == Symbol::Typename){	//Shouldn't happen
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
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
					}
					else{
						std::stringstream msg;
						msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
					}
					YYERROR;
				}
				catch(...){
					YYERROR;
				}
				
			}
	/* Set, value typecast, address factor ?? */
	;
VarRef: /*SimpleVarReference VarQualifier | */
		Identifier VarQualifier {
				//Check for symbols with identifier and then set the type of the factor accordingly
				try{
					std::pair<std::shared_ptr<Symbol>, AsmCode> sym(Program.GetSymbol($1 -> GetStrValue()));
					//Type and form of symbol
					Symbol::Type_T SymType = sym.first -> GetType();
					
					std::shared_ptr<Token_Type> FactorType;
					std::shared_ptr<Token> FactorToken;
					
					if (SymType != Symbol::Variable){
						std::stringstream msg2;
						msg2 << "Identifier " << $1 -> GetStrValue() << "' is not a variable. Only variables can be used with qualifiers.";
						HandleError(msg2.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
						YYERROR;
					}
					
					//Check for qualifier type
					T_Type QualifierType = $2 -> GetTokenType();
					if (QualifierType == PointerDereference){
						//Okay dereferencing a pointer
						//Clone the variable 
						std::shared_ptr<Token_Var> var( new Token_Var( * ( sym.first -> GetTokenDerived<Token_Var>() ) ) );
						var -> SetDereference();
						
						//Clone the symbol
						std::shared_ptr<Symbol> sym(new Symbol( * (var -> GetSymbol()) ) );
						
						//Assignment
						sym -> SetToken(std::static_pointer_cast<Token>(var));
						var -> SetSymbol(sym);
						
						FactorToken = std::static_pointer_cast<Token>(var);
						FactorType = var -> GetVarType();
					}
					//TODO Arrays
					
					$$.reset(new Token_Factor(Token_Factor::VarRef, FactorToken, FactorType));
				}
				catch (AsmCode e){
					if (e == SymbolNotExists){
						std::stringstream msg;
						msg << "Unknown identifier '" << $1 -> GetStrValue() << "'.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
					}
					else{
						std::stringstream msg;
						msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
					}
					YYERROR;
				}
				catch(...){
					YYERROR;
				}								
		}
	/*| SimpleVarReference */
	;
/*
SimpleVarReference: Identifier { $$ = $1; }
		;*/

VarQualifier: FieldSpecifier
		| Index
		| '^' { $$.reset(new Token("^", PointerDereference, true)); }
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
				try{
					std::pair<std::shared_ptr<Symbol>, AsmCode> sym(Program.GetSymbol($1 -> GetStrValue()));
					//Type and form of symbol
					Symbol::Type_T SymType = sym.first -> GetType();
					if (SymType != Symbol::Function){
						std::stringstream msg;
						msg << "Identifier '" << $1 -> GetStrValue() << "' is not a function.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
						YYERROR;
					}
					
					//Check for parameters matching - TODO for optional parameters support.
					std::vector<Token_FormalParam::Param_T> FormalParams = sym.first -> GetTokenDerived<Token_Func>() -> GetParams() -> GetParams();
					std::vector<std::shared_ptr<Token_Expression> > ActualExpr = std::static_pointer_cast<Token_ExprList>($3) -> GetList();
					if (FormalParams.size() != ActualExpr.size()){
						std::stringstream msg;
						msg << "Function '" << $1 -> GetStrValue() << "' expects " << FormalParams.size() << " parameter(s) but " << ActualExpr.size() << " were provided." ;
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
						YYERROR;
					}
					unsigned count = FormalParams.size();
					//Check for parameter type matching
					bool IsError = false;
					for (unsigned i = 0; i < count; i++){
						std::shared_ptr<Token_Type> LHS = FormalParams[i].Variable -> GetVarType(), RHS =  ActualExpr[i]->GetType();
						if (Program.TypeCompatibilityCheck(LHS, RHS) != TypeCompatible ){
							std::stringstream msg;
							msg << "Function '" << $1 -> GetStrValue() << "' expects parameter " << i+1 <<" to be of type " << LHS -> TypeToString() << "\n\tbut type " << RHS -> TypeToString() << " provided.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
							IsError = true;
						}
						//If parameter is a reference, RHS MUST be a variable
						if (FormalParams[i].Reference){
							//Then it will be a strictly simple expression -- otherwise it fails
							std::shared_ptr<Token_Factor> exprvar = ActualExpr[i] -> GetSimple();
							bool fail = false;
							if (exprvar == nullptr)
								fail = true;
							else if (exprvar -> GetForm() != Token_Factor::VarRef){
								//Check form
								fail = true;
							}
							if (fail){
								std::stringstream msg;
								msg << "Function '" << $1 -> GetStrValue() << "' expects parameter " << i+1 <<" to be a variable reference.";
								HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
								IsError = true;
							}
						}
					}
					
					if (IsError){
						YYERROR;
					}
					std::shared_ptr<Token_Func> func = sym.first->GetTokenDerived<Token_Func>();
					//Time to create the factor
					std::shared_ptr<Token_Factor> factor( new Token_Factor(Token_Factor::FuncCall, $3, func->GetReturnType()));
					factor -> SetFuncToken(func);
					
					$$ = std::static_pointer_cast<Token>(factor);
				}
				catch (AsmCode e){
					if (e == SymbolNotExists){
						std::stringstream msg;
						msg << "Identifier '" << $1 -> GetStrValue() << "' does not exist." ;
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
						YYERROR;
					}
				}
			}
	/* | Identifier */
	;

ActualParamList: ActualParamList ',' Expression { $$ = $1; std::static_pointer_cast<Token_ExprList>($$) -> AddExpression(std::static_pointer_cast<Token_Expression>($3)); }
		| Expression {$$.reset(new Token_ExprList(std::static_pointer_cast<Token_Expression>($1)));}
		| {$$.reset(new Token_ExprList());}
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
		| StatementList ';' error { if (Flags.Pedantic) HandleError("Error in statement. Statement ignored.", E_PARSE, E_ERROR, LexerLineCount, LexerCharCount); }
		| Statement
		| error { if (Flags.Pedantic) HandleError("Error in statement. Statement ignored.", E_PARSE, E_ERROR, LexerLineCount, LexerCharCount); }
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
					//Check to see if we are in a function
					std::shared_ptr<AsmBlock> CurrentBlock = Program.GetCurrentBlock();
					if (CurrentBlock -> GetType() != AsmBlock::Function){
						msg << "Identifier '" << $1 -> GetStrValue() << "' is not a variable or a function return variable that is not applicable in this scope.";
						HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
						YYERROR;
					}
					else{
						//Let's check if this is the correct function
						if (CurrentBlock -> GetBlockSymbol() != sym.first){
							msg << "Identifier '" << $1 -> GetStrValue() << "' is a function return variable that does not belong to this scope.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
							YYERROR;
						}
						else{
							sym.first->GetTokenDerived<Token_Func>()->SetHasReturn();
						}
					}
					
				}					
				RHS = std::static_pointer_cast<Token_Expression>($3);
				//Check that variable type matches expression
				if (sym.first -> GetType() != Symbol::Variable)
					LHS_T = sym.first->GetTokenDerived<Token_Func>()->GetReturnType();
				else
					LHS_T = sym.first->GetTokenDerived<Token_Var>()->GetVarType();
				RHS_T = RHS -> GetType();
				if (Program.TypeCompatibilityCheck(LHS_T, RHS_T) != TypeCompatible){
					std::stringstream msg;
					msg << "Incompatible Types: Variable '"<< $1 -> GetStrValue() << "' has type '" << LHS_T -> TypeToString();
					msg << "'\n\tand expression has type '" << RHS_T -> TypeToString() << "'"; 
					
					HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
					YYERROR;
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
		| VarRef OP_ASSIGNMENT Expression{
			//VarRef is a factor
			//Get the variable
			std::shared_ptr<Token_Var> var = std::static_pointer_cast<Token_Factor>($1) -> GetTokenDerived<Token_Var>();
			std::shared_ptr<Token_Expression> RHS = std::static_pointer_cast<Token_Expression>($3);
			std::shared_ptr<Token_Type> LHS_T, RHS_T = RHS -> GetType();
			
			//Pointer dereference
			if (var -> GetDereference()){
				//Set LHS_T
				LHS_T.reset(new Token_Type( *(var -> GetVarType()) ));
				LHS_T -> SetPointer(false);
			}
			
			if (Program.TypeCompatibilityCheck(LHS_T, RHS_T) != TypeCompatible){
				std::stringstream msg;
				msg << "Incompatible Types: Variable '"<< $1 -> GetStrValue() << "' has type '" << LHS_T -> TypeToString();
				msg << "'\n\tand expression has type '" << RHS_T -> TypeToString() << "'"; 
				
				HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
				YYERROR;
			}
			//Clone symbol
			std::shared_ptr<Symbol> sym(new Symbol( *(var -> GetSymbol()) ) );
			sym -> SetToken(std::static_pointer_cast<Token>(var));
			var -> SetSymbol(sym);
			
			Program.CreateAssignmentLine(sym, RHS);
		}
		/* += -= /= *= */
		;

ProcedureStatement: Identifier '(' ActualParamList ')' {
				//We are going to hack in write here
				if ($1 -> GetStrValue() == "write"){
					Program.CreateWriteLine(std::static_pointer_cast<Token_ExprList>($3));
				}
				else if ($1 -> GetStrValue() == "new"){
					Program.CreateNewProcLine(std::static_pointer_cast<Token_ExprList>($3));
				}
				else if ($1 -> GetStrValue() == "dispose"){
					Program.CreateDisposeProcLine(std::static_pointer_cast<Token_ExprList>($3));
				}
				else{
					try{
						std::pair<std::shared_ptr<Symbol>, AsmCode> sym(Program.GetSymbol($1 -> GetStrValue()));
						//Type and form of symbol
						Symbol::Type_T SymType = sym.first -> GetType();
						if (SymType != Symbol::Procedure){
							std::stringstream msg;
							msg << "Identifier '" << $1 -> GetStrValue() << "' is not a procedure.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
							YYERROR;
						}
						
						//Check for parameters matching - TODO for optional parameters support.
						std::vector<Token_FormalParam::Param_T> FormalParams = sym.first -> GetTokenDerived<Token_Func>() -> GetParams() -> GetParams();
						std::vector<std::shared_ptr<Token_Expression> > ActualExpr = std::static_pointer_cast<Token_ExprList>($3) -> GetList();
						if (FormalParams.size() != ActualExpr.size()){
							std::stringstream msg;
							msg << "Procedure '" << $1 -> GetStrValue() << "' expects " << FormalParams.size() << " parameter(s) but " << ActualExpr.size() << " were provided." ;
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
							YYERROR;
						}
						unsigned count = FormalParams.size();
						//Check for parameter type matching
						bool IsError = false;
						for (unsigned i = 0; i < count; i++){
							std::shared_ptr<Token_Type> LHS = FormalParams[i].Variable -> GetVarType(), RHS =  ActualExpr[i]->GetType();
							if (Program.TypeCompatibilityCheck(LHS, RHS) != TypeCompatible ){
								std::stringstream msg;
								msg << "Procedure '" << $1 -> GetStrValue() << "' expects parameter " << i+1 <<" to be of type " << LHS -> TypeToString() << "\n\tbut type " << RHS -> TypeToString() << " provided.";
								HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
								IsError = true;
							}
						}
						
						if (IsError){
							YYERROR;
						}
						
						//Okay time to generate some code
						//TODO - More than three args
						std::shared_ptr<Token_Func> func = sym.first->GetTokenDerived<Token_Func>();
						std::shared_ptr<AsmLine> line = Program.CreateCodeLine(AsmLine::Directive, AsmLine::FUNCALL);
						
						line -> SetRd(nullptr);	//To signify it's a procedure call
						
						std::vector<std::shared_ptr<Token_Expression> >::iterator it;
						unsigned i = 0;
						for (it = ActualExpr.begin(); it < ActualExpr.end() && i < 3; it++, i++){
							std::shared_ptr<Token_Expression> expr = *it;
							
							//OK create a new operator
							std::shared_ptr<AsmOp> ExprRd(new AsmOp(AsmOp::Register, AsmOp::Rm));
							ExprRd -> SetSymbol(FormalParams[i].Variable->GetSymbol());
							std::shared_ptr<AsmLine> line2 = Program.FlattenExpression(expr);
							
							std::shared_ptr<AsmOp> Ri( new AsmOp(*(line2->GetRd())) );
							
							switch (i){
								case 0:
									line -> SetRm(Ri); break;
								case 1: 
									line -> SetRn(Ri); break;
								case 2:
									line -> SetRo(Ri); break;
							}
						}
						
						//Create branch line
						std::shared_ptr<AsmLine> branch = Program.CreateCodeLine(AsmLine::BranchLink, AsmLine::BL);
						std::shared_ptr<AsmOp> BranchOp(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
						BranchOp -> SetLabel(func -> GetSymbol()->GetLabel());
						branch -> SetRd(BranchOp);
					}
					catch (AsmCode e){
						if (e == SymbolNotExists){
							std::stringstream msg;
							msg << "Identifier '" << $1 -> GetStrValue() << "' does not exist." ;
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1->GetLine(), $1->GetColumn());
							YYERROR;
						}
					}
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

	/*
IfStatement: K_IF Expression K_THEN Statement ElsePart
	| K_IF Expression K_THEN Statement
	;
	
	*/

IfStatement: IfTest IfExecute;

IfTest: K_IF Expression K_THEN{
				//Expression
				std::shared_ptr<Token_Expression> expr(std::static_pointer_cast<Token_Expression>($2));
				std::shared_ptr<AsmLine> line = Program.FlattenExpression(expr, nullptr, true), branch;
				
				std::shared_ptr<AsmLabel> label = Program.GetCurrentBlock()->CreateIfElseLabel();
				Program.GetCurrentBlock()->IfLabelStackPush(label);
				Op_T Op = expr -> GetOp();
				if (Op == None){
					//Goodness this is problematic -- probably because this is a boolean variable or something
					//Create one more line to see if this is equal to true or not
					//Create immediate 1
					std::shared_ptr<AsmOp> One(new AsmOp(AsmOp::Immediate, AsmOp::Rn));
					One -> SetImmediate("#1");
					std::shared_ptr<AsmLine> LineTrueTest = Program.CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
					LineTrueTest -> SetRd(line -> GetRd());
					LineTrueTest -> SetRn(One);
				}
				
				branch = Program.CreateCodeLine(AsmLine::Branch, AsmLine::B);
				
				//Handle Label
				std::shared_ptr<AsmOp> OpLabel(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
				OpLabel -> SetLabel(label);
				branch -> SetRd(OpLabel);
				
				
				if (Op == Equal){
					branch -> SetCC(AsmLine::NE);
				}
				else if (Op == LT)
					branch -> SetCC(AsmLine::GE);
				else if (Op == LTE)
					branch -> SetCC(AsmLine::GT);	
				else if (Op == GT)
					branch -> SetCC(AsmLine::LE);
				else if (Op == GTE)
					branch -> SetCC(AsmLine::LT);
				else if (Op == NotEqual)
					branch -> SetCC(AsmLine::EQ);
				else if (Op == None){
					branch -> SetCC(AsmLine::LT);
				}
			};
			
IfExecute: IfBody IfElse {
				//There was an else
				Program.GetCurrentBlock()->SetNextLabel(Program.GetCurrentBlock()->IfLabelStackPop());
				Program.GetCurrentBlock()->IfLineStackPop();
			}
		| IfBody {
			//Oh? Never came to be
			std::shared_ptr<AsmLine> line = Program.GetCurrentBlock()->IfLineStackPop();
			line -> SetCC(AsmLine::NV);
			Program.GetCurrentBlock()->IfLabelStackPop();
			
		}
		;
			
IfBody: Statement{
			std::shared_ptr<AsmLine> line = Program.CreateCodeLine(AsmLine::Branch, AsmLine::B);		//NOTE: ORDER OF LINES ARE IMPORTANT
			Program.GetCurrentBlock()->IfLineStackPush(line);
			Program.GetCurrentBlock()->SetNextLabel(Program.GetCurrentBlock()->IfLabelStackPop());
			std::shared_ptr<AsmLabel> label = Program.GetCurrentBlock()->CreateIfElseLabel();
			Program.GetCurrentBlock()->IfLabelStackPush(label);		
			
			//Handle Label
			std::shared_ptr<AsmOp> OpLabel(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
			OpLabel -> SetLabel(label);
			line -> SetRd(OpLabel);
		};

IfElse: K_ELSE Statement
;	


ForStatement: K_FOR Identifier OP_ASSIGNMENT Expression K_TO Expression { //NOTE: This is $7
					//Get Identifier symbol
					try{
						std::pair<std::shared_ptr<Symbol>, AsmCode> index(Program.GetSymbol($2 -> GetStrValue()));
						//Check that index is a variable
						if (index.first -> GetType() != Symbol::Variable){
							std::stringstream msg;
							msg << "In the for loop header, identifier '" << $2 -> GetStrValue() << "' is not a variable.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2->GetLine(), $2->GetColumn());
							YYERROR;
						}
						
						std::shared_ptr<Token_Expression> FromExpr = std::static_pointer_cast<Token_Expression>($4), ToExpr = std::static_pointer_cast<Token_Expression>($6);
						//std::shared_ptr<Symbol> FromVar = Program.CreateTempVar(FromExpr->GetType()), ToVar = Program.CreateTempVar(ToExpr -> GetType());
						
						//All three parts MUST be integers
						std::shared_ptr<Token_Type> IntegerType = Program.GetTypeSymbol("integer").first->GetTokenDerived<Token_Type>();
						
						if (index.first -> GetTokenDerived<Token_Var>() -> GetVarType() != IntegerType){
							std::stringstream msg;
							msg << "In the for loop header, variable '" << $2 -> GetStrValue() << "' must be of type integer.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2->GetLine(), $2->GetColumn());
							YYERROR;
						}
						
						if (FromExpr -> GetType() != IntegerType){
							HandleError("In the for loop header, the from expression must be of type integer.", E_PARSE, E_ERROR, $4->GetLine(), $4->GetColumn());
							YYERROR;
						}
						
						if (ToExpr -> GetType() != IntegerType){
							HandleError("In the for loop header, the to expression must be of type integer.", E_PARSE, E_ERROR, $6->GetLine(), $6->GetColumn());
							YYERROR;
						}			
						
						/** Checks are done **/
						//Set index to become FromExpr
						Program.CreateAssignmentLine(index.first, FromExpr);
						
						//Create branch label
						std::shared_ptr<AsmLabel> label = Program.GetCurrentBlock()->CreateForLabel();
						Program.GetCurrentBlock()->ForLabelStackPush(label);
						Program.GetCurrentBlock()->SetNextLabel(label);
						Program.GetCurrentBlock()->InLoopStackPush();
					}
					catch (AsmCode e){
						if (e == SymbolNotExists){
							std::stringstream msg;
							msg << "Unknown identifier '" << $2 -> GetStrValue() << "'.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $2->GetLine(), $2->GetColumn());
						}
						else{
							std::stringstream msg;
							msg << "An unknown error of code " << (int) e << " has occurred. This is probably a parser bug.";
							HandleError(msg.str().c_str(), E_PARSE, E_ERROR, $1 -> GetLine(), $1 -> GetColumn());
						}
						YYERROR;
					}
			} K_DO Statement {
				//NOTE: Statement is $9
				//Increment index by one
				std::pair<std::shared_ptr<Symbol>, AsmCode> index(Program.GetSymbol($2 -> GetStrValue()));
				
				std::shared_ptr<AsmOp> IndexOp(new AsmOp(AsmOp::Register, AsmOp::Rd));
				IndexOp -> SetWrite();
				IndexOp -> SetSymbol(index.first);
				std::shared_ptr<AsmLine> line1 = Program.CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
				line1 -> SetRd(IndexOp);
				line1 -> SetRm(IndexOp);
				
				//Create immediate 1
				std::shared_ptr<AsmOp> One(new AsmOp(AsmOp::Immediate, AsmOp::Rn));
				One -> SetImmediate("#1");
				line1 -> SetRn(One);
				
				IndexOp.reset(new AsmOp(*IndexOp));		//Clone IndexOp
				//Create Compare
				std::shared_ptr<AsmLine> line2 = Program.CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
				line2 -> SetRd(IndexOp);
				
				std::shared_ptr<AsmLine> ToExpr = Program.FlattenExpression(std::static_pointer_cast<Token_Expression>($6));
				line2 -> SetRm(ToExpr -> GetRd());
				
				//The branch line
				std::shared_ptr<AsmLabel> label = Program.GetCurrentBlock()->ForLabelStackPop();
				std::shared_ptr<AsmLine> branch = Program.CreateCodeLine(AsmLine::Branch, AsmLine::B);
				branch -> SetCC(AsmLine::LE);
				std::shared_ptr<AsmOp> LabelOp(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
				LabelOp -> SetLabel(label);
				branch -> SetRd(LabelOp);
				
				Program.GetCurrentBlock()->InLoopStackPop();
			}
	;

RepeatStatement: K_REPEAT StatementList K_UNTIL Expression
		;

WhileStatement: K_WHILE Expression { 	//NOTE: This is $3
					std::shared_ptr<Token_Expression> expr = std::static_pointer_cast<Token_Expression>($2);
					if (expr -> GetType() != Program.GetTypeSymbol("boolean").first->GetTokenDerived<Token_Type>()){
						HandleError("In the while loop header, expression must evaluate to type boolean. ", E_PARSE, E_ERROR, $2->GetLine(), $2->GetColumn());
						YYERROR;
					}
					std::shared_ptr<AsmLine> line1 = Program.FlattenExpression(expr, nullptr, true);
					
					std::shared_ptr<AsmLabel> labelStart = Program.GetCurrentBlock()->CreateWhileLabel();
					Program.GetCurrentBlock()->WhileStartLabelStackPush(labelStart);
					
					std::shared_ptr<AsmLabel> labelEnd = Program.GetCurrentBlock()->CreateWhileLabel();
					Program.GetCurrentBlock()->WhileEndLabelStackPush(labelEnd);
					Op_T Op = expr -> GetOp();
					if (Op == None){
						//Goodness this is problematic -- probably because this is a boolean variable or something
						//Create one more line to see if this is equal to true or not
						//Create immediate 1
						std::shared_ptr<AsmOp> One(new AsmOp(AsmOp::Immediate, AsmOp::Rn));
						One -> SetImmediate("#1");
						std::shared_ptr<AsmLine> LineTrueTest = Program.CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
						LineTrueTest -> SetRd(line1 -> GetRd());
						LineTrueTest -> SetRn(One);
					}					
					std::shared_ptr<AsmLine> branch = Program.CreateCodeLine(AsmLine::Processing, AsmLine::B);
					std::shared_ptr<AsmOp> OpLabel(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
					OpLabel -> SetLabel(labelEnd);
					branch -> SetRd(OpLabel);
					
					
					
					if (Op == Equal){
						branch -> SetCC(AsmLine::NE);
					}
					else if (Op == LT)
						branch -> SetCC(AsmLine::GE);
					else if (Op == LTE)
						branch -> SetCC(AsmLine::GT);	
					else if (Op == GT)
						branch -> SetCC(AsmLine::LE);
					else if (Op == GTE)
						branch -> SetCC(AsmLine::LT);
					else if (Op == NotEqual)
						branch -> SetCC(AsmLine::EQ);
					else if (Op == None)
						branch -> SetCC(AsmLine::LT);
						
					Program.GetCurrentBlock()->SetNextLabel(labelStart);
					Program.GetCurrentBlock()->InLoopStackPush();
					
				} K_DO Statement{ //NOTE: Statement is $5
					std::shared_ptr<Token_Expression> expr = std::static_pointer_cast<Token_Expression>($2);
					std::shared_ptr<AsmLine> line1 = Program.FlattenExpression(expr, nullptr, true);
					Op_T Op = expr -> GetOp();
					
					if (Op == None){
						//Goodness this is problematic -- probably because this is a boolean variable or something
						//Create one more line to see if this is equal to true or not
						//Create immediate 1
						std::shared_ptr<AsmOp> One(new AsmOp(AsmOp::Immediate, AsmOp::Rn));
						One -> SetImmediate("#1");
						std::shared_ptr<AsmLine> LineTrueTest = Program.CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
						LineTrueTest -> SetRd(line1 -> GetRd());
						LineTrueTest -> SetRn(One);
					}
					std::shared_ptr<AsmLine> branch = Program.CreateCodeLine(AsmLine::Processing, AsmLine::B);
					std::shared_ptr<AsmOp> OpLabel(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
					OpLabel -> SetLabel(Program.GetCurrentBlock()->WhileStartLabelStackPop());
					branch -> SetRd(OpLabel);
					
					
					if (Op == Equal){
						branch -> SetCC(AsmLine::EQ);
					}
					else if (Op == LT)
						branch -> SetCC(AsmLine::LT);
					else if (Op == LTE)
						branch -> SetCC(AsmLine::LE);	
					else if (Op == GT)
						branch -> SetCC(AsmLine::GT);
					else if (Op == GTE)
						branch -> SetCC(AsmLine::GE);
					else if (Op == NotEqual)
						branch -> SetCC(AsmLine::NE);
					else if (Op == None)
						branch -> SetCC(AsmLine::GE);
						
					Program.GetCurrentBlock()->SetNextLabel(Program.GetCurrentBlock()->WhileEndLabelStackPop());
					Program.GetCurrentBlock()->InLoopStackPop();
				}
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
