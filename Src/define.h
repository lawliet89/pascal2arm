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

#define _Signed_Int -1
#define _Signed_Real -2
#define _Identifier -3
#define _Variable -4
#define _IdentifierList -5
#define _VarList -6
#define _Type -7

#endif