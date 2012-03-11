#include "factor.h"

Token_Factor::Token_Factor(Token_Factor::Form_T Form, std::shared_ptr< Token > value, std::shared_ptr<Token_Type> Type): 
	Token("factor", Factor, false), Form(Form), value(value), Type(Type), Negate(false)
{

}

Token_Factor::Token_Factor(const Token_Factor & obj): 
	Token(obj), Form(obj.Form), value(obj.value), Type(obj.Type), Negate(obj.Negate), FuncToken(obj.FuncToken)
{

}

Token_Factor Token_Factor::operator=(const Token_Factor& obj)
{
	if (this != &obj){
		Token::operator=(obj);
		Form = obj.Form;
		value = obj.value;
		Type = obj.Type;
		Negate = obj.Negate;
		FuncToken = obj.FuncToken;
	}
	
	return *this;
}

std::shared_ptr<Token_Type> Token_Factor::GetType(){
	if (Form == Expression)
		return std::static_pointer_cast<Token_Expression>(value) -> GetType();
	else
		return Type;
}