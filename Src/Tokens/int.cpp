#include "int.h"
#include "../utility.h"

Token_Int::Token_Int(const char *StrValue, T_Type type, bool IsConstant) : 
	Token(StrValue, type, IsConstant), value(FromString<int>(StrValue)){
	//...
}

Token_Int::Token_Int(std::string StrValue, T_Type type, bool IsConstant) : 
Token(StrValue, type, IsConstant), value(FromString<int>(StrValue)){
	//...
}

Token_Int::Token_Int(int value, T_Type type) : Token(ToString<int>(value).c_str(), type), value(value){
	//...
}

Token_Int::Token_Int(const Token_Int &obj): Token(obj), value(obj.value){
	//...
}

Token_Int Token_Int::operator=(const Token_Int &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
	}
	return *this;
}

std::string Token_Int::AsmValue(){
	return ToString<int>(value);
}