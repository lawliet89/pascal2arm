#include "asm.h"
#include "utility.h"
#include "op.h"
#include <iostream>
#include <set>

extern Flags_T Flags;
extern std::stringstream OutputString;		//op.cpp

/**
 * 	AsmFile
 * */

//Constructor
AsmFile::AsmFile(){
	//Create reserved symbols - TODO
	std::shared_ptr<Symbol> sym;

	//Create reserved types
	sym = CreateTypeSymbol("integer", Token_Type::Integer);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("real", Token_Type::Real);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("boolean", Token_Type::Boolean);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("record", Token_Type::Record);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("enum", Token_Type::Enum);
	sym -> SetReserved();

	sym = CreateTypeSymbol("char", Token_Type::Char);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("string", Token_Type::String);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("file", Token_Type::File);
	sym -> SetReserved();
	
	sym = CreateTypeSymbol("set", Token_Type::Set);
	sym -> SetReserved();
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
std::shared_ptr<Symbol> AsmFile::CreateSymbol(Symbol::Type_T type, std::string id, std::shared_ptr<Token> value, std::shared_ptr<AsmBlock> block) throw(int){
	id = StringToLower(id);		//Case insensitive
	if (CheckSymbol(id))
		throw ASM_SymbolExists;
	std::shared_ptr<Symbol> ptr( new Symbol (type, id, value, block));
	
	SymbolList[id] = ptr;
	return ptr;
	
}

//Check if symbol with id exists - true if exists, false otherwise
bool AsmFile::CheckSymbol(std::string id){
	id = StringToLower(id);	//Case insensitive
	std::map<std::string, std::shared_ptr<Symbol> >::iterator it;
	it = SymbolList.find(id);
	
	if (it == SymbolList.end())
		return false;
	else
		return true;
}

//Will return to caller for caller to set additional parameters
//i.e. sym -> GetTokenDerived<Token_Type *>()
std::shared_ptr<Symbol> AsmFile::CreateTypeSymbol(std::string id, Token_Type::P_Type pri, int sec) throw(int){
	id = StringToLower(id);
	Token_Type *ptr = new Token_Type(id, pri, sec);
	
	//TODO Handle Blocks
	std::shared_ptr<Symbol> sym = CreateSymbol(Symbol::Typename, id, std::shared_ptr<Token>(dynamic_cast<Token *>(ptr)));
	TypeList[id] = sym;
	
	return sym;
}

//Create symbol from list
void AsmFile::CreateVarSymbolsFromList(const Token_IDList &listToken, int PrimaryType, int SecondaryType, std::shared_ptr<Token> value) throw(int){
	std::set<std::string> list = listToken.GetList();
	
	std::set<std::string>::iterator it;
	
	for (it=list.begin() ; it != list.end(); it++ ){
		Token_Var * ptr = new Token_Var(*it);
		ptr -> SetPrimaryType(PrimaryType);
		ptr -> SetSecondaryType(SecondaryType);
		ptr -> SetValue(value);
		//TODO Handle current block
		std::shared_ptr<Symbol> sym = CreateSymbol(Symbol::Variable, *it, std::shared_ptr<Token>(dynamic_cast<Token *>(ptr)));	
		ptr -> SetSymbol(sym);
	}
		
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