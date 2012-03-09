#include "asm.h"
#include "utility.h"
#include "op.h"
#include <iostream>
#include <set>
#include <climits>

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
		std::shared_ptr<Token_Var> ptr(new Token_Var(value.first, type));	
		try{
			std::pair<std::shared_ptr<Symbol>, AsmCode> sym = CreateSymbol(Symbol::Variable, value.first, std::dynamic_pointer_cast<Token>(ptr));	
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

	
	//NOTE Initial Code generated assumes ALL the variables are in registers. It is the code generator that has to take care of the stack and what not
	for (it = CodeLines.begin(); it < CodeLines.end(); it++){
		std::shared_ptr<AsmLine> line = *it;
		std::shared_ptr<AsmOp> Rd, Rm, Rn;
		std::pair<std::string, std::string> RdOutput, RmOutput, RnOutput;
		
		Rd = line -> GetRd();
		Rm = line -> GetRm();
		Rn = line -> GetRn();

		/** Rm **/ //TODO
		if (Rm != nullptr){
			//Check its type - Register, Literal, LSL, LSR, ASR, ROR, RRX
			AsmOp::Type_T RmType = Rm -> GetType();
			if (RmType == AsmOp::Register){
				RmOutput = GetCurrentBlock()->GetRegister() -> GetVarRead( Rm -> GetSymbol() );
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
				RnOutput = GetCurrentBlock()->GetRegister() -> GetVarRead( Rn -> GetSymbol() );
			}
			else if (RnType == AsmOp::Immediate){
				RnOutput.first = Rn -> GetImmediate();
			}
			output << RnOutput.second;
		}
		
		/** Rd **/ //TODO More than just destination registers?
		RdOutput = GetCurrentBlock()->GetRegister() -> GetVarWrite( Rd -> GetSymbol() );		//Because Rd can only be a register...
		output << RdOutput.second;
		
		//Label
		if (line -> GetLabel() != nullptr){
			output << line -> GetLabel()->GetID();
		}
		output << "\t";
		//Opcode
		output << line -> GetOpCodeStr() << " ";
		
		if (!RdOutput.first.empty())
			//Rd
			output << RdOutput.first;
		
		if (!RmOutput.first.empty())
			//Rm
			output << ", " << RmOutput.first;
		if (!RnOutput.first.empty())
			//Rn
			output << ", " << RnOutput.first;
		
		//Comments
		std::string comment = line -> GetComment();
		if (!comment.empty())
			output << " ;" << comment;
		
		//EOL
		output << "\n";
	}	
	
	
	
	/** StdLib **/
	try{
		output << ReadFile(Flags.AsmStdLibPath.c_str());
	}
	catch(...){
		HandleError("StdLib file for assembly does not exist.", E_GENERIC, E_FATAL);
	}
	
	/** User functions/procedures **/
	
	
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
	
	std::shared_ptr<AsmOp> Rd(new AsmOp(AsmOp::Register, AsmOp::Rd));
	Rd -> SetSymbol(sym);
	
	line = FlattenExpression(expr, Rd);
	
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

std::shared_ptr<AsmLine> AsmFile::FlattenExpression(std::shared_ptr<Token_Expression> expr, std::shared_ptr<AsmOp> Rd, bool cmp){
	std::shared_ptr<AsmLine> result;
	
	//Check for strict simplicity
	std::shared_ptr<Token_Factor>simple = expr -> GetSimple();
	
	//This is a strictly simple expression - CMP doesn't make sense here
	if (simple != nullptr){
		//std::cout << expr -> GetLine() << ":" << expr -> GetColumn() << "\n";
		
		//Handle based on form
		Token_Factor::Form_T Form = simple -> GetForm();
		
		//Variable reference
		if (Form == Token_Factor::VarRef){
			if (simple -> IsNegate()){
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MVN);
			}
			else{
				result = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
			}
			
			//Create AsmOp for variable
			std::shared_ptr<AsmOp> Rm(new AsmOp( AsmOp::Register, AsmOp::Rm ) );

			Rm -> SetSymbol( std::dynamic_pointer_cast<Token_Var>(simple -> GetValueToken()) -> GetSymbol() );
			result -> SetRm(Rm);
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
		result -> SetRd(Rd);
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
			
		}
		else{
			//In this case we definitely have to generate temporary variables already. We will use the existing Rd for LHS
			std::shared_ptr<AsmLine> LHS;
			std::shared_ptr<AsmOp> RHS(new AsmOp(AsmOp::Register, AsmOp::Rd));
			//Flatten Expression on LHS
			LHS = FlattenExpression(expr -> GetExpression(), std::shared_ptr<AsmOp>( new AsmOp(*Rd)));
			
			std::shared_ptr<Symbol> RHSTemp = CreateTempVar(expr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
			expr -> SetTempVar(RHSTemp);
			RHS -> SetSymbol(RHSTemp);
			//and generate a temporary variable for RHS
			//Flatten expression on RHS
			RHS = FlattenSimExpression(expr -> GetSimExpression(), RHS);
			
			Op_T Op = expr -> GetOp(); //TODO
			
			if (Op == LT){
				
			}
			else if (Op == LTE){
				
			}
			else if (Op == GT){
				
			}
			else if (Op == GTE){
				
			}
			else if (Op == Equal){
				//if (cmp)
				//	result = CreateCodeLine(AsmLine::Processing, AsmLine::CMP);
				//else
				//	result = CreateCodeLine(AsmLine::Processing, AsmLine::AND);
			}
			else if (Op == NotEqual){
				
			}
			else if (Op == In){
				
			}
			result -> SetRm(LHS->GetRd());
			result -> SetRn(RHS);
		}
		if (!cmp)
			result -> SetRd(Rd);		
	}
	result -> SetComment("Line " + ToString<int>(expr -> GetLine()));
	return result;
}

//Terms plus minus
std::shared_ptr<AsmOp> AsmFile::FlattenSimExpression(std::shared_ptr<Token_SimExpression> simexpr, std::shared_ptr<AsmOp> Rd){
	std::shared_ptr<AsmOp> result = Rd;	//Most likely
	std::shared_ptr<AsmLine> line;
	
	if (simexpr -> IsSimple()){
		result = FlattenTerm(simexpr -> GetTerm(), Rd);
	}
	else{
		//It's a plus minus or xor
		std::shared_ptr<AsmOp> LHS, RHS(new AsmOp(AsmOp::Register, AsmOp::Rd));
		LHS = FlattenSimExpression(simexpr -> GetSimExpression(), std::shared_ptr<AsmOp>( new AsmOp(*Rd)));		//Clone.
		
		std::shared_ptr<Symbol> RHSTemp = CreateTempVar(simexpr -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
		RHS -> SetSymbol(RHSTemp);
		//and generate a temporary variable for RHS		
		
		RHS = FlattenTerm(simexpr -> GetTerm(), RHS);
		
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
		}
		else{
			if (Op == Add){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
			}
			else if (Op == Subtract){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::SUB);
			}
			else if (Op == Or){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ORR);
			}
			else if (Op == Xor){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::EOR);
			}
			
			line -> SetRd(Rd);
			line -> SetRm(LHS);
			line -> SetRn(RHS);
			line -> SetComment("Line " + ToString<int>(simexpr -> GetLine()));
			Rd -> SetType(AsmOp::Register);			//We've done calculation
		}
	}
	
	return result;
}

