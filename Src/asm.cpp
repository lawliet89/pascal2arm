#include "asm.h"
#include "utility.h"
#include "op.h"
#include <iostream>
#include <set>
#include <climits>

extern Flags_T Flags;
extern std::stringstream OutputString;		//op.cpp
extern unsigned LexerCharCount, LexerLineCount;		//In lexer.l	- DEBUG purposes
extern AsmFile Program;		//elsewhere

/** Data Structures **/

std::map<AsmLine::OpCode_T, std::string> AsmLine::OpCodeStr;
std::map<AsmLine::CC_T, std::string> AsmLine::CCStr;

/**
 * 	AsmFile
 * */

//Constructor
AsmFile::AsmFile(){
	CreateGlobalScope();
}

//Destructor
AsmFile::~AsmFile(){
	//...
}

AsmFile::AsmFile(const AsmFile &obj): 
	CodeLines(obj.CodeLines),
	DataLines(obj.DataLines),
	FunctionLines(obj.FunctionLines)	//TODO Copy constructor
{
	//operator=(obj);
}

AsmFile AsmFile::operator=(const AsmFile &obj){
	if (&obj != this){		//TODO Assignment operator - make these private?
		//Vector will do deep copying for us.
		CodeLines = obj.CodeLines;
		DataLines = obj.DataLines;
		FunctionLines = obj.FunctionLines;	
	}
	return *this;
}

void AsmFile::CreateGlobalScope(){
	if (GlobalBlock != nullptr)
		throw;
	
	//Create Global scope
	std::shared_ptr<AsmBlock> global = CreateBlock(AsmBlock::Global);
	GlobalBlock = global;
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
	
	//Assign bool type to expression
	Token_Expression::SetBoolType(sym.first->GetTokenDerived<Token_Type>());
	
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

	sym = CreateProcFuncSymbol("write", false, false);
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
	BlockStack.pop_back();		//NOTE: calls destructor...
}

//Create block
std::shared_ptr<AsmBlock> AsmFile::CreateBlock(AsmBlock::Type_T type, std::shared_ptr<Token> tok){
	std::vector<std::shared_ptr<AsmBlock> >::iterator it;
	
	std::shared_ptr<AsmBlock> ptr(new AsmBlock(type,tok));
	BlockList.push_back(ptr);
	
	if (type != AsmBlock::Global)
		GetCurrentBlock()->AddChildBlock(ptr);
	
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
	//TODO pedantic occlusion detection
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
	current -> SetSymbol(ptr);
	
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
	
	std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Typename, id, std::shared_ptr<Token>(ptr));
	return sym;
}

std::pair<std::shared_ptr<Symbol>, AsmCode> AsmFile::GetTypeSymbol(std::string id) throw(AsmCode){
	id = StringToLower(id);	//Case insensitive
	std::pair<std::shared_ptr<Symbol>, AsmCode> result = GetSymbol(id);
	
	if (result.first -> GetType() != Symbol::Typename)
		throw SymbolIsNotAType;
	
	return result;
}

//Create symbol from list - outputs pedantic errors
std::vector<std::shared_ptr<Token_Var> > AsmFile::CreateVarSymbolsFromList(std::shared_ptr<Token_IDList> IDList, std::shared_ptr<Token_Type> type, std::string AsmInitialValue){
	std::map <std::string, std::shared_ptr<Token> > list = IDList->GetList();
	std::map <std::string, std::shared_ptr<Token> >::iterator it;
	
	std::vector<std::shared_ptr<Token_Var> > result;
	
	for (it=list.begin() ; it != list.end(); it++ ){
		std::pair<std::string, std::shared_ptr<Token> > value;
		value = *it;
		std::shared_ptr<Token_Var> ptr(new Token_Var(value.first, type));	
		result.push_back(ptr);
		try{
			std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Variable, value.first, std::static_pointer_cast<Token>(ptr));	
			ptr -> SetSymbol(sym.first);		
			
			if (Flags.Pedantic && sym.second == SymbolExistsInOuterBlock){
				std::stringstream msg;
				msg << "Variable '" << value.first << "' might occlude another symbol defined in an outer scope.";
				HandleError(msg.str().c_str(), E_GENERIC, E_WARNING, value.second -> GetLine(), value.second->GetColumn());
			}
			
			//Create a label
			//Label is a concatenation of the current block name and it's ID
			std::string LabelID = GetCurrentBlock() -> GetID() + "_" + StringToLower(value.first);
			std::shared_ptr<AsmLabel> label = CreateLabel(LabelID, sym.first);		
			sym.first->SetLabel(label);
			
			//Create the data lines
			std::string val;
			if (AsmInitialValue.empty())
				val = type -> AsmDefaultValue();
			else
				val = AsmInitialValue;
			
			std::shared_ptr<AsmLine> line = CreateDataLine(label, val);
			label -> SetLine(line);
			
			//Add Line to data list
			DataLines.push_back(line);
		}
		catch (AsmCode e){
			if (e == SymbolReserved){
				std::stringstream msg;
				msg << "'" << value.first << "' is a reserved keyword.";
				HandleError(msg.str().c_str(), E_PARSE, E_ERROR, value.second -> GetLine(), value.second->GetColumn());
			}
			else if (e == SymbolExistsInCurrentBlock){
				std::stringstream msg;
				msg << "'" << value.first << "' has already been declared in this block.";
				HandleError(msg.str().c_str(), E_PARSE, E_ERROR, value.second -> GetLine(), value.second->GetColumn());
			}
			else if (e == LabelExists){
				std::stringstream msg;
				//This shouldn't happen
				msg << "A duplicate Assembly label has been created. This is a compiler bug.";
				HandleError(msg.str().c_str(), E_PARSE, E_FATAL, value.second -> GetLine(), value.second->GetColumn());
			}
		}
	}
	return result;
		
}

//Create Procedure Symbol
std::pair<std::shared_ptr<Symbol>, AsmCode> AsmFile::CreateProcFuncSymbol(std::string ID, bool function, bool push) throw(AsmCode){	//CreateSymbol throws are not caught
	//Create token value for procedure. Create a symbol. Create a block And link accordingly
	std::pair<std::shared_ptr<Symbol>, AsmCode> result;
	std::shared_ptr<Token_Func> tok;
	std::shared_ptr<AsmBlock> block;
	if (function){
		tok.reset(new Token_Func(ID, Token_Func::Function));
		block = CreateBlock(AsmBlock::Function);
		result = CreateSymbol(Symbol::Function, ID, tok);
	}
	else{
		tok.reset(new Token_Func(ID, Token_Func::Procedure));
		block = CreateBlock(AsmBlock::Procedure);
		result = CreateSymbol(Symbol::Procedure, ID, tok);
	}
	tok -> SetBlock(block);
	block -> SetBlockSymbol(result.first);
	tok -> SetSymbol(result.first);
	
	//Create label
	std::shared_ptr<AsmLabel> label(new AsmLabel("func_" + ID, result.first));
	result.first -> SetLabel(label);
	result.first -> SetBlock(block);
	
	if (push)
		PushBlock(block);
	
	return result;
}

//Create temp variable
std::shared_ptr<Symbol> AsmFile::CreateTempVar(std::shared_ptr<Token_Type> type){
	static unsigned counter = 0;
	
	std::string ID = ToString<unsigned>(counter) + "_temp";
	counter++;
	
	std::shared_ptr<Token_Var> var(new Token_Var(ID, type, false, true));
	
	std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Variable, ID, var);	
	sym.first -> SetTemporary();
	
	return sym.first;
}

