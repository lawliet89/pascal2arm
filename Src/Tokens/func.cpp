#include "func.h"
#include "../asm.h"

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

bool Token_Func::operator==(const Token_Func &obj) const{
	//Compare block ID
	
	bool here = block == nullptr, there = obj.block == nullptr;
	if (here ^ there)
		return false;
	
	if (!here && *block != *(obj.block))
		return false;
	/*
	here = sym == nullptr;
	there = obj.sym == nullptr;
	
	if (here ^ there)
		return false;
	
	if (!here && sym->GetID() != obj.sym -> GetID())
		return false;*/
	
	
	if (ReturnType != obj.ReturnType)
		return false;
	
	//No need to check for FormalParams since we don't do overloading
	
	
	return true;
}
bool Token_Func::operator!=(const Token_Func &obj) const{
	return !operator==(obj);
}

bool Token_Func::operator==(const Token& obj) const
{
	try{
		return operator==(dynamic_cast<const Token_Func &>(obj));
	}
	catch (std::bad_cast e){
		return false;
	}
}

bool Token_Func::operator!=(const Token& obj) const
{
	try{
		return operator!=(dynamic_cast<const Token_Func &>(obj));
	}
	catch (std::bad_cast e){
		return true;
	}
}