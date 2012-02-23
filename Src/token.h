#ifndef TokenH
#define TokenH

#include <string>

class Token;
#define YYSTYPE std::shared_ptr<Token>


/*
 * Token base class that can be derived for the various data types
 * 
 * */

class Token{		//Base class is sufficient for normal strings
public:
	//OCCF
	
	//Constructor
	Token(const char *StrValue, int type);	//Type is the token type as defined by parser.h (generated from parser.y)
	Token(std::string StrValue, int type);	
	virtual ~Token(){ }				//Destructor
	Token(const Token &obj);		//Copy Constructor
	virtual Token operator=(const Token &obj);	//Assignment operator
	
	//Virtual methods
	virtual std::string GetStrValue() const { return StrValue; }
	virtual const char* GetCStrValue() const { return StrValue.c_str(); }
	int GetType() const { return type; }
	
	//Return type is set to void * so that derived classes can override this
	//Use the utility function DereferenceVoidPtr<T> to dereference this
	//where T is the intended type
	virtual const void * GetValue() const { return (void *) &StrValue; }
	virtual const void * operator()() const { return (void *) &StrValue; }
protected:
	std::string StrValue;
	int type;
};

/**
 * 	Non terminal symbols definition
 * 
 * */

#define Signed_Int -1
#define Signed_Real -2

#define Identifier -3


#if defined IN_BISON || defined IN_FLEX
#include "Gen/all.h"	//All specialisations
#endif

#endif