//Generate Code
std::string AsmFile::GenerateCode(){
	std::stringstream output;
	/** Header File **/
	try{
		output << ReadFile(Flags.AsmHeaderPath.c_str());
	}
	catch(...){
		HandleError("Header file for assembly does not exist.", E_GENERIC, E_FATAL);
	}
	/** Data Lines **/
	
	std::vector<std::shared_ptr<AsmLine> > ::iterator it;
	for (it = DataLines.begin(); it < DataLines.end(); it++){
		std::shared_ptr<AsmLine> line = *it;
		
		if (line -> GetLabel() != nullptr){
			output << line -> GetLabel()->GetID();
		}
		output << "\t";
		
		output << line -> GetOpCodeStr();
		output << " " << line -> GetRd() -> GetImmediate() << "\n";
	}	

	/** Stack File **/
	try{
		output << ReadFile(Flags.AsmStackPath.c_str());
	}
	catch(...){
		HandleError("Stack file for assembly does not exist.", E_GENERIC, E_FATAL);
	}

	/** Code Proper **/
	bool LoopDelta = false;
	std::list<std::shared_ptr<AsmLine> >::iterator itList;
	//NOTE Initial Code generated assumes ALL the variables are in registers. It is the code generator that has to take care of the stack and what not
	for (itList = CodeLines.begin(); itList != CodeLines.end(); itList++){
		std::shared_ptr<AsmLine> line = *itList;
		if (line -> GetCC() == AsmLine::NV || line -> GetOpCode() == AsmLine::NOP)
			continue;
		std::shared_ptr<AsmOp> Rd, Rm, Rn, Ro;
		std::pair<std::string, std::string> RdOutput, RmOutput, RnOutput, RoOutput;
		AsmLine::OpCode_T OpCode;
		
		OpCode = line -> GetOpCode();
		
		Rd = line -> GetRd();
		Rm = line -> GetRm();
		Rn = line -> GetRn();
		Ro = line -> GetRo();
		
		std::shared_ptr<AsmRegister> Reg = GetCurrentBlock()->GetRegister();
		unsigned OpCount = 0;
		
		//Label
		if (line -> GetLabel() != nullptr){
			output << line -> GetLabel()->GetID();
		}
		
		if (line -> IsInLoop() != LoopDelta){
			LoopDelta = line -> IsInLoop();
			if (LoopDelta)
				Reg->SetInLoop();
			else
				Reg->SetInLoop(false);
		}
		
		//Internal opcode handling
		if (OpCode == AsmLine::SAVE){
			if (Rd -> GetType() == AsmOp::Register){
				output << Reg->SaveRegister(Rd -> GetSymbol());
			}
			continue;
		}
		else if (OpCode == AsmLine::WRITE_INT || OpCode == AsmLine::WRITE_C){
			AsmOp::Type_T RdType = Rd -> GetType();

			if (RdType == AsmOp::Register){
				output << Reg->ForceVar(Rd -> GetSymbol(),0, true, false);
			}
			else if (RdType == AsmOp::Immediate){
				output << Reg->SaveRegister(0);
				Reg->EvictRegister(0);
				Reg->IncrementCounter();
				output << "\tMOV R0, ";
				output << Rd -> GetImmediate() << "\n";
			}
			if (OpCode == AsmLine::WRITE_INT)
				output << "\tBL PRINTR0_ ;Print integer\n";
			else
				output << "\tSWI SWI_WriteC\n";
			continue;
		}
		else if (OpCode == AsmLine::FUNCALL){			
			//Force save the variable in R4
			//output << Reg -> SaveRegister(4);
			
			//Force the params into their respective positions
			if (Rm != nullptr)
				output << Reg->ForceVar(Rm -> GetSymbol(),0, true, false);
			if (Rn != nullptr)
				output << Reg->ForceVar(Rn -> GetSymbol(),1, true, false);
			if (Ro != nullptr)
				output << Reg->ForceVar(Ro -> GetSymbol(),2, true, false);
			
			//Force Rd to be R4
			output << Reg->ForceVar(Rd -> GetSymbol(),4, false, true, true, false);
			
			continue;
		}
		
		/** Rm **/ //TODO
		if (Rm != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RmType = Rm -> GetType();
			if (RmType == AsmOp::Register){
				if (Rm -> IsWrite())
					RmOutput = Reg -> GetVarWrite( Rm -> GetSymbol() );
				else
					RmOutput = Reg -> GetVarRead( Rm -> GetSymbol() );
			}
			else if (RmType == AsmOp::Immediate){
				RmOutput.first = Rm -> GetImmediate();
			}
			output << RmOutput.second;
		}
		
		/** Rn **/ //TODO
		if (Rn != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RnType = Rn -> GetType();
			if (RnType == AsmOp::Register){
				if (Rn -> IsWrite())
					RnOutput = Reg -> GetVarWrite( Rn -> GetSymbol() );
				else
					RnOutput = Reg -> GetVarRead( Rn -> GetSymbol() );
			}
			else if (RnType == AsmOp::Immediate){
				RnOutput.first = Rn -> GetImmediate();
			}
			output << RnOutput.second;
		}
		
		/** Ro **/
		if (Ro != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RoType = Ro -> GetType();
			if (RoType == AsmOp::Register){
				if (Ro -> IsWrite())
					RoOutput = Reg -> GetVarWrite( Ro -> GetSymbol() );
				else
					RoOutput = Reg -> GetVarRead( Ro -> GetSymbol() );
			}
			else if (RoType == AsmOp::Immediate){
				RoOutput.first = Ro -> GetImmediate();
			}
			output << RoOutput.second;
		}
		
		/** Rd **/ //TODO More than just destination registers?
		if (Rd != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RdType = Rd -> GetType();
			if (RdType == AsmOp::Register){
				if (Rd -> IsWrite())
					RdOutput = Reg -> GetVarWrite( Rd -> GetSymbol() );
				else
					RdOutput = Reg -> GetVarRead( Rd -> GetSymbol() );
			}
			else if (RdType == AsmOp::Immediate){
				RdOutput.first = Rd -> GetImmediate();
			}
			else if (RdType == AsmOp::CodeLabel){
				RdOutput.first = Rd->GetLabel()-> GetID();
			}
			output << RdOutput.second;
		}
		
		output << "\t";
		//Opcode
		output << line -> GetOpCodeStr() << " ";
		
		if (!RdOutput.first.empty()){
			//Rd
			output << RdOutput.first;
			OpCount++;
		}
		if (!RmOutput.first.empty()){
			//Rm
			if (OpCount != 0 ) output << ", ";
			output << RmOutput.first;
			OpCount++;
		}
		if (!RnOutput.first.empty()){
			//Rn
			if (OpCount != 0 ) output << ", ";
			output << RnOutput.first;
			OpCount++;
		}
		if (!RoOutput.first.empty()){
			//Ro
			if (OpCount != 0 ) output << ", ";
			output << RoOutput.first;
			OpCount++;
		}
		//Comments
		std::string comment = line -> GetComment();
		if (!comment.empty())
			output << " ;" << comment;
		
		//EOL
		output << "\n";
	}	
	if (GetCurrentBlock()->NextLabel != nullptr)
		output << GetCurrentBlock()->NextLabel -> GetID();
	
	if (Flags.SaveRegisters)
		output << GetCurrentBlock()->GetRegister()->SaveAllRegisters();
	
	output << "\tSWI SWI_EXIT";
	
	/** StdLib **/
	try{
		output << ReadFile(Flags.AsmStdLibPath.c_str());
	}
	catch(...){
		HandleError("StdLib file for assembly does not exist.", E_GENERIC, E_FATAL);
	}
	
	/** User functions/procedures **/
	std::vector<std::string> LineStacks;
	std::stringstream CurrentOutput;
	LoopDelta = false;
	//NOTE Initial Code generated assumes ALL the variables are in registers. It is the code generator that has to take care of the stack and what not
	for (itList = FunctionLines.begin(); itList != FunctionLines.end(); itList++){
		std::shared_ptr<AsmLine> line = *itList;
		if (line -> GetCC() == AsmLine::NV || line -> GetOpCode() == AsmLine::NOP)
			continue;
		std::shared_ptr<AsmOp> Rd, Rm, Rn, Ro;
		std::pair<std::string, std::string> RdOutput, RmOutput, RnOutput, RoOutput;
		AsmLine::OpCode_T OpCode;
		
		OpCode = line -> GetOpCode();
		
		Rd = line -> GetRd();
		Rm = line -> GetRm();
		Rn = line -> GetRn();
		Ro = line -> GetRo();
		
		std::shared_ptr<AsmRegister> Reg = GetCurrentBlock()->GetRegister();
		unsigned OpCount = 0;
		
		
		
		//Label
		if (line -> GetLabel() != nullptr){
			CurrentOutput << line -> GetLabel()->GetID();
		}
		
		if (line -> IsInLoop() != LoopDelta){
			LoopDelta = line -> IsInLoop();
			if (LoopDelta)
				Reg->SetInLoop();
			else
				Reg->SetInLoop(false);
		}
		
		//Internal opcode handling
		if (OpCode == AsmLine::BLOCKPUSH){		
			if (!CurrentOutput.str().empty()){
				LineStacks.push_back(CurrentOutput.str());
				CurrentOutput.clear();
				CurrentOutput.str("");
			}
			std::shared_ptr<Symbol> sym = Rd -> GetSymbol();
			std::shared_ptr<Token_Func> func = sym->GetTokenDerived<Token_Func>();
			std::vector<Token_FormalParam::Param_T> params = func -> GetParams()->GetParams();
			//Push in the block
			PushBlock(sym -> GetBlock());
			
			//Okay, time to handle the memory
			Reg = GetCurrentBlock()->GetRegister();
			Reg -> SetFunctionRegisters(params.size());
			
			std::vector<Token_FormalParam::Param_T>::iterator paramIt;
			unsigned i = 0;
			
			for (paramIt = params.begin(); paramIt < params.end(); paramIt++, i++){
				Token_FormalParam::Param_T param = *paramIt;
				Reg -> SetSymbol(i, param.Variable->GetSymbol());
			}
			
			if (sym -> GetType() == Symbol::Function){
				//Return variable - in the last used for now
				Reg -> SetSymbol(i, sym);
				Reg -> SetPermanent(i);
				//Reg -> SetBelongToScope(i);
				Reg -> SetInitialUse(++i);
			}
			//Label for next line
			//CurrentOutput << Rd -> GetSymbol() -> GetLabel() -> GetID();
			continue;
		}
		else if (OpCode == AsmLine::BLOCKPOP){
			std::shared_ptr<Symbol> sym = Rd -> GetSymbol();
			std::shared_ptr<Token_Func> func = sym->GetTokenDerived<Token_Func>();
			std::vector<Token_FormalParam::Param_T> params = func -> GetParams()->GetParams();
			
			output << "; ------------------------------------------------------\n";
			output << "; " << sym -> GetLabel() -> GetID() << "\n";
			unsigned i = 0;
			for (std::vector<Token_FormalParam::Param_T>::iterator paramIt = params.begin(); paramIt < params.end(); i++, paramIt++){
				output << "; R" << i << " - ";
				output << (*paramIt).Variable -> GetID() << ": " << (*paramIt).Variable -> GetVarType() -> TypeToString();
				output << "\n";
			}
			
			output << "; ------------------------------------------------------\n";
			//STMED
			output << sym -> GetLabel() -> GetID();
			output << "\tSTMED SP!, {";
			
			std::vector<unsigned> list = Reg->GetListOfNotBelong();
			for (std::vector<unsigned>::iterator itReg = list.begin(); itReg != list.end(); itReg++){
				if (sym -> GetType() == Symbol::Function && *itReg == 4)
					continue;		//We won't save R4 since we are going to be using it as return
				output << "R" << *itReg << ",";
			}
			output << "R14}; save to stack\n";
			
			output << CurrentOutput.str();
			
			if (GetCurrentBlock()->NextLabel != nullptr)
				output << GetCurrentBlock()->NextLabel -> GetID();	
			
			if (sym -> GetType() == Symbol::Function){
				//Move return value to R4
				std::pair<std::string, std::string> ReturnString = Reg -> GetVarRead( sym );
				if (ReturnString.first != "R4"){
					output << ReturnString.second;
					output << "\tMOV R4, " << ReturnString.first << "; return value\n";
				}
			}
			
			output << "\tLDMED SP!, {";
			
			for (std::vector<unsigned>::iterator itReg = list.begin(); itReg != list.end(); itReg++){
				if (sym -> GetType() == Symbol::Function && *itReg == 4)
					continue; //We won't save R0 since we are going to be using it as return
				output << "R" << *itReg << ",";
			}
			output << "R15}; return\n";
			
			if (!LineStacks.empty()){
				CurrentOutput.clear();
				CurrentOutput.str(*LineStacks.rbegin());
				LineStacks.pop_back();
			}
			else{
				CurrentOutput.clear();
				CurrentOutput.str("");				
			}
			continue;
		}		
		else if (OpCode == AsmLine::SAVE){
			if (Rd -> GetType() == AsmOp::Register){
				CurrentOutput << Reg->SaveRegister(Rd -> GetSymbol());
			}
			continue;
		}
		else if (OpCode == AsmLine::WRITE_INT || OpCode == AsmLine::WRITE_C){
			AsmOp::Type_T RdType = Rd -> GetType();
			bool save = true;
			
			//if (GetCurrentBlock()->GetBlockSymbol()->GetTokenDerived<Token_Func>()->GetParams()->GetSize() > 0)			//WOW WOW WOW this is getting out of hand
			//	save = false;
			
			//if (!save)
				//No where to save -- so we use the stack
			//	CurrentOutput << "\tSTMED SP!, {R0}; save R0\n";
			if (RdType == AsmOp::Register){
				CurrentOutput << Reg->ForceVar(Rd -> GetSymbol(),0, true, false, save);
			}
			else if (RdType == AsmOp::Immediate){
				if (save) 
					CurrentOutput << Reg->SaveRegister(0);
				Reg->EvictRegister(0);
				Reg->IncrementCounter();
				CurrentOutput << "\tMOV R0, ";
				CurrentOutput << Rd -> GetImmediate() << "\n";
			}
			if (OpCode == AsmLine::WRITE_INT)
				CurrentOutput << "\tBL PRINTR0_ ;Print integer\n";
			else
				CurrentOutput << "\tSWI SWI_WriteC\n";
			
			//if (!save)
			//	CurrentOutput << "\tLDMED SP!, {R0}; retrieve R0\n";
			continue;
		}
		
		
		/** Rm **/ //TODO
		if (Rm != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RmType = Rm -> GetType();
			if (RmType == AsmOp::Register){
				if (Rm -> IsWrite())
					RmOutput = Reg -> GetVarWrite( Rm -> GetSymbol() );
				else
					RmOutput = Reg -> GetVarRead( Rm -> GetSymbol() );
			}
			else if (RmType == AsmOp::Immediate){
				RmOutput.first = Rm -> GetImmediate();
			}
			CurrentOutput << RmOutput.second;
		}
		
		/** Rn **/ //TODO
		if (Rn != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RnType = Rn -> GetType();
			if (RnType == AsmOp::Register){
				if (Rn -> IsWrite())
					RnOutput = Reg -> GetVarWrite( Rn -> GetSymbol() );
				else
					RnOutput = Reg -> GetVarRead( Rn -> GetSymbol() );
			}
			else if (RnType == AsmOp::Immediate){
				RnOutput.first = Rn -> GetImmediate();
			}
			CurrentOutput << RnOutput.second;
		}
		
		/** Ro **/
		if (Ro != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RoType = Ro -> GetType();
			if (RoType == AsmOp::Register){
				if (Ro -> IsWrite())
					RoOutput = Reg -> GetVarWrite( Ro -> GetSymbol() );
				else
					RoOutput = Reg -> GetVarRead( Ro -> GetSymbol() );
			}
			else if (RoType == AsmOp::Immediate){
				RoOutput.first = Ro -> GetImmediate();
			}
			CurrentOutput << RoOutput.second;
		}
		
		/** Rd **/ //TODO More than just destination registers?
		if (Rd != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RdType = Rd -> GetType();
			if (RdType == AsmOp::Register){
				if (Rd -> IsWrite())
					RdOutput = Reg -> GetVarWrite( Rd -> GetSymbol() );
				else
					RdOutput = Reg -> GetVarRead( Rd -> GetSymbol() );
			}
			else if (RdType == AsmOp::Immediate){
				RdOutput.first = Rd -> GetImmediate();
			}
			else if (RdType == AsmOp::CodeLabel){
				RdOutput.first = Rd->GetLabel()-> GetID();
			}
			CurrentOutput << RdOutput.second;
		}
		
		CurrentOutput << "\t";
		//Opcode
		CurrentOutput << line -> GetOpCodeStr() << " ";
		
		if (!RdOutput.first.empty()){
			//Rd
			CurrentOutput << RdOutput.first;
			OpCount++;
		}
		if (!RmOutput.first.empty()){
			//Rm
			if (OpCount != 0 ) CurrentOutput << ", ";
			CurrentOutput << RmOutput.first;
			OpCount++;
		}
		if (!RnOutput.first.empty()){
			//Rn
			if (OpCount != 0 ) CurrentOutput << ", ";
			CurrentOutput << RnOutput.first;
			OpCount++;
		}
		if (!RoOutput.first.empty()){
			//Ro
			if (OpCount != 0 ) CurrentOutput << ", ";
			CurrentOutput << RoOutput.first;
			OpCount++;
		}
		//Comments
		std::string comment = line -> GetComment();
		if (!comment.empty())
			CurrentOutput << " ;" << comment;
		
		//EOL
		CurrentOutput << "\n";
	}
	output << CurrentOutput.str();	//For global scoped functions
	
	output << "\n\tEND";
	return output.str();
}

