#ifndef DEF_H
#define DEF_H

//Macros definition

/** ASM Exception Codes **/
#define ASM_SymbolExists 1
#define Asm_SymbolNotExists 2


/**
 * 	Non terminal symbols definition
 * 
 * */

#define _Signed_Int -1
#define _Signed_Real -2
#define _Identifier -3
#define _Variable -4
#define _IdentifierList -5
#define _VarList -6

/**
 * 
 * 	Variable Type definition
 * 
 * */
#define Var_Integer -900
#define Var_Real -901
#define Var_Boolean -902
#define Var_Char -903
#define Var_String -904
#define Var_Enum -905
#define Var_Record -906
#define Var_Set -907
#define Var_File -908

#define Var_Subrange -909
#define Var_Array -910
#define Var_Pointer -911

#endif