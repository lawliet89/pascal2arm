#include "varList.h"

Token_VarList::Token_VarList():
	Token("VarList", _VarList, false)
{
	
}

Token_VarList::Token_VarList(std::shared_ptr<Token_Var> &ptr):
	Token("VarList", _VarList, false)
{
	list.push_back(ptr);
}

Token_VarList::Token_VarList(const Token_VarList &obj):
	Token(obj), list(obj.list)
	{}
	
Token_VarList Token_VarList::operator=(const Token_VarList &obj){
	if (this != &obj){
		Token::operator=(obj);
		list = obj.list;
	}
	
	return *this;
}

void Token_VarList::AddVariable(std::shared_ptr<Token_Var> &ptr){
	list.push_back(ptr);
}