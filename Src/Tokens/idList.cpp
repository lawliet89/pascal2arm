#include "idList.h"

//Constructors
Token_IDList::Token_IDList(char const *StrValue):
	Token(StrValue, Identifier,true)
{
	list.insert(std::string(StrValue));
}

Token_IDList::Token_IDList(std::string StrValue):
Token(StrValue, Identifier,true)
{
	list.insert(StrValue);
}

Token_IDList::Token_IDList(std::shared_ptr<Token> token):
Token(StrValue, Identifier,true)
{
	list.insert(token->GetStrValue());
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
void Token_IDList::AddID(std::string id) throw(int){
	std::pair< std::set<std::string>::iterator, bool> result;
	result = list.insert(id);
	if (!result.second)
		throw ASM_SymbolExists;
}

void Token_IDList::AddID(std::shared_ptr<Token> token) throw(int){
	return AddID(token -> GetStrValue());
}