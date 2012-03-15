#include "exprlist.h"
Token_ExprList::Token_ExprList(): Token("expression list", Expression, false){
	
}
Token_ExprList::Token_ExprList(std::shared_ptr<Token_Expression> expr):
	Token("expression list", Expression, false), list(1, expr)
{}

Token_ExprList::Token_ExprList(const Token_ExprList& obj): 
	Token(obj), list(obj.list)
{

}

Token_ExprList Token_ExprList::operator=(const Token_ExprList& obj)
{
	if (&obj != this){
		Token::operator=(obj);
		list = obj.list;
	}
	
	return *this;
}
