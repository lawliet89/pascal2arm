#ifndef SymbolsH
#define SymbolsH

#include <string>
#include <memory>
#include "token.h"

/**
*	Symbols function and classes
*
**/
//Forward declaration
class AsmFile;	//Declared in asm.h - forward delcared so that we can define friend relations
class Symbol; //Symbols definition
class Scope; //Scopes
class ScopeStack; //Scope Stack

/*
 * Symbols
 * 
 * */
class Symbol{
public:
	friend class AsmFile;
	
	//Type of symbols
	enum Type_T{
		Constant,
		Variable,
		Function,
		Procedure, 
		Typename
	};
	
protected:	//Consturctor is protected so that no one but AsmFile can instantiate
	/** Methods **/
	//OCCF
	Symbol();
	~Symbol();
	Symbol(const Symbol &obj);
	Symbol operator=(const Symbol &obj);
	
	/** Data Members **/
	std::string ID;		//Identifier
	bool IDUser;		//Whether ID is user defined
	Type_T Type;
	std::shared_ptr<Token> Value;	//Token storing the value
	
};

#endif