std::shared_ptr<AsmOp> AsmFile::FlattenTerm(std::shared_ptr<Token_Term> term, std::shared_ptr<AsmOp> Rd){
	std::shared_ptr<AsmOp> result = Rd;		//Most likely
	std::shared_ptr<AsmLine> line;
	
	if (term -> IsSimple()){		//TODO Strict simplicity optimisation
		/*std::shared_ptr<AsmOp> factor =*/ FlattenFactor(term -> GetFactor(), Rd);
		//line  = CreateCodeLine(AsmLine::Processing, AsmLine::MOV);
		//line -> SetRd(Rd);
		//line -> SetRm(factor);
	}
	else{
		//It's a * / div mod and
		std::shared_ptr<AsmOp> LHS, RHS(new AsmOp(AsmOp::Register, AsmOp::Rd));		//RHS has to use a temp variable
		LHS = FlattenTerm(term -> GetTerm(), std::shared_ptr<AsmOp>( new AsmOp(*Rd))); //Clone
		
		std::shared_ptr<Symbol> RHSTemp = CreateTempVar(term -> GetType());		//TODO - Check for strict simplicity to reduce temp var usage
		term -> SetTempVar(RHSTemp);
		RHS -> SetSymbol(RHSTemp);			

		RHS = FlattenFactor(term -> GetFactor(), RHS);
		
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
		}
		else{
			if (Op == Multiply){		//TODO Rd and Rm must be different...?
				line = CreateCodeLine(AsmLine::Processing, AsmLine::MUL);
			}
			else if (Op == Divide){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
				line -> SetComment("Unsupported, for now");
			}
			else if (Op == Div){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
				line -> SetComment("Unsupported, for now");
			}
			else if (Op == Mod){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::ADD);
				line -> SetComment("Unsupported, for now");
			}
			else if (Op == And){
				line = CreateCodeLine(AsmLine::Processing, AsmLine::AND);
			}
			
			line -> SetRd(Rd);
			line -> SetRm(LHS);
			line -> SetRn(RHS);
			line -> SetComment("Line " + ToString<int>(term -> GetLine()));
			Rd -> SetType(AsmOp::Register);			//We've done calculation
		}
	}
	
	return result;
}

