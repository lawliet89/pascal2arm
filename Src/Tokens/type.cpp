#include "type.h"
#include "../define.h"
#include <sstream>

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

std::string Token_Type::TypeToString(){
	std::stringstream str;
	switch(Primary){
		case Integer:
			str << "integer"; break;
		case Real:
			str << "real"; break;
		case Boolean:
			str << "boolean"; break;
		case Record:
			str << "record"; break;
		case Enum:
			str << "enum"; break;
		case Char:
			str << "char"; break;
		case String:
			str << "string"; break;
		case File:
			str << "file"; break;
		case Set:
			str << "set"; break;
		case Void:
			str << "void"; break;
		default:
			str << "unknown";
	}
	return str.str();
}

bool Token_Type::operator==(const Token_Type &obj){
	return (obj.Primary == Primary && obj.Secondary == Secondary);
}
bool Token_Type::operator!=(const Token_Type &obj){
	return !operator==(obj);
}