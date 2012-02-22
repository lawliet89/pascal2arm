#include "real.h"
#include "../utility.h"

Token_Real::Token_Real(char const *StrValue, int type) : Token(StrValue, type), value(FromString<double>(StrValue)){
	//...
}

Token_Real::Token_Real(std::string StrValue, int type) : Token(StrValue, type), value(FromString<double>(StrValue)){
	//...
}


Token_Real::Token_Real(double value, int type) : Token(ToString<double>(value).c_str(), type), value(value){
	//...
}


Token_Real::Token_Real(const Token_Real &obj): Token(obj), value(obj.value){
	//...
}

Token_Real Token_Real::operator=(const Token_Real &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
	}
	return *this;
}
