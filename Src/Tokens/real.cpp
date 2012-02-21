#include "real.h"
#include "../utility.h"

Token_Real::Token_Real(char const *StrValue, yytokentype type) : Token(StrValue, type), value(FromString<double>(StrValue)){
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
