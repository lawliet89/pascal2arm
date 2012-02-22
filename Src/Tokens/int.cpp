#include "int.h"
#include "../utility.h"

Token_Int::Token_Int(const char *StrValue, int type) : Token(StrValue, type), value(FromString<int>(StrValue)){
	//...
}

Token_Int::Token_Int(std::string StrValue, int type) : Token(StrValue, type), value(FromString<int>(StrValue)){
	//...
}

Token_Int::Token_Int(int value, int type) : Token(ToString<int>(value).c_str(), type), value(value){
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
