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
	SymbolIsNotAVariable,
	
	LabelExists,
	LabelNotExists,
	
	TypeCompatible,
	TypeIncompatible,
	OperatorIncompatible,
	
	BoolTypeUndefined,
	BoolTypeIncorrect,
	
	ArrayOutofBound,
	ArrayDimensionOutOfBound,
	ArrayDimensionMismatch,
	ArrayBoundInvalid
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
	SimExpression,
	Term,
	Factor,
	Operator,
	FormalParamList,
	PointerDereference,
	ArrayIndices
};

/** Operator Type **/
enum Op_T{
		LT,
		LTE,
		GT,
		GTE,
		Equal,
		NotEqual,
		In,
	
		Add=10,
		Subtract,
		Or,
		Xor,
	
		//FactorOP
		Multiply=20,
		Divide,
		Div,
		Mod, 
		And,
		
		None=99
	};

#endif