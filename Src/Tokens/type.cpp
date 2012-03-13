#include "type.h"
#include "../define.h"
#include <sstream>

Token_Type::Token_Type(std::string id, Token_Type::P_Type pri, int sec, int size):
	Token(StringToLower(id), Type, true), Primary(pri), Secondary(sec), size(size), range(0,0)
{
	CleanSecondary();
}

Token_Type::Token_Type(const Token_Type &obj):
	Token(obj), Primary(obj.Primary), Secondary(obj.Secondary), size(obj.size), range(obj.range), ArrayDimension(obj.ArrayDimension)
{
	CleanSecondary();
}

Token_Type Token_Type::operator=(const Token_Type &obj){
	if (&obj != this){
		Token::operator=(obj);
		Primary = obj.Primary;
		Secondary = obj.Secondary;
		size = obj.size;
		range = obj.range;
		ArrayDimension = obj.ArrayDimension;
	}
	CleanSecondary();
	return *this;
}

std::string Token_Type::AsmDefaultValue(){
	switch(Primary){
		case Integer:
			return "0";
			break;
		case Real:
			return "0";
			break;
		case Boolean:
			return "0";
			break;
		case Char:
			return "0";
			break;
		default:
			return "";
	}
}

std::string Token_Type::TypeToString(){
	std::stringstream str;
	if (IsPointer())
		str << "pointer to ";
	switch(Primary){
		case Integer:
			str << "integer"; break;
		case Real:
			str << "real"; break;
		case Boolean:
			str << "boolean"; break;
		case Record:
			str << "record"; break;
		case Enum:
			str << "enum"; break;
		case Char:
			str << "char"; break;
		case String:
			str << "string"; break;
		case File:
			str << "file"; break;
		case Set:
			str << "set"; break;
		case Void:
			str << "void"; break;
		default:
			str << "unknown";
	}
	if (IsArray()){
		str << " array of size ";		//TODO
	}
	return str.str();
}

bool Token_Type::operator==(const Token_Type &obj) const{
	if (Primary != obj.Primary)
		return false;
	//Check pointer
	if (IsPointer() != obj.IsPointer())
		return false;
	
	//Subrange we can ignore
		
	//Check array
	if (IsArray() != obj.IsArray())
		return false;
	
	return true;
}
bool Token_Type::operator!=(const Token_Type &obj) const{
	return !operator==(obj);
}

bool Token_Type::operator==(const Token& obj) const
{
	try{
		return operator==(dynamic_cast<const Token_Type &>(obj));
	}
	catch (std::bad_cast e){
		return false;
	}
	
}

bool Token_Type::operator!=(const Token& obj) const
{
	try{
		return operator!=(dynamic_cast<const Token_Type &>(obj));
	}
	catch (std::bad_cast e){
		return false;
	}
	
}



void Token_Type::SetArray(bool val)
{
	if (val){
		Secondary = Secondary | (unsigned) Array;
	}
	else{
		Secondary = Secondary & (unsigned) !Array;
	}
	CleanSecondary();
		
}

void Token_Type::SetPointer(bool val)
{
	if (val){
		Secondary = Secondary | (unsigned) Pointer;
	}
	else{
		Secondary = Secondary & (unsigned) !Pointer;
	}
	CleanSecondary();
		
}

void Token_Type::SetSubrange(bool val)
{
	if (val){
		Secondary = Secondary | (unsigned) Subrange;
	}
	else{
		Secondary = Secondary & (unsigned) !Subrange;
	}
	CleanSecondary();
		
}

void Token_Type::CleanSecondary()
{
	Secondary = Secondary & 7u;
}

std::pair< int, int > Token_Type::GetArrayDimensionBound(unsigned int n) const throw(AsmCode) 
{
	if (n >= ArrayDimension.size())
		throw ArrayDimensionOutOfBound;
	
	return ArrayDimension.at(n);
}

unsigned Token_Type::SetArrayDimensionBound(std::pair< int, int > bound) throw(AsmCode)
{
	if (bound.first >= bound.second)
		throw ArrayBoundInvalid;
	ArrayDimension.push_back(bound);
	return ArrayDimension.size()-1;
}

unsigned int Token_Type::GetArrayDimensionSize(unsigned int dimension) const throw(AsmCode)
{
	if (dimension >= ArrayDimension.size())
		throw ArrayDimensionOutOfBound;
	
	std::pair<int, int> bounds = ArrayDimension.at(dimension);
	return bounds.second-bounds.first+1;
}
