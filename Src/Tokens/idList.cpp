#include "idList.h"

//Constructors
Token_IDList::Token_IDList(std::shared_ptr<Token> token):
Token(token->GetStrValue(), _IdentifierList,true)
{
	list[token->GetStrValue()] = token;
}

Token_IDList::Token_IDList(const Token_IDList &obj):
	Token(obj), list(list)
	{
	}

//Operator=
Token_IDList Token_IDList::operator=(const Token_IDList &obj){
	if (&obj != this){
		Token::operator=(obj);
		list = obj.list;
	}
	return *this;
}

//AddID
void Token_IDList::AddID(std::shared_ptr<Token> token) throw(AsmCode){
	std::pair< std::map <std::string, std::shared_ptr<Token> >::iterator, bool> result;
	result = list.insert(std::pair<std::string, std::shared_ptr<Token> >(token -> GetStrValue(), token));
	if (!result.second)
		throw SymbolExists;
	
}