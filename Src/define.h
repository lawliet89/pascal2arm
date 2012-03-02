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
	
	SymbolIsNotAType
};

/**
 * 	Non terminal symbols definition
 * 	- TODO move to enum
 * */

enum T_Type{
	V_Character,
	V_String,
	V_Identifier,
	V_Real,
	V_Int,
	V_Nil,
	
	I_True,
	I_False,
	
	Signed_Int,
	Signed_Real,
	Identifier,
	Variable,
	IdentifierList,
	VarList,
	Type,
	FuncProc
};

#endif