std::shared_ptr<AsmOp> AsmFile::FlattenFactor(std::shared_ptr<Token_Factor> factor, std::shared_ptr<AsmOp> Rd){
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
		//TODO
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
	ImmediateValue(obj.ImmediateValue), tok(obj.tok)
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

AsmBlock::AsmBlock(AsmBlock::Type_T type, std::shared_ptr<Token> tok): Type(type), TokenAssoc(tok), Register(new AsmRegister(type == AsmBlock::Global))
{
	if (type == Global){
		ID = "GLOBAL";
	}
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
	SymbolList(obj.SymbolList), Type(obj.Type), ChildBlocks(obj.ChildBlocks), TokenAssoc(obj.TokenAssoc), ID(obj.ID), Register(obj.Register)
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

/** AsmRegisters **/
AsmRegister::AsmRegister(bool IsGlobal) : 
	Registers(AsmUsableReg, AsmRegister::State_T(IsGlobal)), counter(0), InitialUse(0)
{

}

AsmRegister::AsmRegister(unsigned FuncRegister):
	Registers(AsmUsableReg, AsmRegister::State_T(false)), counter(0), InitialUse(FuncRegister-1)
{
	for (unsigned i = 0; i < FuncRegister; i++){
		GetRegister(i).Permanent = true;
		GetRegister(i).BelongToScope = true;
	}
}

AsmRegister::AsmRegister(const AsmRegister& obj):
	Registers(obj.Registers), counter(obj.counter), InitialUse(obj.InitialUse)
{

}

AsmRegister AsmRegister::operator=(const AsmRegister& obj)
{
	if (this != &obj){
		Registers = obj.Registers;
		counter = obj.counter;
		InitialUse = obj.InitialUse;
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
			State_T Register = GetRegister(i);
			if (!Register.Permanent && Register.LastUsed < counter){
				counter = Register.LastUsed;
				result = i;
				candidate = true;
			}
		}
		
		if (!candidate)		//Unlikely. In the event someone actually sets all AsmUsableReg registers as permanent....
			throw;
		
		ToReturn.first = result;
		State_T Register = GetRegister(result);
		
		//Check if register has been written to and if the variable is a temporary
		if (Register.WrittenTo && !Register.sym->GetTokenDerived<Token_Var>() -> IsTemp()){
			output << "\tLDR R" << AsmScratch << ", =" << Register.sym -> GetLabel() -> GetID() << "\n";
			output << "\tSTR R" << result << ", [R" << AsmScratch << "]\n";
		}
	}
	if (sym != nullptr && load){
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
	}
	else{
		//Found
		result.first = "R" + ToString<unsigned>(sym.second);
		GetRegister(sym.second).LastUsed = counter;
		GetRegister(sym.second).WrittenTo = true;
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