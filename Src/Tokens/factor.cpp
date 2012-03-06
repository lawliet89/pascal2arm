#include "factor.h"

Token_Factor::Token_Factor(Token_Factor::Form_T Form, std::shared_ptr< Token > value, std::shared_ptr<Token_Type> Type): 
	Token("factor", Factor, false), Form(Form), value(value), Type(Type)
{

}

Token_Factor::Token_Factor(const Token_Factor & obj): 
	Token(obj), Form(obj.Form), value(obj.value), Type(obj.Type)
{

}

Token_Factor Token_Factor::operator=(const Token_Factor& obj)
{
	if (this != &obj){
		Token::operator=(obj);
		Form = obj.Form;
		value = obj.value;
		Type = obj.Type;
	}
	
	return *this;
}