/** Line Related Methods **/
std::shared_ptr<AsmLine> AsmFile::CreateDataLine(std::shared_ptr<AsmLabel> Label, std::string value)
{
	//TODO Comments
	std::shared_ptr<AsmLine> line(new AsmLine(AsmLine::Directive, AsmLine::DCD));
	line -> SetLabel(Label);
	
	std::shared_ptr<AsmOp> op(new AsmOp(AsmOp::Immediate, AsmOp::Rd));
	op->SetImmediate(value);
	
	line -> SetRd(op);
	
	return line;
}

std::shared_ptr<AsmLine> AsmFile::CreateCodeLine(AsmLine::OpType_T OpType, AsmLine::OpCode_T OpCode){
	std::shared_ptr<AsmLine> line(new AsmLine(OpType, OpCode));
	
	if (GetCurrentBlock()->NextLabel != nullptr){
		line -> SetLabel(GetCurrentBlock()->NextLabel);
		GetCurrentBlock()->NextLabel.reset();
	}
	if (GetCurrentBlock()->IsInLoop())
		line -> SetInLoop();
	
	if (GetCurrentBlock()-> IsGlobal())
		CodeLines.push_back(line);
	else
		FunctionLines.push_back(line);
	return line;
}

std::pair<std::shared_ptr<AsmLine>, std::list<std::shared_ptr<AsmLine> >::iterator> AsmFile::CreateCodeLineIt(AsmLine::OpType_T OpType, AsmLine::OpCode_T OpCode){
	std::pair<std::shared_ptr<AsmLine>, std::list<std::shared_ptr<AsmLine> >::iterator> result;
	result.first = CreateCodeLine(OpType, OpCode);
	result.second = CodeLines.end()--;
	return result;
}

std::shared_ptr<AsmLine> AsmFile::CreateAssignmentLine(std::shared_ptr<Symbol> sym, std::shared_ptr<Token_Expression> expr){
	std::shared_ptr<AsmLine> line;
	
	std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
	Rd -> SetSymbol(sym);
	
	line = FlattenExpression(expr, Rd);
	
	if (sym -> GetType() == Symbol::Function){
		//Then Rd will be set to temp to prevent saving
		sym -> SetTemporary();
	}
	Rd -> SetWrite();
	
	return line;
}

/** Label Related Methods **/
std::shared_ptr<AsmLabel> AsmFile::CreateLabel(std::string ID, std::shared_ptr<Symbol> sym, std::shared_ptr<AsmLine> Line) throw(AsmCode){
	std::shared_ptr<AsmLabel> ptr(new AsmLabel(ID, sym, Line));
	std::pair<std::map<std::string, std::shared_ptr<AsmLabel> >::iterator, bool> result;
	result = LabelList.insert(
			std::pair<std::string, std::shared_ptr<AsmLabel> >(ID, ptr)		
	);
	if (!result.second)
		throw LabelExists;
	return ptr;
}

/** Statement Methods **/
AsmCode AsmFile::TypeCompatibilityCheck(std::shared_ptr<Token_Type> LHS, std::shared_ptr<Token_Type> RHS){
	return (*LHS) == (*RHS) ? TypeCompatible : TypeIncompatible;	//TODO more checks
}

