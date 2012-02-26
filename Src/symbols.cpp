#include "symbols.h"

/** Constructor **/
Symbol::Symbol(Type_T type, std::shared_ptr<Token> value, std::string id): Type(type), Value(value), ID(id){
	//...
}
Symbol::Symbol(const Symbol &obj):
	ID(obj.ID),
	Type(obj.Type),
	Value(obj.Value)
{
	//...
}

Symbol Symbol::operator=(const Symbol &obj){
	if (&obj != this){
		ID = obj.ID;
		Type = obj.Type;
		Value = obj.Value;
	}
	return *this;
}

/** Destructor **/
Symbol::~Symbol(){
	
}

