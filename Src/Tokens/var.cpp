#include "var.h"

Token_Var::Token_Var(std::string id, std::shared_ptr<Token_Type> type):
	Token(StringToLower(id), _Variable, false), Type(type)
{
	//...
}

Token_Var::Token_Var(const Token_Var &obj):
	Token(obj), value(obj.value), Type(obj.Type), Sym(obj.Sym)
{
}

Token_Var Token_Var::operator=(const Token_Var &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
		Type = obj.Type;
		Sym = obj.Sym;
	}
	
	return *this;
}