std::shared_ptr<AsmLine> AsmFile::FlattenExpression(std::shared_ptr<Token_Expression> expr, std::shared_ptr<AsmOp> Rd, bool cmp){
	std::shared_ptr<AsmLine> result;
	
//	if (Rd == nullptr){
		//Let's create an Rd
//		Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
//	}
	
	//Check for strict simplicity
	std::shared_ptr<Token_Factor>simple = expr -> GetSimple();
	
	//This is a strictly simple expression - CMP doesn't make sense here
	if (simple != nullptr){		
		//Handle based on form
		Token_Factor::Form_T Form = simple -> GetForm();
		
		//Variable reference
		if (Form == Token_Factor::VarRef){
			if (simple -> IsNegate()){
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
				if (Rd == nullptr){
					//Let's create an Rd
					Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
					std::shared_ptr<Symbol> temp = CreateTempVar(expr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
					Rd -> SetSymbol(temp);
				}
				//Create AsmOp for variable
				std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Register, AsmOp::Rm ) );

				Rm -> SetSymbol( std::static_pointer_cast<Token_Var>(simple -> GetValueToken()) -> GetSymbol() );
				Rd -> SetWrite();
				result -> SetRm(Rm);
				result -> SetRd(Rd);	
			}
			else{
				if (Rd == nullptr){		//i.e. it just wants the value of expession
					//Fake line
					result.reset(new AsmLine(AsmLine::Directive, AsmLine::NOP));
					//Create AsmOp for variable
					std::shared_ptr<AsmOp> Rd(new AsmOp( AsmOp::Register, AsmOp::Rd ) );

					Rd -> SetSymbol( std::static_pointer_cast<Token_Var>(simple -> GetValueToken()) -> GetSymbol() );
					result -> SetRd(Rd);
				}
				else{
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					//Create AsmOp for variable
					std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Register, AsmOp::Rm ) );

					Rm -> SetSymbol( std::static_pointer_cast<Token_Var>(simple -> GetValueToken()) -> GetSymbol() );
					//Rd -> SetWrite();
					result -> SetRm(Rm);
					result -> SetRd(Rd);
				}
							
			}

		}
		//Constant
		else if (Form == Token_Factor::Constant){
			if (simple -> IsNegate()){
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
			
				std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Immediate, AsmOp::Rm ) );
				Rm -> SetImmediate(simple -> GetValueToken() -> AsmValue());
				
				result -> SetRm(Rm);
				if (Rd == nullptr){
					//Let's create an Rd
					Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
					std::shared_ptr<Symbol> temp = CreateTempVar(expr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
					Rd -> SetSymbol(temp);
				}
				result -> SetRd(Rd);
				Rd -> SetWrite();
			}
			else{
				if (Rd == nullptr){		//i.e. it just wants the value of expession
					//Fake line
					result.reset(new AsmLine(AsmLine::Directive, AsmLine::NOP));
					//Create AsmOp for variable
					std::shared_ptr<AsmOp> Rd(new AsmOp( AsmOp::Immediate, AsmOp::Rd ) );

					Rd -> SetImmediate(simple -> GetValueToken() -> AsmValue());
					result -> SetRd(Rd);
				}
				else{
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					//Create AsmOp for variable
					std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Immediate, AsmOp::Rm ) );
					Rm -> SetImmediate(simple -> GetValueToken() -> AsmValue());
					result -> SetRm(Rm);
					result -> SetRd(Rd);
					Rd -> SetWrite();
				}
			}
		}
		//Function
		else if (Form == Token_Factor::FuncCall){
			//TODO - More than three args
			if (Rd == nullptr){
				//Let's create an Rd
				Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
				std::shared_ptr<Symbol> temp = CreateTempVar(expr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
				Rd -> SetSymbol(temp);
			}
			std::shared_ptr<Token_Func> func = simple -> GetFuncToken();
			std::vector<Token_FormalParam::Param_T> FormalParams = func -> GetParams() -> GetParams();
			std::vector<std::shared_ptr<Token_Expression> >  exprs = simple->GetTokenDerived<Token_ExprList>()->GetList();
			std::shared_ptr<AsmLine> line = CreateCodeLine(AsmLine::Directive, AsmLine::FUNCALL);
			line -> SetRd(Rd);
			
			std::vector<std::shared_ptr<Token_Expression> >::iterator it;
			unsigned i = 0;
			for (it = exprs.begin(); it < exprs.end() && i < 3; it++, i++){
				std::shared_ptr<Token_Expression> expr = *it;
				
				//OK create a new operator
				std::shared_ptr<AsmOp> ExprRd(new AsmOp(AsmOp::Register, AsmOp::Rm));
				ExprRd -> SetSymbol(FormalParams[i].Variable->GetSymbol());
				std::shared_ptr<AsmLine> line2 = FlattenExpression(expr);
				
				std::shared_ptr<AsmOp> Ri( new AsmOp(*(line2->GetRd())) );
				
				switch (i){
					case 0:
						line -> SetRm(Ri); break;
					case 1: 
						line -> SetRn(Ri); break;
					case 2:
						line -> SetRo(Ri); break;
					
				}
			}
			//Create branch line
			std::shared_ptr<AsmLine> branch = CreateCodeLine(AsmLine::BranchLink, AsmLine::BL);
			std::shared_ptr<AsmOp> BranchOp(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
			BranchOp -> SetLabel(func -> GetSymbol()->GetLabel());
			branch -> SetRd(BranchOp);
			
			if (simple -> IsNegate()){
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
				result -> SetRm(Rd);
				result -> SetRd(Rd);
			}
			else{
				//Fake line
				result.reset(new AsmLine(AsmLine::Directive, AsmLine::NOP));
				//Create AsmOp for variable
				result -> SetRd(Rd);
			}
						
		}
		
	}
	else{
		//No? More hard work :(
		//Check if Expression is simple
		if (expr -> IsSimple()){			//CMP does not make sense here
			//Then there is nothing to generate for expression. Move down to flattening the SimExpression
			Rd = FlattenSimExpression(expr -> GetSimExpression(), Rd);		
			//Generate a fake line with Rd set
			result.reset(new AsmLine(AsmLine::Directive, AsmLine::NOP));
			result -> SetRd(Rd);
			//Rd -> SetWrite();
		}
		else{
			if (!cmp && Rd == nullptr){
				//Let's create an Rd
				Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
				std::shared_ptr<Symbol> temp = CreateTempVar(expr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
				Rd -> SetSymbol(temp);				
			}
			//In this case we definitely have to generate temporary variables already. We will use the existing Rd for LHS
			std::shared_ptr<AsmLine> LHSLine;
			std::shared_ptr<AsmOp> LHS, RHS(new AsmOp(AsmOp::Register, AsmOp::Rd));
			//Flatten Expression on LHS
			if (cmp)
				LHSLine = FlattenExpression(expr -> GetExpression(), nullptr );
			else
				LHSLine = FlattenExpression(expr -> GetExpression(), std::shared_ptr<AsmOp>( new AsmOp(*Rd)));
			std::shared_ptr<Symbol> RHSTemp = CreateTempVar(expr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
			expr -> SetTempVar(RHSTemp);
			RHS -> SetSymbol(RHSTemp);
			//and generate a temporary variable for RHS
			//Flatten expression on RHS
			RHS = std::shared_ptr<AsmOp>(new AsmOp(FlattenSimExpression(expr -> GetSimExpression(), RHS)));	//Clone
			LHS = LHSLine -> GetRd();	
			LHS  = std::shared_ptr<AsmOp>(new AsmOp(LHS));//Clone
			
			RHS -> SetWrite(false);
			LHS -> SetWrite(false);
			
			Op_T Op = expr -> GetOp(); //TODO
			
			if (cmp){
				result = CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
				result -> SetRm(LHS);
				result -> SetRn(RHS);
			}
			else{
				//We have to do a comparison first
				result = CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
				result -> SetRm(LHS);
				result -> SetRn(RHS);
				result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
				
				std::shared_ptr<AsmOp> One(new AsmOp(AsmOp::Immediate, AsmOp::Rm));
				One -> SetImmediate("1");
				std::shared_ptr<AsmOp> Zero(new AsmOp(AsmOp::Immediate, AsmOp::Rm));
				Zero -> SetImmediate("0");
				
				if (Op == LT){
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::LT);
					result -> SetRd(Rd);
					result -> SetRm(One);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::GE);
					result -> SetRd(Rd);
					result -> SetRm(Zero);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					
				}
				else if (Op == LTE){
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::LE);
					result -> SetRd(Rd);
					result -> SetRm(One);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::GT);
					result -> SetRd(Rd);
					result -> SetRm(Zero);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
				}
				else if (Op == GT){
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::GT);
					result -> SetRd(Rd);
					result -> SetRm(One);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::LE);
					result -> SetRd(Rd);
					result -> SetRm(Zero);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
				}
				else if (Op == GTE){
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::GE);
					result -> SetRd(Rd);
					result -> SetRm(One);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::LT);
					result -> SetRd(Rd);
					result -> SetRm(Zero);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
				}
				else if (Op == Equal){
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::EQ);
					result -> SetRd(Rd);
					result -> SetRm(One);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::NE);
					result -> SetRd(Rd);
					result -> SetRm(Zero);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
				}
				else if (Op == NotEqual){
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::NE);
					result -> SetRd(Rd);
					result -> SetRm(One);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
					result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					result -> SetCC(AsmLine::EQ);
					result -> SetRd(Rd);
					result -> SetRm(Zero);
					result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
				}
				else if (Op == In){
					
				}
				Rd -> SetWrite();
			}
		}
	}
	result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
	return result;
}

