#ifndef TokenH
#define TokenH

#include <string>

class Token;
#define YYSTYPE Token *
#include "parser.h"

/*
 * Token base class that can be derived for the various data types
 * 
 * */

class Token{
public:
	//OCCF
	
	//Constructor
	Token(const char *StrValue, yytokentype type);	//Type is the token type as defined by parser.h (generated from parser.y)
	~Token(){ }				//Destructor
	Token(const Token &obj);		//Copy Constructor
	Token operator=(const Token &obj);	//Assignment operator
	
	//Virtual methods
	virtual std::string GetStrValue() const { return StrValue; }
	yytokentype GetType() const { return type; }
	
protected:
	std::string StrValue;
	yytokentype type;
};

#include "all.h"	//All specialisations

#endif