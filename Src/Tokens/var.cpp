#include "var.h"

Token_Var::Token_Var(std::string id, std::shared_ptr<Token_Type> type, bool constant, bool temp):
	Token(StringToLower(id), Variable, constant), Type(type), Temp(temp), Dereference(false)
{
	//...
}

Token_Var::Token_Var(const Token_Var &obj):
	Token(obj), value(obj.value), Type(obj.Type), Sym(obj.Sym), Temp(obj.Temp), Dereference(obj.Dereference), Index(obj.Index)
{
}

Token_Var Token_Var::operator=(const Token_Var &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
		Type = obj.Type;
		Sym = obj.Sym;
		Temp = obj.Temp;
		Dereference = obj.Dereference;
		Index = obj.Index;
	}
	
	return *this;
}
bool Token_Var::operator==(const Token_Var &obj) const{
	if (GetStrValue() != obj.GetStrValue())
		return false;
	
	bool here, there;
	here = Type == nullptr;
	there = obj.Type == nullptr;
	
	if (here ^ there)
		return false;
	
	if (!here && *Type != *(obj.Type))
		return false;
	
	//Check that both have dereference flags
	if (Dereference != obj.Dereference)
		return false;
	
	return true;
}
bool Token_Var::operator!=(const Token_Var &obj) const{
	return !operator==(obj);
}

bool Token_Var::operator==(const Token& obj) const
{
    return operator==(dynamic_cast<const Token_Var &>(obj));
}

bool Token_Var::operator!=(const Token& obj) const
{
	return operator!=(dynamic_cast<const Token_Var &>(obj));
}