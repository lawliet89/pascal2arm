#include "var.h"

Token_Var::Token_Var(std::string id, std::shared_ptr<Token_Type> type, bool constant, bool temp):
	Token(StringToLower(id), Variable, constant), Type(type), Temp(temp), Dereference(false), Index(0), IndexSet(false)
{
	//...
}

Token_Var::Token_Var(const Token_Var &obj):
	Token(obj), value(obj.value), Type(obj.Type), Sym(obj.Sym), Temp(obj.Temp), Dereference(obj.Dereference), Index(obj.Index), IndexSet(obj.IndexSet)
{
}

Token_Var Token_Var::operator=(const Token_Var &obj){
	if (&obj != this){
		Token::operator=(obj);
		value = obj.value;
		Type = obj.Type;
		Sym = obj.Sym;
		Temp = obj.Temp;
		Dereference = obj.Dereference;
		Index = obj.Index;
		IndexSet = obj.IndexSet;
	}
	
	return *this;
}
bool Token_Var::operator==(const Token_Var &obj) const{
	if (GetStrValue() != obj.GetStrValue())
		return false;
	
	bool here, there;
	here = Type == nullptr;
	there = obj.Type == nullptr;
	
	if (here ^ there)
		return false;
	
	if (!here && *Type != *(obj.Type))
		return false;
	
	//Check that both have dereference flags
	if (Dereference != obj.Dereference)
		return false;
	
	return true;
}
bool Token_Var::operator!=(const Token_Var &obj) const{
	return !operator==(obj);
}

bool Token_Var::operator==(const Token& obj) const
{
	try{
		return operator==(dynamic_cast<const Token_Var &>(obj));
	}
	catch (std::bad_cast e){
		return false;
	}
}

bool Token_Var::operator!=(const Token& obj) const
{
	try{
		return operator!=(dynamic_cast<const Token_Var &>(obj));
	}
	catch (std::bad_cast e){
		return true;
	}
}

void Token_Var::SetIndex(unsigned int dimension, int val)  throw(AsmCode)
{
	//Check dimension
	if (dimension >= Type -> GetArrayDimensionCount())
		throw ArrayDimensionOutOfBound;
	
	//Get bound
	std::pair<int, int> bound = Type -> GetArrayDimensionBound(dimension);
	if (val < bound.first || val > bound.second)
		throw ArrayOutofBound;
	
	Index[dimension] = val;
}

int Token_Var::GetIndex(unsigned int dimension) const throw(AsmCode)
{
	//Check dimension
	if (dimension >= Type -> GetArrayDimensionCount())
		throw ArrayDimensionOutOfBound;
	
	return Index.at(dimension);
}

unsigned int Token_Var::GetIndexOffset()
{
	unsigned offset = 0;
	unsigned dimensions = Type -> GetArrayDimensionCount();
	unsigned i;
	for (i = 0; i < dimensions-1; i++){
		offset += Index.at(i)*Type -> GetArrayDimensionSize(i);
	}
	offset += Index.at(i);
	
	return offset;
}
