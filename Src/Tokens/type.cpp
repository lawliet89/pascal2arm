#include "type.h"
#include "../define.h"

Token_Type::Token_Type(std::string id, Token_Type::P_Type pri, int sec, int size):
	Token(StringToLower(id), Type, true), Primary(pri), Secondary(sec), size(size)
{
	//...
}

Token_Type::Token_Type(const Token_Type &obj):
	Token(obj), Primary(obj.Primary), Secondary(obj.Secondary), size(obj.size)
{
	//...
}

Token_Type Token_Type::operator=(const Token_Type &obj){
	if (&obj != this){
		Token::operator=(obj);
		Primary = obj.Primary;
		Secondary = obj.Secondary;
		size = obj.size;
	}
	
	return *this;
}

std::string Token_Type::AsmDefaultValue(){
	switch(Primary){
		case Integer:
			return "0";
			break;
		case Real:
			return "0";
			break;
		case Boolean:
			return "0";
			break;
		case Char:
			return "0";
			break;
		default:
			return "";
	}
}