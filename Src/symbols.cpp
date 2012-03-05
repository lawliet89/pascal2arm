#include "symbols.h"

/** Constructor **/
Symbol::Symbol(Type_T type, std::string id, std::shared_ptr<Token> value,std::shared_ptr<AsmBlock> block): 
Type(type), ID(StringToLower(id)), Value(value), Reserved(false), Block(block)
{
	//...
}
Symbol::Symbol(const Symbol &obj):
	ID(obj.ID),
	Type(obj.Type),
	Value(obj.Value),
	Reserved(obj.Reserved),
	Block(obj.Block)
{
	//...
}

Symbol Symbol::operator=(const Symbol &obj){
	if (&obj != this){
		ID = obj.ID;
		Type = obj.Type;
		Value = obj.Value;
		Reserved = obj.Reserved;
		Block = obj.Block;
	}
	return *this;
}

/** Destructor **/
Symbol::~Symbol(){
	
}

