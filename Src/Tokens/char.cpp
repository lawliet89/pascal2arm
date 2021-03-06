#include "char.h"
#include "../utility.h"
#include <iostream>


Token_Char::Token_Char(char value, bool IsConstant): 
	Token(ToString<char>(value), V_Character, IsConstant), value(value)
{
}
 
Token_Char::Token_Char(const Token_Char &obj):
	Token(obj), value(value)
	{}
	
Token_Char Token_Char::operator=(const Token_Char &obj){
	if (this != &obj){
		Token::operator=(obj);
		value = obj.value;
	}
	
	return *this;
}

std::string Token_Char::AsmValue(){
	return "#" + ToString<unsigned>((unsigned) value);
}

bool Token_Char::operator==(const Token_Char& obj) const
{
	return value == obj.value;
}

bool Token_Char::operator!=(const Token_Char& obj) const
{
	return value != obj.value;
}


bool Token_Char::operator==(const Token& obj) const
{
    return operator==(dynamic_cast<const Token_Char &>(obj));
}

bool Token_Char::operator!=(const Token& obj) const
{
	return operator!=(dynamic_cast<const Token_Char &>(obj));
}
