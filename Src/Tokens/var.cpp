#include "var.h"

Token_Var::Token_Var(std::string id):
	Token(StringToLower(id), _Variable, false)
{
	//...
}

Token_Var::Token_Var(const Token_Var &obj):
	Token(obj), value(obj.value), PrimaryType(obj.PrimaryType), SecondaryType(obj.SecondaryType),
	Sym(obj.Sym)
{
}

Token_Var Token_Var::operator=(const Token_Var &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
		PrimaryType = obj.PrimaryType;
		SecondaryType = obj.SecondaryType;
		Sym = obj.Sym;
	}
	
	return *this;
}