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

void AsmFile::CreateGlobalScope(){
	//TODO Global scope recreation check
	
	//Create Global scope
	std::shared_ptr<AsmBlock> global = CreateBlock(AsmBlock::Global);
	PushBlock(global);		//Set as current block
	
	//Create reserved symbols - TODO
	std::pair<std::shared_ptr<Symbol>, AsmCode> sym;

	//Create reserved types
	sym = CreateTypeSymbol("integer", Token_Type::Integer);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("real", Token_Type::Real);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("boolean", Token_Type::Boolean);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("record", Token_Type::Record);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("enum", Token_Type::Enum);
	sym.first -> SetReserved();

	sym = CreateTypeSymbol("char", Token_Type::Char);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("string", Token_Type::String);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("file", Token_Type::File);
	sym.first -> SetReserved();
	
	sym = CreateTypeSymbol("set", Token_Type::Set);
	sym.first -> SetReserved();	
	
}

/** Block Methods **/
std::shared_ptr<AsmBlock> AsmFile::GetCurrentBlock(){
	return BlockStack.back();
}

void AsmFile::PushBlock(std::shared_ptr<AsmBlock> block){
	BlockStack.push_back(block);
}

void AsmFile::PopBlock(){
	BlockStack.pop_back();
}


std::shared_ptr<AsmBlock> AsmFile::CreateBlock(AsmBlock::Type_T type){
	//TODO Global scope recreation check
	std::vector<std::shared_ptr<AsmBlock> >::iterator it;
	it = BlockList.begin();
	
	std::shared_ptr<AsmBlock> ptr(new AsmBlock(type));
	BlockList.insert(it, ptr);
	
	return ptr;
}

/** Symbols Methods **/

//Check if symbol with id exist
AsmCode AsmFile::CheckSymbol(std::string id){
	id = StringToLower(id);	//Case insensitive
	std::shared_ptr<AsmBlock> working, current;
	std::vector<std::shared_ptr<AsmBlock> >::reverse_iterator it;
	
	current = GetCurrentBlock();
	for (it = BlockStack.rbegin(); it < BlockStack.rend(); it++){
		working = *it;
		AsmCode status = working-> CheckSymbol(id);
		if (status == SymbolReserved){
			return SymbolReserved;
		}
		if (status == SymbolExistsInCurrentBlock){
			if (current == working)
				return SymbolExistsInCurrentBlock;
			else
				return SymbolExistsInOuterBlock;
		}
	}
	
	return SymbolNotExists;
}

std::pair<std::shared_ptr<Symbol>, AsmCode> AsmFile::GetSymbol(std::string id) throw(AsmCode){
	id = StringToLower(id);
	std::pair<std::shared_ptr<Symbol>, AsmCode> result;
	std::shared_ptr<AsmBlock> working, current;
	std::vector<std::shared_ptr<AsmBlock> >::reverse_iterator it;
	
	current = GetCurrentBlock();
	for (it = BlockStack.rbegin(); it < BlockStack.rend(); it++){
		working = *it;
		try{
			result.first = working->GetSymbol(id);
			if (working == current)
				result.second = SymbolExistsInCurrentBlock;
			else
				result.second = SymbolExistsInOuterBlock;
			
			return result;
		}
		catch(AsmCode e){
			//.. carry on
		}
	}
	
	throw SymbolNotExists;
}

//Create Symbol
std::pair<std::shared_ptr<Symbol>, AsmCode> AsmFile::CreateSymbol(Symbol::Type_T type, std::string id, std::shared_ptr<Token> value) throw(AsmCode){
	id = StringToLower(id);		//Case insensitive
	std::pair<std::shared_ptr<Symbol>, AsmCode> result;
	std::shared_ptr<AsmBlock> current = GetCurrentBlock();
	
	AsmCode SymbolStatus = CheckSymbol(id);
	if (SymbolStatus == SymbolReserved)
		throw SymbolReserved;
	
	if (SymbolStatus == SymbolExistsInCurrentBlock)
		throw SymbolExistsInCurrentBlock;
	
	std::shared_ptr<Symbol> ptr( new Symbol (type, id, value, current));
	current -> AssignSymbol(ptr);
	
	result.first = ptr;
	
	if (SymbolStatus == SymbolExistsInOuterBlock)
		result.second = SymbolOccludes;	//Occlusion has occurred.
	else
		result.second = SymbolCreated;
	
	return result;
}


//Create type symbol in the current block
//Will return to caller for caller to set additional parameters
//i.e. sym -> GetTokenDerived<Token_Type *>()
std::pair<std::shared_ptr<Symbol>, AsmCode> AsmFile::CreateTypeSymbol(std::string id, Token_Type::P_Type pri, int sec) throw(AsmCode){
	id = StringToLower(id);
	Token_Type *ptr = new Token_Type(id, pri, sec);
	
	std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Typename, id, std::shared_ptr<Token>(dynamic_cast<Token *>(ptr)));
	return sym;
}

std::shared_ptr<Symbol> AsmFile::GetTypeSymbol(std::string id) throw(AsmCode){
	//TODO Check symbol stack
	id = StringToLower(id);	//Case insensitive
	
	
}
/*
//Create symbol from list
void AsmFile::CreateVarSymbolsFromList(const Token_IDList &listToken, int PrimaryType, int SecondaryType, std::shared_ptr<Token> value) throw(AsmCode){
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
*/
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

/**
 * 	AsmBlock
 * 
 * */
AsmBlock::AsmBlock(AsmBlock::Type_T type): Type(type)
{
	//...
}

AsmBlock::AsmBlock(const AsmBlock& obj): 
	SymbolList(obj.SymbolList), Type(obj.Type)
{

}

AsmBlock AsmBlock::operator=(const AsmBlock &obj){
	if (&obj != this){
		Type = obj.Type;
		SymbolList = obj.SymbolList;
		//SymbolStart = obj.SymbolStart;
	}
	return *this;
}

void AsmBlock::AssignSymbol(std::shared_ptr<Symbol> sym) throw(AsmCode){
	SymbolList.insert(std::pair<std::string, std::shared_ptr<Symbol> >(sym -> GetID(), sym));
}

std::shared_ptr<Symbol> AsmBlock::GetSymbol(std::string id) throw(AsmCode){
	id = StringToLower(id);
	std::map<std::string, std::shared_ptr<Symbol> >::iterator it;
	it = SymbolList.find(id);
	
	if (it == SymbolList.end()){
		throw SymbolNotExists;
	}
	return (*it).second;
}

AsmCode AsmBlock::CheckSymbol(std::string id) throw(){
	id = StringToLower(id);
	std::map<std::string, std::shared_ptr<Symbol> >::iterator it;
	it = SymbolList.find(id);
	
	if (it == SymbolList.end()){
		return SymbolNotExists;
	}
	else{
		if ((*it).second -> IsReserved())
			return SymbolReserved;
		else
			return SymbolExistsInCurrentBlock;
	}
}