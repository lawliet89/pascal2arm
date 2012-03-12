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
		Token::operator=(obj);
		Params = obj.Params;
	}
	return *this;
}


void Token_FormalParam::Merge(Token_FormalParam& obj)
{
	std::vector<Param_T>::iterator first, last, position;
	position = Params.end();
	first = obj.Params.begin();
	last = obj.Params.end();
	
	Params.insert(position, first, last);
}

void Token_FormalParam::AddParams(std::vector<std::shared_ptr<Token_Var> > list, bool Required, bool Reference)
{
	std::vector<std::shared_ptr<Token_Var> >::iterator it = list.begin();
	for (; it < list.end(); it++){
		Params.push_back(Param_T(*it, Required, Reference));
	}
}
/*
bool Token_FormalParam::operator==(const Token_FormalParam& obj) const
{
	return Params == obj.Params;
}

bool Token_FormalParam::operator!=(const Token_FormalParam& obj) const
{
	return Params != obj.Params;
}


bool Token_FormalParam::operator==(const Token& obj) const
{
    return operator==(dynamic_cast<const Token_FormalParam &>(obj));
}

bool Token_FormalParam::operator!=(const Token& obj) const
{
	return operator!=(dynamic_cast<const Token_FormalParam &>(obj));
}

*/