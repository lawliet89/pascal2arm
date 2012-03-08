#include "asm.h"
#include "utility.h"
#include "op.h"
#include <iostream>
#include <set>

extern Flags_T Flags;
extern std::stringstream OutputString;		//op.cpp
extern unsigned LexerCharCount, LexerLineCount;		//In lexer.l	- DEBUG purposes


/** Data Structures **/

std::map<AsmLine::OpCode_T, std::string> AsmLine::OpCodeStr;

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
void AsmFile::CreateVarSymbolsFromList(std::shared_ptr<Token_IDList> IDList, std::shared_ptr<Token_Type> type, std::shared_ptr<Token> InitialValue){
	std::map <std::string, std::shared_ptr<Token> > list = IDList->GetList();
	std::map <std::string, std::shared_ptr<Token> >::iterator it;
	
	for (it=list.begin() ; it != list.end(); it++ ){
		std::pair<std::string, std::shared_ptr<Token> > value;
		value = *it;
		Token_Var * ptr = new Token_Var(value.first, type);		//TODO use shared pointer
		try{
			std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Variable, value.first, std::shared_ptr<Token>(dynamic_cast<Token *>(ptr)));	
			ptr -> SetSymbol(sym.first);		
			
			if (Flags.Pedantic && sym.second == SymbolExistsInOuterBlock){
				std::stringstream msg;
				msg << "Variable '" << value.first << "' might occlude another symbol defined in an outer scope.";
				HandleError(msg.str().c_str(), E_GENERIC, E_WARNING, value.second -> GetLine(), value.second->GetColumn());
			}
			
			//Create a label
			//Label is a concatenation of the current block name and it's ID
			std::string LabelID = GetCurrentBlock() -> GetID() + "_" + value.first;
			std::shared_ptr<AsmLabel> label = CreateLabel(LabelID, sym.first);		
			sym.first->SetLabel(label);
			
			//Create the data lines
			std::string val;
			if (InitialValue == nullptr)
				val = type -> AsmDefaultValue();
			else
				val = InitialValue -> AsmValue();
			
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
		
}

//Create Procedure Symbol
std::pair<std::shared_ptr<Symbol>, AsmCode> AsmFile::CreateProcSymbol(std::string ID){
	//Create token value for procedure. Create a symbol. Create a block And link accordingly
	std::pair<std::shared_ptr<Symbol>, AsmCode> result;
	std::shared_ptr<Token_Func> tok(new Token_Func(ID, Token_Func::Procedure));
	
	//Create block
	std::shared_ptr<AsmBlock> block = CreateBlock(AsmBlock::Procedure);
	//Push block
	PushBlock(block);
	
	return result;
}

//Create temp variable
std::shared_ptr<Symbol> AsmFile::CreateTempVar(std::shared_ptr<Token_Expression> expr){
	std::shared_ptr<Token_Var> var(new Token_Var(expr -> GetIDStr(), expr->GetType(), false, true));
	
	std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Variable, expr -> GetIDStr(), var);	
	sym.first -> SetTemporary();
	expr -> SetTempVar(sym.first);
	
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
	
	output << ";--------------------------------------------------------------------------------" 
		<< "\n;Program Code\n"
		<< ";--------------------------------------------------------------------------------" 
		<< "\n";
	
	/** Code Proper **/
	output << "\tAREA Program, CODE\n\tENTRY\n";	
	
	//NOTE Initial Code generated assumes ALL the variables are in registers. It is the code generator that has to take care of the stack and what not
	for (it = CodeLines.begin(); it < CodeLines.end(); it++){
		std::shared_ptr<AsmLine> line = *it;
		
		if (line -> GetLabel() != nullptr){
			output << line -> GetLabel()->GetID();
		}
		output << "\t";
		
		output << line -> GetOpCodeStr() << " ";
		//output << " " << line -> GetRd() -> GetImmediate() << "\n";
		
		//TODO
		output << "R1, ";
		output << line -> GetRm() -> GetImmediate();
		
		output << "\n";
	}	
	
	
	
	/** StdLib **/
	try{
		output << ReadFile(Flags.AsmStdLibPath.c_str());
	}
	catch(...){
		HandleError("StdLib file for assembly does not exist.", E_GENERIC, E_FATAL);
	}
	
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
	CodeLines.push_back(line);
	
	return line;
}