//Terms plus minus
std::shared_ptr<AsmOp> AsmFile::FlattenSimExpression(std::shared_ptr<Token_SimExpression> simexpr, std::shared_ptr<AsmOp> Rd){
	std::shared_ptr<AsmOp> result;
	std::shared_ptr<AsmLine> line;
	
	if (simexpr -> IsSimple()){
		result = FlattenTerm(simexpr -> GetTerm(), Rd);
	}
	else{
		if (Rd == nullptr){
			//Let's create an Rd
			Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
			std::shared_ptr<Symbol> temp = CreateTempVar(simexpr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
			Rd -> SetSymbol(temp);
		}
		result = Rd;

		//It's a plus minus or xor
		std::shared_ptr<AsmOp> LHS, RHS(new AsmOp(AsmOp::Register, AsmOp::Rd));
		LHS = FlattenSimExpression(simexpr -> GetSimExpression(), std::shared_ptr<AsmOp>( new AsmOp(*Rd)));		//Clone.
		
		std::shared_ptr<Symbol> RHSTemp = CreateTempVar(simexpr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
		RHS -> SetSymbol(RHSTemp);
		//and generate a temporary variable for RHS		
		
		RHS = FlattenTerm(simexpr -> GetTerm(), RHS);
		
		//Clone
		LHS = std::shared_ptr<AsmOp>(new AsmOp(LHS));
		RHS = std::shared_ptr<AsmOp>(new AsmOp(RHS));
		LHS -> SetWrite(false);
		RHS -> SetWrite(false);
		
		Op_T Op = simexpr -> GetOp(); 
		if (LHS -> GetType() == AsmOp::Immediate && RHS -> GetType() == AsmOp::Immediate){
			//Can be simplified into an immediate
			std::shared_ptr<Token> LHSToken(LHS -> GetToken()), RHSToken(RHS -> GetToken()), ReturnToken;
			//NOTE Are we going to do type checks here?
			if (LHSToken -> GetTokenType() == V_Int){
				int LHSValue = GetValue<int>(LHSToken), RHSValue = GetValue<int>(RHSToken), ReturnValue;
				switch (Op){
					case Add:
						ReturnValue = LHSValue + RHSValue;
						break;
					case Subtract:
						ReturnValue = LHSValue - RHSValue;
						break;
					case Or:
						ReturnValue = LHSValue | RHSValue;
						break;
					case Xor:
						ReturnValue = LHSValue ^ RHSValue;
					default:
						ReturnValue = LHSValue;
				}
				ReturnToken.reset(new Token_Int(ReturnValue, V_Int));
				
				result.reset(new AsmOp(AsmOp::Immediate, AsmOp::Rd));
				result -> SetImmediate(ReturnToken -> AsmValue());
				result -> SetToken(ReturnToken);
			}
			else if (LHSToken -> GetTokenType() == V_Character){
				char LHSValue = GetValue<char>(LHSToken), RHSValue = GetValue<char>(RHSToken), ReturnValue;
				switch (Op){
					case Add:
						ReturnValue = LHSValue + RHSValue;
						break;
					case Subtract:
						ReturnValue = LHSValue - RHSValue;
						break;
					case Or:
						ReturnValue = LHSValue | RHSValue;
						break;
					case Xor:
						ReturnValue = LHSValue ^ RHSValue;
					default:
						ReturnValue = LHSValue;
				}
				ReturnToken.reset(new Token_Char(ReturnValue, true));
				
				result.reset(new AsmOp(AsmOp::Immediate, AsmOp::Rd));
				result -> SetImmediate(ReturnToken -> AsmValue());
				result -> SetToken(ReturnToken);
			}
			line = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
			line -> SetRd(Rd);
			line -> SetRm(result);
			line -> SetComment("Line " + ToString<int>(simexpr -> GetLine()));
			Rd -> SetWrite();
		}
		else{
			if (Op == Add){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
			}
			else if (Op == Subtract){
				if (RHS -> GetType() == AsmOp::Register && LHS -> GetType() == AsmOp::Immediate){
					line = CreateCodeLine(AsmLine::Processing, AsmLine::RSB);
				}
				else
					line = CreateCodeLine(AsmLine::Processing, AsmLine::SUB);
			}
			else if (Op == Or){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ORR);
			}
			else if (Op == Xor){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::EOR);
			}
			if (RHS -> GetType() == AsmOp::Register && LHS -> GetType() == AsmOp::Immediate){
				line -> SetRm(RHS);
				line -> SetRn(LHS);
			}
			else{
				line -> SetRm(LHS);
				line -> SetRn(RHS);
			}
			line -> SetRd(Rd);
			line -> SetComment("Line " + ToString<int>(simexpr -> GetLine()));
			Rd -> SetType(AsmOp::Register);			//We've done calculation
			Rd -> SetWrite();
		}
	}
	
	return result;
}

