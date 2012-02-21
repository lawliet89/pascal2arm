#include "token.h"

//Constructors
Token::Token(const char *StrValue, int type): StrValue(StrValue), type(type){
	//...
}

//Copy constructor
Token::Token (const Token &obj){
	StrValue = obj.StrValue;
	type = obj.type;
}

//Assignment operator
Token Token::operator=(const Token &obj){
	//Handles self assignment
	if (&obj != this){
		StrValue = obj.StrValue;
		type = obj.type;
	}
	
	return *this;
}