std::shared_ptr<AsmLine> AsmFile::CreateAssignmentLine(std::shared_ptr<Symbol> sym, std::shared_ptr<Token_Expression> expr){
	std::shared_ptr<AsmLine> line;
	line = FlattenExpression(expr);
	std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
	
	Rd -> SetSymbol(sym);
	
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
	return LHS == RHS ? TypeCompatible : TypeIncompatible;	//TODO more checks
}

std::shared_ptr<AsmLine> AsmFile::FlattenExpression(std::shared_ptr<Token_Expression> expr, bool cmp){
	std::shared_ptr<AsmLine> result;
	
	//Check for simplicity
	std::shared_ptr<Token_Factor>simple = expr -> GetSimple();
	
	//This is a simple expression - CMP doesn't make sense here
	if (simple != nullptr){
		//std::cout << expr -> GetLine() << ":" << expr -> GetColumn() << "\n";
		
		//Handle based on form
		Token_Factor::Form_T Form = simple -> GetForm();
		
		//Variable reference
		if (Form == Token_Factor::VarRef){
			if (simple -> IsNegate()){
				//result = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
			}
			else{
				//result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
			}
			
			//Create AsmOp for variable - TODO
			std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Register, AsmOp::Rm ) );
			//Rm -> SetSymbol();
		}
		//Constant
		else if (Form == Token_Factor::Constant){
			if (simple -> IsNegate()){
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
			}
			else{
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
			}
			std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Immediate, AsmOp::Rm ) );
			Rm -> SetImmediate(simple -> GetValueToken() -> AsmValue());
			
			result -> SetRm(Rm);
		}
		return result;
	}
	return result;
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
	Type(Type), OpCode(OpCode)
{
	InitialiseStaticMaps();
}

AsmLine::AsmLine(const AsmLine &obj):
	OpCode(obj.OpCode),
	Condition(obj.Condition),
	Qualifier(obj.Qualifier),
	Type(obj.Type),
	Label(obj.Label),
	Rd(obj.Rd), Rm(obj.Rm), Rn(obj.Rn),
	Comment(obj.Comment)
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
		Comment = obj.Comment;
	}
	return *this;
}

std::string AsmLine::GetOpCodeStr() const{
	return OpCodeStr[OpCode];
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
	}
}

/**
 * 	AsmOp
 * */

AsmOp::AsmOp(Type_T Type, Position_T Position):
	Type(Type), Position(Position), Scale(AsmOp::NoScale), sym(nullptr), OffsetAddressOp(nullptr), ScaleOp(nullptr)
{}

AsmOp::AsmOp(const AsmOp &obj):
	Type(obj.Type), Position(obj.Position), Scale(obj.Scale), sym(obj.sym), 
	OffsetAddressOp(obj.OffsetAddressOp), ScaleOp(obj.ScaleOp),
	ImmediateValue(obj.ImmediateValue)
{}

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

AsmBlock::AsmBlock(AsmBlock::Type_T type, std::shared_ptr<Token> tok): Type(type), TokenAssoc(tok)
{
	if (type == Global)
		ID = "GLOBAL";
	else if (TokenAssoc != nullptr)
		ID = TokenAssoc -> GetStrValue();
	else{
		std::stringstream temp;
		temp << "block_" << count;
		ID = temp.str();
	}
	
	count++;
}

AsmBlock::AsmBlock(const AsmBlock& obj): 
	SymbolList(obj.SymbolList), Type(obj.Type), ChildBlocks(obj.ChildBlocks), TokenAssoc(obj.TokenAssoc), ID(obj.ID)
{
}

AsmBlock AsmBlock::operator=(const AsmBlock &obj){
	if (&obj != this){
		Type = obj.Type;
		SymbolList = obj.SymbolList;
		ChildBlocks = obj.ChildBlocks;
		TokenAssoc = obj.TokenAssoc;
		ID = obj.ID;
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