#include "var.h"

Token_Var::Token_Var(std::string id, std::shared_ptr<Token_Type> type, bool constant, bool temp):
	Token(StringToLower(id), Variable, constant), Type(type), Temp(temp)
{
	//...
}

Token_Var::Token_Var(const Token_Var &obj):
	Token(obj), value(obj.value), Type(obj.Type), Sym(obj.Sym), Temp(obj.Temp)
{
}

Token_Var Token_Var::operator=(const Token_Var &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
		Type = obj.Type;
		Sym = obj.Sym;
		Temp = obj.Temp;
	}
	
	return *this;
}