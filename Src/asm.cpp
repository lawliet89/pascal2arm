#include "asm.h"
#include "utility.h"
#include "op.h"
#include <iostream>

extern Flags_T Flags;
extern std::stringstream OutputString;		//op.cpp

/**
 * 	AsmFile
 * */

//Constructor
AsmFile::AsmFile(){
	//Create reserved symbols - TODO
	
	
	
}

//Destructor
AsmFile::~AsmFile(){
	//...
}

AsmFile::AsmFile(const AsmFile &obj): 
	CodeLines(obj.CodeLines),
	DataLines(obj.DataLines),
	FunctionLines(obj.FunctionLines)
{
	//operator=(obj);
}

AsmFile AsmFile::operator=(const AsmFile &obj){
	if (&obj != this){
		//Vector will do deep copying for us.
		CodeLines = obj.CodeLines;
		DataLines = obj.DataLines;
		FunctionLines = obj.FunctionLines;	
	}
	return *this;
}

//Create Symbol
std::shared_ptr<Symbol> AsmFile::CreateSymbol(Symbol::Type_T type, std::shared_ptr<Token> value, std::string id) throw(int){
	if (CheckSymbol(id))
		throw ASM_SymbolExists;
	std::shared_ptr<Symbol> ptr( new Symbol (type, value, id));
	
	SymbolList[id] = ptr;
	return ptr;
	
}

//Check if symbol with id exists - true if exists, false otherwise
bool AsmFile::CheckSymbol(std::string id){
	std::map<std::string, std::shared_ptr<Symbol> >::iterator it;
	it = SymbolList.find(id);
	
	if (it == SymbolList.end())
		return false;
	else
		return true;
}

//Generate Code
void AsmFile::GenerateCode(std::stringstream &output){
	
	try{
		output << ReadFile(Flags.AsmHeaderPath.c_str());
	}
	catch(...){
		HandleError("Header file for assembly does not exist.", E_GENERIC, E_FATAL);
	}
	//User data variables etc.
	//Std Libary
	output << ReadFile(Flags.AsmStdLibPath.c_str());
}

/**
 * 	AsmLine
 * */
AsmLine::AsmLine(){
	//...
}

AsmLine::~AsmLine(){
	//...
}

AsmLine::AsmLine(const AsmLine &obj):
	OpCode(obj.OpCode),
	Condition(obj.Condition),
	Qualifier(obj.Qualifier),
	Type(obj.Type)
{
	//...
}

AsmLine AsmLine::operator=(const AsmLine &obj){
	if (&obj != this){
		OpCode = obj.OpCode;
		Condition = obj.Condition;
		Qualifier = obj.Qualifier;
		Type = obj.Type;
	}
	return *this;
}

/**
 * 	AsmOp
 * */

/**
 * 	AsmLabels
 * */