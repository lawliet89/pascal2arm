#ifndef DEF_H
#define DEF_H

/**
 * Message Enum
 * 
 * */

enum AsmCode{
	SymbolExists,
	
	SymbolReserved,
	SymbolExistsInCurrentBlock,
	SymbolExistsInOuterBlock,
	SymbolNotExists,
	
	SymbolCreated,
	SymbolOccludes,
	
	SymbolIsNotAType,
	
	LabelExists,
	LabelNotExists,
	
	TypeIncompatible,
	OperationIncompatible
};

/**
 * 	Non terminal symbols definition
 * 	- TODO move to enum
 * */

enum T_Type{	//for token type
	V_Character,
	V_String,
	V_Identifier,
	V_Real,
	V_Int,
	V_Nil,
	V_Boolean,
	
	I_True,
	I_False,
	
	Signed_Int,
	Signed_Real,
	Identifier,
	Variable,
	IdentifierList,
	VarList,
	Type,
	FuncProc,
	Expression,
	Term,
	Factor
};

/** Operator Type **/
enum Op_T{
		Multiply,
		Divide,
		Div,
		Mod, 
		And,
		
		None
	};

#endif