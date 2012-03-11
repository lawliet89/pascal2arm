#include "formalparam.h"

Token_FormalParam::Token_FormalParam():
	Token("formalparam", FormalParamList, true)
{

}
Token_FormalParam::Token_FormalParam(Token_FormalParam::Param_T param): Token("formalparam", FormalParamList, true), Params(1, param)
{

}

Token_FormalParam::Token_FormalParam(const Token_FormalParam& obj): Token(obj), Params(obj.Params)
{

}

Token_FormalParam Token_FormalParam::operator=(const Token_FormalParam& obj)
{
	if (this != &obj){
		Params = obj.Params;
	}
	return *this;
}

