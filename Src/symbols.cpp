#include "symbols.h"

/** Constructor **/
Symbol::Symbol(Type_T type, std::string id, std::shared_ptr<Token> value,std::shared_ptr<AsmBlock> block): 
Type(type), ID(StringToLower(id)), Value(value), Reserved(false), Block(block), Temporary(false)
{
	//...
}
Symbol::Symbol(const Symbol &obj):
	ID(obj.ID),
	Type(obj.Type),
	Value(obj.Value),
	Reserved(obj.Reserved),
	Block(obj.Block),
	Temporary(obj.Temporary),
	Label(obj.Label)
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
		Temporary = obj.Temporary;
		Label = obj.Label;
	}
	return *this;
}

/** Destructor **/
Symbol::~Symbol(){
	
}

bool Symbol::operator==(const Symbol& obj) const{
	//We are actually comparing the tokens
	if (Value == nullptr || obj.Value == nullptr)
		return ID == obj.ID;
	else{
		if (ID != obj.ID)
			return false;
		if (Value == nullptr && obj.Value != nullptr)
			return false;
		if (obj.Value == nullptr && Value != nullptr)
			return false;
		return *Value == *(obj.Value);
	}
}

bool Symbol::operator!=(const Symbol& obj) const
{
	return !operator==(obj);
}



