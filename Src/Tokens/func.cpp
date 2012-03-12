#include "func.h"

Token_Func::Token_Func(std::string ID, Token_Func::Type_T type): 
	Token(ID, FuncProc), Type(type), HasReturn(false)
{

}

Token_Func::Token_Func(const Token_Func &obj):
	Token(obj), ReturnType(obj.ReturnType), Params(obj.Params), block(obj.block), sym(obj.sym), HasReturn(obj.HasReturn)
{
	
}

Token_Func Token_Func::operator=(const Token_Func &obj){
	if (&obj != this){
		Token::operator=(obj);
		ReturnType = obj.ReturnType;
		Params = obj.Params;
		block = obj.block;
		sym = obj.sym;
		HasReturn = obj.HasReturn;
	}
	return *this;
}
