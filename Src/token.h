#ifndef TokenH
#define TokenH

#include <string>

class Token;
#define YYSTYPE Token *

//Because parser files are generated
#include "Gen/parser.h"

/*
 * Token base class that can be derived for the various data types
 * 
 * */

class Token{		//Base class is sufficient for normal strings
public:
	//OCCF
	
	//Constructor
	Token(const char *StrValue, yytokentype type);	//Type is the token type as defined by parser.h (generated from parser.y)
	virtual ~Token(){ }				//Destructor
	Token(const Token &obj);		//Copy Constructor
	Token operator=(const Token &obj);	//Assignment operator
	
	//Virtual methods
	virtual std::string GetStrValue() const { return StrValue; }
	yytokentype GetType() const { return type; }
	
protected:
	std::string StrValue;
	yytokentype type;
};

#if defined IN_BISON || defined IN_FLEX
#include "Gen/all.h"	//All specialisations
#endif

#endif