std::shared_ptr<AsmOp> AsmFile::FlattenTerm(std::shared_ptr<Token_Term> term, std::shared_ptr<AsmOp> Rd){
	std::shared_ptr<AsmOp> result;
	std::shared_ptr<AsmLine> line;
	
	if (term -> IsSimple()){		//TODO Strict simplicity optimisation
		Rd = FlattenFactor(term -> GetFactor(), Rd);
		result = Rd;
	}
	else{
		if (Rd == nullptr){
			//Let's create an Rd
			Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
			std::shared_ptr<Symbol> temp = CreateTempVar(term -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
			Rd -> SetSymbol(temp);
			
		}
		result = Rd;
		//It's a * / div mod and
		std::shared_ptr<AsmOp> LHS, RHS(new AsmOp(AsmOp::Register, AsmOp::Rd));		//RHS has to use a temp variable
		LHS = FlattenTerm(term -> GetTerm(), std::shared_ptr<AsmOp>( new AsmOp(*Rd))); //Clone
		
		std::shared_ptr<Symbol> RHSTemp = CreateTempVar(term -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
		term -> SetTempVar(RHSTemp);
		RHS -> SetSymbol(RHSTemp);			

		RHS = FlattenFactor(term -> GetFactor(), RHS);
		
		//Clone
		LHS = std::shared_ptr<AsmOp>(new AsmOp(LHS));
		RHS = std::shared_ptr<AsmOp>(new AsmOp(RHS));
		LHS -> SetWrite(false);
		RHS -> SetWrite(false);
		
		Op_T Op = term -> GetOp(); 
		if (LHS -> GetType() == AsmOp::Immediate && RHS -> GetType() == AsmOp::Immediate){
			//Can be simplified into an immediate
			std::shared_ptr<Token> LHSToken(LHS -> GetToken()), RHSToken(RHS -> GetToken()), ReturnToken;
			//NOTE Are we going to do type checks here?
			if (LHSToken -> GetTokenType() == V_Int){
				int LHSValue = GetValue<int>(LHSToken), RHSValue = GetValue<int>(RHSToken), ReturnValue;
				switch (Op){
					case Multiply:
						ReturnValue = LHSValue * RHSValue;
						break;
					case Divide:
					case Div:
						ReturnValue = LHSValue / RHSValue;
						break;
					case Mod:
						ReturnValue = LHSValue % RHSValue;
						break;
					case And:
						ReturnValue = LHSValue & RHSValue;
					default:
						ReturnValue = LHSValue;
				}
				ReturnToken.reset(new Token_Int(ReturnValue, V_Int));
				
				result.reset(new AsmOp(AsmOp::Immediate, AsmOp::Rd));
				result -> SetImmediate(ReturnToken -> AsmValue());
				result -> SetToken(ReturnToken);
			}
			else if (LHSToken -> GetTokenType() == V_Character){
				char LHSValue = GetValue<char>(LHSToken), RHSValue = GetValue<char>(RHSToken), ReturnValue;
				switch (Op){
					case Multiply:
						ReturnValue = LHSValue * RHSValue;
						break;
					case Divide:
					case Div:
						ReturnValue = LHSValue / RHSValue;
						break;
					case Mod:
						ReturnValue = LHSValue % RHSValue;
						break;
					case And:
						ReturnValue = LHSValue & RHSValue;
					default:
						ReturnValue = LHSValue;
				}
				ReturnToken.reset(new Token_Char(ReturnValue, true));
				
				result.reset(new AsmOp(AsmOp::Immediate, AsmOp::Rd));
				result -> SetImmediate(ReturnToken -> AsmValue());
				result -> SetToken(ReturnToken);
			}
			line = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
			line -> SetRd(Rd);
			line -> SetRm(result);
			line -> SetComment("Line " + ToString<int>(term -> GetLine()));
			Rd -> SetWrite();
		}
		else{
			if (Op == Multiply){		
				//Okay if LHS or RHS are immediates, we have to move them into registries
				if (LHS -> GetType() == AsmOp::Immediate){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> LHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(LHS);
					RdTemp -> SetSymbol(LHSTemp);
					LHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				
				if (RHS -> GetType() == AsmOp::Immediate){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					RHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(RHS);
					RdTemp -> SetSymbol(RHSTemp);
					RHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				//Check to see if Rd == Rm
				if (Rd -> GetType() == AsmOp::Register && LHS -> GetType() == AsmOp::Register && Rd -> GetSymbol() == LHS -> GetSymbol()){
					//Rd cannot be the same as Rm or the compiler will complain of undefined behaviour.
					//Check if Rm also = Rn
					if (LHS -> GetSymbol() == RHS -> GetSymbol()){
						//Let's create a temp variable for LHS
						std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
						std::shared_ptr<Symbol> LHSTemp = CreateTempVar(term -> GetType());	
						std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
						line2 -> SetRd(RdTemp);
						line2 -> SetRm(LHS);
						RdTemp -> SetSymbol(LHSTemp);
						LHS = RdTemp;
						line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
					}
					else{
						//Swap them
						std::shared_ptr<AsmOp> temp = LHS;
						LHS = RHS;
						RHS = temp;
					}
				}
				line = CreateCodeLine(AsmLine::Processing, AsmLine::MUL);
				line -> SetRm(LHS);
				line -> SetRn(RHS);
				line -> SetRd(Rd);
				Rd -> SetWrite();
				
			}
			else if (Op == Divide || Op == Div){
				//If LHS is a register, we have to check it's saved
				if (LHS -> GetType() == AsmOp::Register){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::SAVE);
					line2 -> SetRd(LHS);		
				}
				
				//Okay if LHS or RHS are immediates, we have to move them into registries
				if (LHS -> GetType() == AsmOp::Immediate){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> LHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(LHS);
					RdTemp -> SetSymbol(LHSTemp);
					LHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				
				if (RHS -> GetType() == AsmOp::Immediate){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					RHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(RHS);
					RdTemp -> SetSymbol(RHSTemp);
					RHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				//Check if any of the registers are the same
				//Check to see if Rd == Rm
				if (Rd -> GetType() == AsmOp::Register && LHS -> GetType() == AsmOp::Register && Rd -> GetSymbol() == LHS -> GetSymbol()){
					//Let's create a temp variable for LHS
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> LHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(LHS);
					RdTemp -> SetSymbol(LHSTemp);
					LHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				
				//Check if Rd == RHS
				if (Rd -> GetType() == AsmOp::Register && RHS -> GetType() == AsmOp::Register && Rd -> GetSymbol() == RHS -> GetSymbol()){
					//Let's create a temp variable for RHS
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> RHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(RHS);
					RdTemp -> SetSymbol(RHSTemp);
					RHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				
				//Check if LHS = RHS
				if (LHS -> GetType() == AsmOp::Register && RHS -> GetType() == AsmOp::Register && LHS -> GetSymbol() == RHS -> GetSymbol()){
					//Let's create a temp variable for RHS
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> RHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(RHS);
					RdTemp -> SetSymbol(RHSTemp);
					RHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}				
				
				//Since it's LHS/RHS, 	
				line = CreateCodeLine(AsmLine::Processing, AsmLine::DivMod);
				
				line -> SetRd(Rd);
				line -> SetRm(LHS);
				line -> SetRn(RHS);
				
				//Create temporary variable
				//std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
				std::shared_ptr<Symbol> DivTemp = CreateTempVar(term -> GetType());	
				std::shared_ptr<AsmOp> Ro(new AsmOp(AsmOp::Register, AsmOp::Ro));
				Ro -> SetSymbol(DivTemp);
				line -> SetRo(Ro);
				
				Ro -> SetWrite();
				LHS -> SetWrite();
				Rd -> SetWrite();
			}
			//else if (Op == Div){
			//	line = CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
			//	line -> SetComment("Unsupported, for now");
			//}
			else if (Op == Mod){
				//If LHS is a register, we have to check it's saved
				if (LHS -> GetType() == AsmOp::Register){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::SAVE);
					line2 -> SetRd(LHS);		
				}
				//Okay if LHS or RHS are immediates, we have to move them into registries
				if (LHS -> GetType() == AsmOp::Immediate){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> LHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(LHS);
					RdTemp -> SetSymbol(LHSTemp);
					LHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				
				if (RHS -> GetType() == AsmOp::Immediate){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					RHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(RHS);
					RdTemp -> SetSymbol(RHSTemp);
					RHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
	
				//Check if LHS = RHS
				if (LHS -> GetType() == AsmOp::Register && RHS -> GetType() == AsmOp::Register && LHS -> GetSymbol() == RHS -> GetSymbol()){
					//Let's create a temp variable for RHS
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					std::shared_ptr<Symbol> RHSTemp = CreateTempVar(term -> GetType());	
					std::shared_ptr<AsmOp> RdTemp(new AsmOp(AsmOp::Register, AsmOp::Rd));
					line2 -> SetRd(RdTemp);
					line2 -> SetRm(RHS);
					RdTemp -> SetSymbol(RHSTemp);
					RHS = RdTemp;
					line2 -> SetComment("Line " + ToString<int>(term -> GetLine()));
				}
				
				//Since it's LHS/RHS, the macro wants Rd  = RHS, Rm = LHS, Rn = nothing, Ro = some temp variable				
				line = CreateCodeLine(AsmLine::Processing, AsmLine::DivMod);
				
				//line -> SetRd(Rd);
				line -> SetRm(LHS);
				line -> SetRn(RHS);
				
				//Create a nothing Op
				std::shared_ptr<AsmOp> nothing(new AsmOp(AsmOp::Immediate, AsmOp::Rd));
				nothing -> SetImmediate("\"\"");
				line -> SetRd(nothing);
				
				//Create temporary variable
				//std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
				std::shared_ptr<Symbol> DivTemp = CreateTempVar(term -> GetType());	
				std::shared_ptr<AsmOp> Ro(new AsmOp(AsmOp::Register, AsmOp::Ro));
				Ro -> SetSymbol(DivTemp);
				line -> SetRo(Ro);
				Ro -> SetWrite();
				LHS -> SetWrite();
				//Then let's MOV LHS to Rd
				if (Rd -> GetSymbol() != LHS -> GetSymbol()){
					std::shared_ptr<AsmLine> line2 = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
					line2 -> SetRd(Rd);
					line2 -> SetRm(LHS);
				}
				Rd -> SetWrite();
			}
			else if (Op == And){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::AND);
				if (RHS -> GetType() == AsmOp::Register && LHS -> GetType() == AsmOp::Immediate){		//Swap
					line -> SetRm(RHS);
					line -> SetRn(LHS);
				}
				else{
					line -> SetRm(LHS);
					line -> SetRn(RHS);
				}
				line -> SetRd(Rd);
				Rd -> SetWrite();
			}
			
			line -> SetComment("Line " + ToString<int>(term -> GetLine()));
			Rd -> SetType(AsmOp::Register);			//We've done calculation
		}
	}
	
	return result;
}

std::shared_ptr<AsmOp> AsmFile::FlattenFactor(std::shared_ptr<Token_Factor> factor, std::shared_ptr<AsmOp> Rd){
	if (Rd == nullptr){
		//Let's create an Rd
		Rd.reset(new AsmOp(AsmOp::Register, AsmOp::Rd));
		std::shared_ptr<Symbol> temp = CreateTempVar(factor -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
		Rd -> SetSymbol(temp);
	}
	std::shared_ptr<AsmOp> result = Rd;		//Most likely the case
	//Check form of factor
	Token_Factor::Form_T form = factor->GetForm();
	if (form == Token_Factor::Constant){
		result -> SetType(AsmOp::Immediate);
		result -> SetImmediate(factor -> GetValueToken() -> AsmValue());
		result -> SetToken( factor -> GetValueToken());
	}
	else if (form == Token_Factor::Expression){
		//Flatten an expression
		std::shared_ptr<AsmLine> line = FlattenExpression(factor -> GetTokenDerived<Token_Expression>(), Rd);
		result = line -> GetRd();
	}
	else if (form == Token_Factor::VarRef){
		result -> SetType(AsmOp::Register);
		result -> SetSymbol(factor -> GetTokenDerived<Token_Var>() -> GetSymbol());
		result -> SetToken( factor -> GetValueToken());
	}
	else if (form == Token_Factor::FuncCall){
		//TODO - More than three args
		std::shared_ptr<Token_Func> func = factor -> GetFuncToken();
		std::vector<Token_FormalParam::Param_T> FormalParams = func -> GetParams() -> GetParams();
		std::vector<std::shared_ptr<Token_Expression> >  exprs = factor->GetTokenDerived<Token_ExprList>()->GetList();
		std::shared_ptr<AsmLine> line = CreateCodeLine(AsmLine::Directive, AsmLine::FUNCALL);
		line -> SetRd(Rd);
		
		std::vector<std::shared_ptr<Token_Expression> >::iterator it;
		unsigned i = 0;
		for (it = exprs.begin(); it < exprs.end() && i < 3; it++, i++){
			std::shared_ptr<Token_Expression> expr = *it;
			
			//OK create a new operator
			std::shared_ptr<AsmOp> ExprRd(new AsmOp(AsmOp::Register, AsmOp::Rm));
			ExprRd -> SetSymbol(FormalParams[i].Variable->GetSymbol());
			std::shared_ptr<AsmLine> line2 = FlattenExpression(expr);
			
			std::shared_ptr<AsmOp> Ri( new AsmOp(*(line2->GetRd())) );
			
			switch (i){
				case 0:
					line -> SetRm(Ri); break;
				case 1: 
					line -> SetRn(Ri); break;
				case 2:
					line -> SetRo(Ri); break;
			}
		}
		
		//Create branch line
		std::shared_ptr<AsmLine> branch = CreateCodeLine(AsmLine::BranchLink, AsmLine::BL);
		std::shared_ptr<AsmOp> BranchOp(new AsmOp(AsmOp::CodeLabel, AsmOp::Rd));
		BranchOp -> SetLabel(func -> GetSymbol()->GetLabel());
		branch -> SetRd(BranchOp);
	}
	
	//Negate value
	if (factor -> IsNegate()){
		std::shared_ptr<AsmLine> line = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
		line -> SetRd(result);
		line -> SetRm(result);
		line -> SetComment("Line " + ToString<int>(factor -> GetLine()));
	}
	
	return result;
}	

void AsmFile::CreateWriteLine(std::shared_ptr<Token_ExprList> list){
	//We will only look at the first expression...
	std::shared_ptr<Token_Expression> expr = list->GetList()[0];
	std::shared_ptr<Token_Type> type = expr->GetType();
	std::shared_ptr<AsmLine> line;
	if (type->GetPrimary() == Token_Type::Integer){		
		std::shared_ptr<AsmLine> exprLine = FlattenExpression(expr);
		line = CreateCodeLine(AsmLine::Processing, AsmLine::WRITE_INT);	
		line -> SetRd(exprLine -> GetRd());
	}
	else if (type->GetPrimary() == Token_Type::Char){
std::shared_ptr<AsmLine> exprLine = FlattenExpression(expr);
		line = CreateCodeLine(AsmLine::Processing, AsmLine::WRITE_C);	
		line -> SetRd(exprLine -> GetRd());
	}
	
}

/** Compiler Debugging Methods **/
void AsmFile::PrintSymbols(){  //TODO Print type of symbol and type of variable/function etc.
	std::cout << "Printing Symbols (" << LexerLineCount << ")\n";
	
	std::pair<std::shared_ptr<Symbol>, AsmCode> result;
	std::shared_ptr<AsmBlock> working;
	std::vector<std::shared_ptr<AsmBlock> >::reverse_iterator it;
	
	int depth = 0;
	
	for (it = BlockStack.rbegin(); it < BlockStack.rend(); it++, depth--){
		std::cout << "Block Depth " << depth << ":\n";
		working = *it;
		std::map<std::string, std::shared_ptr<Symbol> > list = (*it) -> GetList();
		std::map<std::string, std::shared_ptr<Symbol> >::iterator iterator;
		for (iterator = list.begin(); iterator != list.end(); iterator++){
			std::shared_ptr<Symbol> value = (*iterator).second;
			std::cout << "\t" << value->GetID() << " - " << int(value->GetType()) << "\n";
		}
	}
	std::cout << std::endl;
}

void AsmFile::PrintBlocks(){
	//Print current block child blocks
	std::shared_ptr<AsmBlock> current = GetCurrentBlock();
	std::cout << "Current Block '" << current->GetID() << "' Type " << (int) current->GetType() << " (" << LexerLineCount << ")\n";
	
	std::vector<std::shared_ptr<AsmBlock> > child = current->GetChildBlocks();
	std::vector<std::shared_ptr<AsmBlock> >::iterator it;
	for (it = child.begin(); it < child.end(); it++){
		std::cout << "\t'" << (*it)->GetID() << "' Type " << (int) (*it)->GetType() << "\n";
	}
	std::cout << std::endl;
}

/**
 * 	AsmLine
 * */
AsmLine::AsmLine(OpType_T Type, OpCode_T OpCode):
	Type(Type), OpCode(OpCode), Condition(AL), InLoop(false)
{
	InitialiseStaticMaps();
}

AsmLine::AsmLine(const AsmLine &obj):
	OpCode(obj.OpCode),
	Condition(obj.Condition),
	Qualifier(obj.Qualifier),
	Type(obj.Type),
	Label(obj.Label),
	Rd(obj.Rd), Rm(obj.Rm), Rn(obj.Rn), Ro(obj.Ro),
	Comment(obj.Comment),
	InLoop(obj.InLoop)
{
	InitialiseStaticMaps();
}

AsmLine AsmLine::operator=(const AsmLine &obj){
	if (&obj != this){
		OpCode = obj.OpCode;
		Condition = obj.Condition;
		Qualifier = obj.Qualifier;
		Type = obj.Type;
		Label = obj.Label;
		Rd = obj.Rd;
		Rm = obj.Rm;
		Rn = obj.Rn;
		Ro = obj.Ro;
		Comment = obj.Comment;
		InLoop = obj.InLoop;
	}
	return *this;
}

std::string AsmLine::GetOpCodeStr() const{
	if (Condition == AL)
		return OpCodeStr[OpCode];
	else
		return OpCodeStr[OpCode] + CCStr[Condition];
}

//Initialise 
void AsmLine::InitialiseStaticMaps(){
	static bool init = false;
	if (!init){
		init = true;
		
		OpCodeStr[AND] = "AND";
		OpCodeStr[EOR] = "EOR";
		OpCodeStr[SUB] = "SUB";
		OpCodeStr[RSB] = "RSB";
		OpCodeStr[ADD] = "ADD";
		OpCodeStr[ADC] = "ADC";
		OpCodeStr[SBC] = "SBC";
		OpCodeStr[RSC] = "RSC";
		OpCodeStr[TST] = "TST";
		OpCodeStr[TEQ] = "TEQ";
		OpCodeStr[CMP] = "CMP";
		OpCodeStr[CMN] = "CMN";
		OpCodeStr[ORR] = "ORR";
		OpCodeStr[MOV] = "MOV";
		OpCodeStr[BIC] = "BIC";
		OpCodeStr[MVN] = "MVN";
		OpCodeStr[SWP] = "SWP";
		
		OpCodeStr[MUL] = "MUL";
		OpCodeStr[MLA] = "MLA";
		OpCodeStr[UMULL] = "UMULL";
		OpCodeStr[UMLAL] = "UMLAL";
		OpCodeStr[SMULL] = "SMULL";
		OpCodeStr[SMLAL] = "SMLAL";
		
		OpCodeStr[LDR] = "LDR";
		OpCodeStr[STR] = "STR";
		OpCodeStr[STM] = "STM";
		OpCodeStr[LDM] = "LDM";
		
		OpCodeStr[B] = "B";
		OpCodeStr[BL] = "BL";
		
		OpCodeStr[SWI] = "SWI";
		
		OpCodeStr[ADR] = "ADR";
		OpCodeStr[AREA] = "AREA";
		OpCodeStr[END] = "END";
		OpCodeStr[ENTRY] = "ENTRY";
		OpCodeStr[DATA] = "DATA";
		OpCodeStr[DCD] = "DCD";
		OpCodeStr[DCB] = "DCB";
		//OpCodeStr[DCFD] = "DCFD";
		//OpCodeStr[DCFS] = "DCFS";
		OpCodeStr[EQU] = "EQU";
		
		OpCodeStr[DivMod] = "DivMod";
		
		CCStr[CS] = "CS";
		CCStr[SQ] = "SQ";
		CCStr[VS] = "VS";
		CCStr[GT] = "GT";
		CCStr[GE] = "GE";
		CCStr[PL] = "PL";
		CCStr[HI] = "HI";
		CCStr[HS] = "HS";
		CCStr[CC] = "CC";
		CCStr[NE] = "NE";
		CCStr[VC] = "VC";
		CCStr[LT] = "LT";
		CCStr[LE] = "LE";
		CCStr[MI] = "MI";
		CCStr[LO] = "LO";
		CCStr[LS] = "LS";
		CCStr[AL] = "AL";
		CCStr[NV] = "NV";
	}
}

/**
 * 	AsmOp
 * */

AsmOp::AsmOp(Type_T Type, Position_T Position):
	Type(Type), Position(Position), Scale(AsmOp::NoScale), sym(nullptr), OffsetAddressOp(nullptr), ScaleOp(nullptr), Write(false), Label(nullptr)
{
	
}

AsmOp::AsmOp(const AsmOp &obj):
	Type(obj.Type), Position(obj.Position), Scale(obj.Scale), sym(obj.sym), 
	OffsetAddressOp(obj.OffsetAddressOp), ScaleOp(obj.ScaleOp),
	ImmediateValue(obj.ImmediateValue), tok(obj.tok), Write(obj.Write),
	Label(obj.Label)
{
	
}

AsmOp::AsmOp(std::shared_ptr<AsmOp> ptr):
	Type(ptr->Type), Position(ptr->Position), Scale(ptr->Scale), sym(ptr->sym), 
	OffsetAddressOp(ptr->OffsetAddressOp), ScaleOp(ptr->ScaleOp),
	ImmediateValue(ptr->ImmediateValue), tok(ptr->tok), Write(ptr->Write),
	Label(ptr->Label)
{
	
}

AsmOp AsmOp::operator=(const AsmOp& obj)
{
	if (this != &obj){
		Type = obj.Type;
		Position = obj.Position;
		Scale = obj.Scale;
		sym = obj.sym;
		OffsetAddressOp = obj.OffsetAddressOp;
		ScaleOp = obj.ScaleOp;
		ImmediateValue = obj.ImmediateValue;
		tok = obj.tok;
		Write = obj.Write;
		Label = obj.Label;
	}
	
	return *this;
}

/**
 * 	AsmLabels
 * */

AsmLabel::AsmLabel(std::string ID, std::shared_ptr<Symbol> sym, std::shared_ptr<AsmLine> Line):
	sym(sym), Line(Line), ID(ID)
	{}

AsmLabel::AsmLabel(const AsmLabel& obj):
	sym(obj.sym), Line(obj.Line), ID(obj.ID)
{

}

AsmLabel AsmLabel::operator=(const AsmLabel& obj)
{
	if (this != &obj){
		sym = obj.sym;
		Line = obj.Line;
		ID = obj.ID;
	}
	
	return *this;
}



/**
 * 	AsmBlock
 * 
 * */

int AsmBlock::count = 0;

AsmBlock::AsmBlock(AsmBlock::Type_T type, std::shared_ptr<Token> tok): Type(type), TokenAssoc(tok), InLoopCount(0)
{
	if (type == Global){
		ID = "GLOBAL";
		Register.reset(new AsmRegister(true));
	}
	else{
		Register.reset(new AsmRegister);
		if (TokenAssoc != nullptr)
			ID = TokenAssoc -> GetStrValue();
		else{
			std::stringstream temp;
			temp << "block_" << count;
			ID = temp.str();
		}
	}
	count++;
}

AsmBlock::AsmBlock(const AsmBlock& obj): 
	SymbolList(obj.SymbolList), Type(obj.Type), ChildBlocks(obj.ChildBlocks), TokenAssoc(obj.TokenAssoc), ID(obj.ID), Register(obj.Register), BlockSymbol(obj.BlockSymbol)
{
}

AsmBlock AsmBlock::operator=(const AsmBlock &obj){
	if (&obj != this){
		Type = obj.Type;
		SymbolList = obj.SymbolList;
		ChildBlocks = obj.ChildBlocks;
		TokenAssoc = obj.TokenAssoc;
		ID = obj.ID;
		Register = obj.Register;
		BlockSymbol = obj.BlockSymbol;
	}
	return *this;
}

void AsmBlock::SetSymbol(std::shared_ptr<Symbol> sym) throw(AsmCode){
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

std::shared_ptr<AsmLabel> AsmBlock::CreateIfElseLabel(){
	static unsigned counter = 0;
	std::shared_ptr<AsmLabel> result = Program.CreateLabel("IFELSE_" + ToString<unsigned>(counter));		//TODO Clean up
	
	counter++;
	return result;
}

std::shared_ptr<AsmLabel> AsmBlock::CreateForLabel(){
	static unsigned counter = 0;
	std::shared_ptr<AsmLabel> result = Program.CreateLabel("FOR_" + ToString<unsigned>(counter));//TODO Clean up
	
	counter++;
	return result;
}

std::shared_ptr<AsmLabel> AsmBlock::CreateWhileLabel(){
	static unsigned counter = 0;
	std::shared_ptr<AsmLabel> result = Program.CreateLabel("WHILE_" + ToString<unsigned>(counter));		//TODO clean up
	
	counter++;
	return result;
}


/** AsmRegisters **/
AsmRegister::AsmRegister(bool IsGlobal) : 
	Registers(AsmUsableReg+1, AsmRegister::State_T(IsGlobal)), counter(0), InitialUse(0),  InLoop(false)
{

}

AsmRegister::AsmRegister(unsigned FuncRegister):
	Registers(AsmUsableReg, AsmRegister::State_T(false)), counter(0), InitialUse(FuncRegister-1), InLoop(false)
{
	for (unsigned i = 0; i < FuncRegister; i++){
		GetRegister(i).Permanent = true;
		GetRegister(i).BelongToScope = true;
	}
	InitialUse = FuncRegister;
}

AsmRegister::AsmRegister(const AsmRegister& obj):
	Registers(obj.Registers), counter(obj.counter), InitialUse(obj.InitialUse), InLoop(obj.InLoop)
{

}

AsmRegister AsmRegister::operator=(const AsmRegister& obj)
{
	if (this != &obj){
		Registers = obj.Registers;
		counter = obj.counter;
		InitialUse = obj.InitialUse;
		InLoop = obj.InLoop;
	}
	
	return *this;
}

/**Simple Getters and Setters **/

AsmRegister::State_T &AsmRegister::GetRegister(unsigned ID){
	if (ID > AsmUsableReg)
		throw;	//Illegal access
	return Registers.at(ID);
	
}

const AsmRegister::State_T &AsmRegister::GetRegister(unsigned ID) const{
	if (ID > AsmUsableReg)
		throw;	//Illegal access
	return Registers.at(ID);
	
}

void AsmRegister::SetBelongToScope(unsigned ID, bool val){
	GetRegister(ID).BelongToScope = val;
}

void AsmRegister::SetSymbol(unsigned ID, std::shared_ptr<Symbol> sym){
	GetRegister(ID).sym = sym;
}

void AsmRegister::SetWrittenTo(unsigned ID, bool val){
	GetRegister(ID).WrittenTo = val;
}

void AsmRegister::SetPermanent(unsigned ID, bool val){
	GetRegister(ID).Permanent = val;
}

std::shared_ptr<Symbol> AsmRegister::GetSymbol(unsigned ID){
	return GetRegister(ID).sym;
}
bool AsmRegister::GetBelongToScope(unsigned ID) const{
	return GetRegister(ID).BelongToScope;
}
bool AsmRegister::GetWrittenTo(unsigned ID) const{
	return GetRegister(ID).WrittenTo;
}
bool AsmRegister::GetPermanent(unsigned ID) const{
	return GetRegister(ID).Permanent;
}

/** Getters and Setters - Aggregate **/
std::pair<unsigned, std::string> AsmRegister::GetAvailableRegister(std::shared_ptr<Symbol> sym, bool load){
	std::pair<unsigned, std::string> ToReturn;
	std::stringstream output;
	if (InitialUse <= AsmUsableReg){
		//Okay easy
		ToReturn.first =  InitialUse;
		InitialUse++;
	}
	else{
		//Find LRU
		unsigned result, counter=UINT_MAX;
		bool candidate = false;
		for (unsigned i = 0; i < AsmUsableReg; i++){
			State_T &Register = GetRegister(i);
			if (!Register.Permanent && Register.LastUsed < counter){
				counter = Register.LastUsed;
				result = i;
				candidate = true;
			}
		}
		
		if (!candidate)		//Unlikely. In the event someone actually sets all AsmUsableReg registers as permanent....
			throw;
		
		ToReturn.first = result;
		//State_T &Register = GetRegister(result);
		
		SaveRegister(result);
		EvictRegister(result);
	}
	if (sym != nullptr && load && !sym -> IsTemporary()){
		//Load data into register
		output << "\tLDR R" << AsmScratch << ", =" << sym -> GetLabel() -> GetID() << " ;Loading variable\n";
		output << "\tLDR R" << ToReturn.first << ", [R" << AsmScratch << "]\n";
	}
	GetRegister(ToReturn.first).sym = sym;
	ToReturn.second = output.str();
	return ToReturn;
}

std::pair<std::string, std::string> AsmRegister::GetVarRead(std::shared_ptr<Symbol> var){
	std::pair<std::string, std::string> result;
	std::map<std::shared_ptr<Symbol>, unsigned >::iterator it;
	std::pair<std::shared_ptr<Symbol>, unsigned> sym = FindSymbol(var);
	
	if (sym.first == nullptr){
		//Not found.
		std::pair<unsigned, std::string> ID = GetAvailableRegister(var, true);
		result.second = ID.second;  //Any neccessary instructions
		result.first = "R" + ToString<unsigned>(ID.first);
		GetRegister(ID.first).LastUsed = counter;
	}
	else{
		//Found
		result.first = "R" + ToString<unsigned>(sym.second);
		GetRegister(sym.second).LastUsed = counter;
	}
	counter++;
	return result;
}

std::pair<std::string, std::string> AsmRegister::GetVarWrite(std::shared_ptr<Symbol> var){
	std::pair<std::string, std::string> result;
	std::pair<std::shared_ptr<Symbol>, unsigned> sym = FindSymbol(var);
	
	if (sym.first == nullptr){
		//Not found.
		std::pair<unsigned, std::string> ID = GetAvailableRegister(var);
		result.second = ID.second;  //Any neccessary instructions
		result.first = "R" + ToString<unsigned>(ID.first);
		GetRegister(ID.first).LastUsed = counter;
		GetRegister(ID.first).WrittenTo = true;
		GetRegister(ID.first).WrittenBefore = true;
	}
	else{
		//Found
		result.first = "R" + ToString<unsigned>(sym.second);
		GetRegister(sym.second).LastUsed = counter;
		GetRegister(sym.second).WrittenTo = true;
		GetRegister(sym.second).WrittenBefore = true;
	}
	counter++;
	return result;
}

std::pair<std::shared_ptr<Symbol>, unsigned> AsmRegister::FindSymbol(std::shared_ptr<Symbol> sym){
	std::pair<std::shared_ptr<Symbol>, unsigned> result;
	unsigned end = InitialUse;
	
	if (end > AsmUsableReg)
		end = AsmUsableReg;
	
	for (unsigned i = 0; i <= end; i++){
		if(Registers[i].sym.get() == sym.get()){
			result.first = Registers[i].sym;
			result.second = i;
			break;
		}
	}
	
	return result;
}

std::string AsmRegister::SaveRegister(std::shared_ptr<Symbol> var){
	std::stringstream output;
	std::pair<std::shared_ptr<Symbol>, unsigned> result = FindSymbol(var);
	if (result.first != nullptr){
		//Might need to saved
		State_T &Register = GetRegister(result.second);
		//Check if register has been written to and if the variable is a temporary and if it's permanent
		if ( (Register.WrittenTo || InLoop )  && Register.sym != nullptr && !Register.sym->GetTokenDerived<Token_Var>() -> IsTemp() && Register.sym -> GetLabel() != nullptr){
			output << "\tLDR R" << AsmScratch << ", =" << Register.sym -> GetLabel() -> GetID() << "; Force storage of variable\n";
			output << "\tSTR R" << result.second << ", [R" << AsmScratch << "]\n";
		}
	}
	return output.str();
}

std::string AsmRegister::SaveRegister(unsigned no){
	State_T &Register = GetRegister(no);
	std::stringstream output;
	
	if ( (Register.WrittenTo || InLoop ) && Register.sym != nullptr && !Register.sym->GetTokenDerived<Token_Var>() -> IsTemp() && Register.sym -> GetLabel() != nullptr){
		output << "\tLDR R" << AsmScratch << ", =" << Register.sym -> GetLabel() -> GetID() << "; Force storage of variable\n";
		output << "\tSTR R" << no << ", [R" << AsmScratch << "]\n";
	}
	return output.str();
}
//Once evicted, the class will no longer have any idea about the register use unless this was called internally
//Then this register will be thrown out if there is a need to clear any register accordingly. So make sure to update the LastUsed accordingly.
//Probably best to only use it immediately.
void AsmRegister::EvictRegister(unsigned no){		
	State_T &Register = GetRegister(no);
	
	Register.sym = nullptr;
	Register.WrittenTo = false;
	Register.LastUsed = counter;
}

std::string AsmRegister::ForceVar(std::shared_ptr<Symbol> var, unsigned no, bool load, bool write, bool save, bool move){
	//Check if var already exists
	std::stringstream result;
	std::pair<std::shared_ptr<Symbol>, unsigned> sym = FindSymbol(var);		//Find if var already exists elsewhere
	State_T &Register = GetRegister(no);
	
	if (sym.first != nullptr && sym.second == no)
		//Trivial
		return "";
	
	if (save)
		//Force save no
		result << SaveRegister(no);	
	
	if (sym.first != nullptr){
		//Found
		EvictRegister(no);
		State_T &Previous = GetRegister(sym.second);
		//Assign
		Register.sym = var;
		Register.WrittenTo = Previous.WrittenTo;
		if (move)
			result << "\tMOV R" << no << ", R" << sym.second << " ; force moving\n"; 
		EvictRegister(sym.second);
	}
	else{
		//Not found
		//Evict register
		EvictRegister(no);
		//Assign
		Register.sym = var;
		//Load?
		if (Register.sym != nullptr && load && !Register.sym -> IsTemporary()){
			//Load data into register
			result << "\tLDR R" << AsmScratch << ", =" << Register.sym -> GetLabel() -> GetID() << " ;Loading variable\n";
			result << "\tLDR R" << no << ", [R" << AsmScratch << "]\n";
		};
	}

	if (write)
		Register.WrittenTo = true;
	
	counter++;
	if (InitialUse <= no)
		InitialUse = ++no;
	
	return result.str();
}

std::string AsmRegister::SaveAllRegisters(){
	std::stringstream result;
	unsigned end = InitialUse;
	
	if (end > AsmUsableReg)
		end = AsmUsableReg;
	for (unsigned i = 0; i <= end; i++)
		result << SaveRegister(i);
	
	return result.str();
}

void AsmRegister::SetFunctionRegisters(unsigned count){
	for (unsigned i = 0; i < count; i++){
		GetRegister(i).Permanent = true;
		GetRegister(i).BelongToScope = true;
	}	
	InitialUse = count;
}
std::vector<unsigned> AsmRegister::GetListOfNotBelong(){
	std::vector<unsigned> result;
	unsigned end = InitialUse;
	
	if (end > AsmUsableReg)
		end = AsmUsableReg;
	
	for (unsigned i = 0; i <= end; i++){
		State_T &Register = GetRegister(i);
		if (Register.WrittenBefore && !Register.BelongToScope)
			result.push_back(i);
	}
	return result;
}