#include "func.h"

Token_Func::Token_Func(std::string ID, Token_Func::Type_T type): 
	Token(ID, FuncProc), Type(type)
{

}

Token_Func::Token_Func(const Token_Func &obj):
	Token(obj), Type(obj.Type), Params(obj.Params)
{
	
}

Token_Func Token_Func::operator=(const Token_Func &obj){
	if (&obj != this){
		Token::operator=(obj);
		Type = obj.Type;
		Params = obj.Params;
	}
	return *this;
}