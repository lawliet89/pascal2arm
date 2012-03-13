#ifndef ASMH
#define ASMH
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>	//C++11
#include <map>
#include <set>
#include <list>
#include "token.h"
#include "Gen/all.h"
#include "symbols.h"
#include "define.h"

#define AsmUsableReg 11			//No of registers usable
#define AsmScratch 12		//Register to be the scratch register

/**
 * 
 *  Assembly "intermediate" and generation
 * 
 * */
//Forward declaration
class AsmFile;
class AsmLine;	//Lines of code
class AsmOp;	//Operators 
class AsmLabel;	//Labels
class AsmRegister;	//State of the registers and data - used when generating code
class AsmBlock;


/** AsmBlock
 * 
 * */
class AsmBlock{
public:
	friend class AsmFile;
	//According to Free Pascal, only Blocks and Record define scopes.
	//Further spefic to the three types of block scopes
	enum Type_T{
		Global,
		Procedure,
		Function,
		Record
	};
	
	//OCCF
	AsmBlock(Type_T type,std::shared_ptr<Token> tok=nullptr);		//TODO associate with a line
	~AsmBlock(){}
	AsmBlock (const AsmBlock &obj);
	AsmBlock operator=(const AsmBlock &obj);
	
	//Assign one symbol to the list
	void SetSymbol(std::shared_ptr<Symbol>) throw(AsmCode);		//Set one symbol
	void AddChildBlock(std::shared_ptr<AsmBlock> block){ ChildBlocks.push_back(block); }
	void SetToken(std::shared_ptr<Token> tok){ TokenAssoc = tok; }
	void SetID(std::string ID){ this->ID = ID; }
	void SetBlockSymbol(std::shared_ptr<Symbol> sym) { BlockSymbol = sym; }
	
	//Getters
	std::shared_ptr<Symbol> GetSymbol(std::string) throw(AsmCode);
	AsmCode CheckSymbol(std::string) throw();
	std::map<std::string, std::shared_ptr<Symbol> > GetList(){ return SymbolList; }
	std::vector<std::shared_ptr<AsmBlock> > GetChildBlocks(){ return ChildBlocks; }
	std::shared_ptr<Token> GetToken(){ return TokenAssoc; }
	std::string GetID() const{ return ID; }
	Type_T GetType() const{ return Type; }
	std::shared_ptr<AsmRegister> GetRegister(){ return Register; }
	bool IsGlobal() const { return Type == Global; }
	std::shared_ptr<Symbol> GetBlockSymbol() { return BlockSymbol; }
	
	/** Conditional Stacks **/
	//If Label Stacks
	std::shared_ptr<AsmLabel> CreateIfElseLabel();
	void IfLabelStackPush(std::shared_ptr<AsmLabel> label){ IfLabelStack.push_back(label); }
	std::shared_ptr<AsmLabel> IfLabelStackPop(){ 
		std::shared_ptr<AsmLabel> result = *IfLabelStack.rbegin();
		
		IfLabelStack.pop_back();
		return result;
	}
	//If Line Stacks
	void IfLineStackPush(std::shared_ptr<AsmLine> line){ IfLineStack.push_back(line); }
	std::shared_ptr<AsmLine> IfLineStackPop(){ 
		std::shared_ptr<AsmLine> result = *IfLineStack.rbegin();
		
		IfLineStack.pop_back();
		return result;
	}
	//For Line Stacks
	std::shared_ptr<AsmLabel> CreateForLabel();
	void ForLabelStackPush(std::shared_ptr<AsmLabel> label){ ForLabelStack.push_back(label); }
	std::shared_ptr<AsmLabel> ForLabelStackPop(){ 
		std::shared_ptr<AsmLabel> result = *ForLabelStack.rbegin();
		
		ForLabelStack.pop_back();
		return result;
	}
	
	//WhileLabelStack
	std::shared_ptr<AsmLabel> CreateWhileLabel();
	void WhileStartLabelStackPush(std::shared_ptr<AsmLabel> label){ WhileStartLabelStack.push_back(label); }
	std::shared_ptr<AsmLabel> WhileStartLabelStackPop(){ 
		std::shared_ptr<AsmLabel> result = *WhileStartLabelStack.rbegin();
		
		WhileStartLabelStack.pop_back();
		return result;
	}
	
	void WhileEndLabelStackPush(std::shared_ptr<AsmLabel> label){ WhileEndLabelStack.push_back(label); }
	std::shared_ptr<AsmLabel> WhileEndLabelStackPop(){ 
		std::shared_ptr<AsmLabel> result = *WhileEndLabelStack.rbegin();
		
		WhileEndLabelStack.pop_back();
		return result;
	}
	void InLoopStackPush(){ InLoopCount++; }
	void InLoopStackPop() { InLoopCount--; }
	bool IsInLoop() { return (InLoopCount != 0); }	
	void SetNextLabel(std::shared_ptr<AsmLabel> label) { NextLabel = label; }
	
	bool operator==(const AsmBlock &obj) const;
	bool operator!=(const AsmBlock &obj) const;
protected:
	Type_T Type;		//Type of block & scope
	//List of symbols defined in this block
	std::map<std::string, std::shared_ptr<Symbol> > SymbolList; 
	std::vector<std::shared_ptr<AsmBlock> > ChildBlocks;	//List of child blocks declared in this block - this is different from BlockStack!
	std::shared_ptr<Token> TokenAssoc;		//Associated token, if any
	std::string ID;
	std::shared_ptr<Symbol> BlockSymbol;
	
	std::shared_ptr<AsmRegister> Register;
	
	static int count;
	//Conditional stacks
	std::shared_ptr<AsmLabel> NextLabel;
	std::vector<std::shared_ptr<AsmLabel> > IfLabelStack;
	std::vector<std::shared_ptr<AsmLine> > IfLineStack;
	
	std::vector<std::shared_ptr<AsmLabel> > ForLabelStack;
	std::vector<std::shared_ptr<AsmLabel> > WhileStartLabelStack;
	std::vector<std::shared_ptr<AsmLabel> > WhileEndLabelStack;
	
	unsigned InLoopCount;
};

/**
 * 	Assembly line class
 * 		Wrapper class
 * 		Cannot be instantiated by anything other than AsmFile
 * 		Not to be accessed directly
 * 
 * */
class AsmLine{
public:
	friend class AsmFile;
	
	
	/**
	 * 	Types definition
	 */
	
	enum OpType_T{
		Directive,		//Psuedo and directive
		Processing,		//Data processing
		Data,			//Load store etc
		Branch,
		BranchLink,
		Interrupt,//SWI etc.
		CommentLine		//Purely comment
	};
	
	/* OpCode Definition */
	enum OpCode_T{
		//Data Processing
		AND=100, EOR, SUB, RSB, ADD, ADC, SBC, RSC, TST, TEQ, CMP, CMN, ORR, MOV, BIC, MVN,
		SWP,
		
		//Multiply
		MUL=200, MLA, UMULL, UMLAL, SMULL, SMLAL,
		
		//Floating points
		
		//Data Transfer
		LDR=400, STR, STM, LDM,
		
		//Branch
		B=500, BL,
		
		//Other
		SWI=600,
		
		//Psuedo and directives
		ADR=900,
		AREA, //http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Cacbjgcc.html
		END, //http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Babcfbje.html
		ENTRY, //http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Cacbbebi.html
		DATA, //http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Chebjhje.html
		//ALIGN,
		DCD,	//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Babbfcga.html
		DCB, 	//Byte version of DCD which is word
		DCFD,	//Double precision - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Caccijca.html
		DCFS,	//Single - http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Cacdagie.html
		EQU, 	//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Caccddic.html
		NOP,
		//INCLUDE	?
		
		
		//Macros
		DivMod=1000,
		
		
		//Internal Use only
		WRITE_INT=2000,	//Write int
		WRITE_C,			//Write Char
		SAVE,			//Force Rd to be saved to memory if it was written
		BLOCKPUSH,		//Push a block -- used to signify beginning of a function -- Rd should be the SYMBOL of the block.
		BLOCKPOP,		//Pop a block -- used to signify end of a function  -- Rd should be the SYMBOL of the block.
		FUNCALL,		//Rd is the return value, Rm, Rn, Ro supported i,e, max 3 args TODO for more
		NEW,
		DISPOSE
	};
	enum CC_T{	//Condition Code
		EQ, CS, SQ, VS, GT, GE, PL, HI, HS, CC, NE, VC, LT, LE, MI, LO, LS,
		AL=900,
		NV=999
	};
	
	/* Qualifiers */
	enum Qualifier_T{
		Byte,	//For byte operation - mnemoic is just B
		ED,EA,FD,FA
		//IB, IA, DB, DA	//probably won't use
	};
	
	/** Public Methods **/
	~AsmLine(){}
	AsmLine(const AsmLine &obj);
	AsmLine operator=(const AsmLine &obj);
	
	//Setter
	void SetCC(CC_T cc){ Condition = cc; }
	void SetQualifier (Qualifier_T Qualifier) { this -> Qualifier = Qualifier; }
	void SetLabel(std::shared_ptr<AsmLabel> Label){ this -> Label = Label; }
	void SetRd(std::shared_ptr<AsmOp> val){ Rd = val; }
	void SetRm(std::shared_ptr<AsmOp> val){ Rm = val; }
	void SetRn(std::shared_ptr<AsmOp> val){ Rn = val; }
	void SetRo(std::shared_ptr<AsmOp> val){ Ro = val; }
	void SetComment(std::string val){ Comment = val; }
	void SetInLoop(bool val = true) { InLoop = val; }
	
	//Getters
	OpCode_T GetOpCode() const { return OpCode; }
	CC_T GetCC() const { return Condition; }
	Qualifier_T GetQualifier() const { return Qualifier; }
	OpType_T GetType() const { return Type; }
	std::shared_ptr<AsmLabel> GetLabel(){ return Label; }
	std::shared_ptr<AsmOp> GetRd(){ return Rd; }
	std::shared_ptr<AsmOp> GetRm(){ return Rm; }
	std::shared_ptr<AsmOp> GetRn(){ return Rn; }
	std::shared_ptr<AsmOp> GetRo(){ return Ro; }
	std::string GetComment() const{ return Comment; }
	
	std::string GetOpCodeStr() const;
	bool IsInLoop() const { return InLoop; }
	
protected:
	/** Data Members **/
	OpCode_T OpCode;
	CC_T Condition;
	Qualifier_T Qualifier;
	OpType_T Type;
	
	std::shared_ptr<AsmLabel> Label;
	
	std::shared_ptr<AsmOp> Rd, Rm, Rn, Ro;		//For some OpCode, there is an Ro
	
	std::string Comment;	
	
	//Line flags
	bool InLoop;
	
	/** Constructor Protected **/
	AsmLine(OpType_T Type, OpCode_T OpCode);
	
	/** Static Members **/
	static std::map<AsmLine::OpCode_T, std::string> OpCodeStr;
	static std::map<AsmLine::CC_T, std::string> CCStr;
	static void InitialiseStaticMaps();
};

/**
 * 	Assembly operators
 * 		Wrapper class
 * 		Cannot be instantiated by anything other than AsmFile
 * 		Not to be accessed directly
 * */
class AsmOp{
public:
	friend class AsmLine;
	friend class AsmFile;

	//Register
	//Immediate
	//Shifts 
	//Indexing
	//Type?
	//STMED write back '!'
	//LDM, STM qualifiers
	
	//Detination Register
	//Update '!'
	//OP1
	/*	Register, Literal, LSL, LSR, ASR, ROR, RRX
	 * 
	 * */
	//OP2
	/*
	 * 	Register, Literal Constant, Scaled, offset addressing
	 * 
	 * */
	
	enum Position_T{
		Rd, Rm, Rn, Ro
	};
	
	enum Type_T{
		Register, Immediate, CodeLabel
		//Note Complicated shit like LDR R0, [R1,R2,LSL #2]!
	};
	
	enum Scale_T{
		NoScale,
		LSL, LSR, ASR, ROR, RRX
	};
	
	
	AsmOp(Type_T Type, Position_T Position);
	AsmOp(std::shared_ptr<AsmOp> ptr);
	~AsmOp() { }
	AsmOp(const AsmOp &obj);
	AsmOp operator=(const AsmOp &obj);
	
	void SetType(Type_T type) { Type = type; }
	void SetScale(Scale_T scale){ Scale = scale; }
	void SetWriteBack(bool val) { WriteBack = val;}
	void SetSymbol(std::shared_ptr<Symbol> val) { sym = val; }
	void SetOffsetAddressOp(std::shared_ptr<AsmOp> val) { OffsetAddressOp = val; }
	void SetScaleOp(std::shared_ptr<AsmOp> val) { ScaleOp = val; }
	void SetImmediate(std::string val) { ImmediateValue = val; }
	void SetToken(std::shared_ptr<Token> tok){ this -> tok = tok; }
	void SetWrite(bool val = true){ Write = val; }
	void SetLabel(std::shared_ptr<AsmLabel> val){ Label = val; }
	//void SetDereference(bool val = true) { Dereference = val; }
	
	//Getters
	Type_T GetType() const { return Type; }
	Position_T GetPosition() const { return Position; }
	Scale_T GetScale() const { return Scale; }
	bool GetWriteBack() const { return WriteBack; }
	std::shared_ptr<Symbol> GetSymbol() { return sym; }
	std::shared_ptr<AsmOp> GetOffsetAddressOp() { return OffsetAddressOp; }
	std::shared_ptr<AsmOp> GetScaleOp() { return ScaleOp; }
	std::string GetImmediate() const { return ImmediateValue; }
	std::shared_ptr<Token> GetToken() { return tok;}
	bool IsWrite() const { return Write; }
	std::shared_ptr<AsmLabel> GetLabel() { return Label ;}
	//bool GetDereference() const { return Dereference; }
	
protected:
	Type_T Type;
	Position_T Position;
	Scale_T Scale;
	bool WriteBack;			//The ! thing
	std::shared_ptr<Symbol> sym;	//Symbol associated, if any
	std::shared_ptr<Token> tok;		//Token associated, if any
	std::shared_ptr<AsmLabel> Label;	//Label associated, if any
	
	std::shared_ptr<AsmOp> OffsetAddressOp;		//If the type is OffsetAddr -initialise to nullptr
	std::shared_ptr<AsmOp> ScaleOp;
	std::string ImmediateValue;			//Value of immediate
	bool Write;			//Is this written to?
	
	//Variable flags
	//bool Dereference;	//If the Op is a register, this means that the Op has a variable symbol that is to be dereferenced (i.e. variable symbol is a pointer)
};

/** AsmLabel
 *	List of labels linked to lines and symbols and scope**/
class AsmLabel{
public:
	friend class AsmFile;
	
	AsmLabel(const AsmLabel &obj);
	~AsmLabel(){}
	AsmLabel operator=(const AsmLabel &obj);
	
	void SetSymbol(std::shared_ptr<Symbol> val) { sym = val; }
	void SetLine(std::shared_ptr<AsmLine> val) { Line = val; }
	void SetID(std::string val) { ID = val; }
	
	std::shared_ptr<Symbol> GetSymbol() { return sym; }
	std::shared_ptr<AsmLine> GetLine() { return Line; }
	std::string GetID() const { return ID; }
	
protected:
	std::shared_ptr<Symbol> sym;	//Symbol associated, if any
	std::shared_ptr<AsmLine> Line;	//Associated Line
	std::string ID;		//Label ID
	
	AsmLabel(std::string ID, std::shared_ptr<Symbol> sym=nullptr, std::shared_ptr<AsmLine> Line=nullptr);
};

/** AsmRegister
 * 
 * State of registers during run time generation
 * */
class AsmRegister{
public:	
	friend class AsmFile;
	
	AsmRegister(bool IsGlobal=false);
	AsmRegister(unsigned FuncRegister);		//If this is inside a function, use this to set the number of initial registers that area already assigned. Assign symbols if neccessary
	~AsmRegister() { }
	AsmRegister(const AsmRegister &obj);
	AsmRegister operator=(const AsmRegister &obj);
	
	
	//NOTE: Take note of temp variables! -
	std::pair<std::string, std::string> GetVarRead(std::shared_ptr<Symbol> var);		//Returns a pair of string. The first refers to the operator to access the register, the second refers to any stack movement.
	std::pair<std::string, std::string> GetVarWrite(std::shared_ptr<Symbol> var);		//The first refers to the operator to access the register, the second refers to any stack movement.
	
	std::shared_ptr<Symbol> GetSymbol(unsigned ID);
	bool GetBelongToScope(unsigned ID) const;
	bool GetWrittenTo(unsigned ID) const;
	bool GetPermanent(unsigned ID) const;
	
	bool IsInLoop() const { return InLoop; }
	void SetInLoop(bool val = true) { InLoop = val; }
	
	//Aggregate Getters
	std::vector<unsigned> GetListOfNotBelong();			//Returns a list of registers that do not belong to this scope 
	
	//Special methods
	std::string SaveRegister(std::shared_ptr<Symbol> var);		//Force save variable
	std::string SaveRegister(unsigned no);			//Force save reg no
	void EvictRegister(unsigned no);
	std::string ForceVar(std::shared_ptr<Symbol> var, unsigned no, bool load=true, bool write=false, bool save=true, bool move=true);	//Force variable to be in register no
	void IncrementCounter(){ counter++; }
	std::string SaveAllRegisters();
	unsigned GetCounter() const { return counter; }
	
	void SetFunctionRegisters(unsigned count);
	
protected:
	struct State_T{		//State of registers
		std::shared_ptr<Symbol> sym;
		bool WrittenTo;			//Has the register been written to?
		bool BelongToScope;		//Set whether current register belong to scope
		unsigned LastUsed;		//Counter where register was last used
		bool Permanent;		//Set to disallow storage of this to memory. Used in a function for R0-R3
		bool WrittenBefore;	//Written to at all in its life.
		State_T(bool IsGlobal=false): WrittenTo(false), BelongToScope(IsGlobal), Permanent(false), LastUsed(0), WrittenBefore(false){}
	};
	
	std::vector<State_T> Registers;
	unsigned counter;		//Kind of like a PC. To track least recently used
	unsigned InitialUse;	//For initial tracking until all 13 gets filled up
	
	State_T &GetRegister(unsigned ID);
	const State_T &GetRegister(unsigned ID) const;
	
	//Get ID of available register
	std::pair<unsigned, std::string> GetAvailableRegister(std::shared_ptr<Symbol>sym, bool load=false);		//Set load to a symbol if you want to load its value
	
	void SetBelongToScope(unsigned ID, bool val=true);
	void SetSymbol(unsigned ID, std::shared_ptr<Symbol> sym);
	void SetWrittenTo(unsigned ID, bool val=true);
	void SetPermanent(unsigned ID, bool val=true);
	void SetInitialUse(unsigned val) { InitialUse = val; }
	std::pair<std::shared_ptr<Symbol>, unsigned> FindSymbol(std::shared_ptr<Symbol>);		//Iterate through the registers and see if symbol is already represented
	
	//If we are in a loop, variables are always saved because we don't know for sure if it will be written to
	bool InLoop;
};

/*
 * 	Assembly file class - the "main" class
 * 
 * */
class AsmFile{
public:
	//OCCF
	AsmFile();
	~AsmFile(); //Destroy all the pointers
	AsmFile(const AsmFile &obj);
	AsmFile operator=(const AsmFile &obj);
	
	void CreateGlobalScope();	//Create Global Scope and reserved symbols
	
	/** Symbols Related Methods **/
	//Creates symbol in the current scope. AsmCode gives the status
	//Will throw SymbolReserved if the symbol to be created infringes on a reserved symbol
	//Also throws SymbolExistsInCurrentBlock if it is already defined in the current block
	std::pair<std::shared_ptr<Symbol>, AsmCode> CreateSymbol(Symbol::Type_T type, std::string id, std::shared_ptr<Token> value=nullptr) throw(AsmCode);
	
	//Check if symbol with id that is accessible from the current scope exists. return AsmCode with appropriate information
	AsmCode CheckSymbol(std::string id);
	std::pair<std::shared_ptr<Symbol>, AsmCode> GetSymbol(std::string id) throw(AsmCode);	//Throws SymbolNotExists if not found
	
	/** Variables **/
	//Create Type symbol
	std::pair<std::shared_ptr<Symbol>, AsmCode> CreateTypeSymbol(std::string ID, Token_Type::P_Type pri, int sec=0) throw(AsmCode);
	std::pair<std::shared_ptr<Symbol>, AsmCode> GetTypeSymbol(std::string id) throw(AsmCode); //Throws SymbolNotExists if not found
	
	//Create permanent variable symbols from Identifier List Tokens
	std::vector<std::shared_ptr<Token_Var> > CreateVarSymbolsFromList(std::shared_ptr<Token_IDList> IDList, std::shared_ptr<Token_Type> type, std::string AsmInitialValue="");
	
	
	//Create temporary variables for complex expressions
	std::shared_ptr<Symbol> CreateTempVar(std::shared_ptr<Token_Type> type);
	
	//Function and procedures
	std::pair<std::shared_ptr<Symbol>, AsmCode> CreateProcFuncSymbol(std::string ID, bool function=false, bool block=true, bool push=true) throw(AsmCode); //Create for the current block
	
	/** Block Related Methods **/
	//Block Stack
	std::shared_ptr<AsmBlock> GetCurrentBlock(); //Current blocks
	void PushBlock(std::shared_ptr<AsmBlock>);		//Set current block
	void PopBlock();								//Return from current block
	
	std::shared_ptr<AsmBlock> CreateBlock(AsmBlock::Type_T type, std::shared_ptr<Token> tok=nullptr);	//Create block
	std::shared_ptr<AsmBlock> GetGlobalBlock(){ return GlobalBlock; }		//Get global block pointer
	
	/** Code Related Methods **/
	//Generate Code
	std::string GenerateCode();
	
	/** Line Related Methods **/
	std::shared_ptr<AsmLine> CreateDataLine(std::shared_ptr<AsmLabel> Label, std::string value);	//Pass method with a completed label
	std::shared_ptr<AsmLine> CreateCodeLine(AsmLine::OpType_T, AsmLine::OpCode_T);		//Create an empty line and add it to list
	std::pair<std::shared_ptr<AsmLine>, std::list<std::shared_ptr<AsmLine> >::iterator> CreateCodeLineIt(AsmLine::OpType_T, AsmLine::OpCode_T);	
	std::shared_ptr<AsmLine> CreateAssignmentLine(std::shared_ptr<Symbol> sym, std::shared_ptr<Token_Expression> expr);		//For assignment statements
	
	/** Label Related Methods **/
	std::shared_ptr<AsmLabel> CreateLabel(std::string ID, std::shared_ptr<Symbol> sym=nullptr, std::shared_ptr<AsmLine> Line = nullptr) throw(AsmCode);
	
	/** Statement Related Methods **/
	AsmCode TypeCompatibilityCheck(std::shared_ptr<Token_Type> LHS, std::shared_ptr<Token_Type> RHS);	//Return AsmCode
	
	//Take an expression, flatten it by generating AsmLines and return an AsmLine
	//If cmp is set to true, will be a CMP type instruction and so Rd will be undefined
	//If expression is simple, then the return is undefined. You most likely will not be using it anyway
	//Recursive
	std::shared_ptr<AsmLine> FlattenExpression(std::shared_ptr<Token_Expression> expr, std::shared_ptr<AsmOp> Rd=nullptr, bool cmp=false);	
	
	//Take a SimpleExpression, flatten it by generating AsmLines and return an AsmOp to refer to the simexpr
	std::shared_ptr<AsmOp> FlattenSimExpression(std::shared_ptr<Token_SimExpression> simexpr, std::shared_ptr<AsmOp> Rd=nullptr);
	
	//Take a term and flatten it by generating AsmLines and return an AsmOp to refer to the term
	std::shared_ptr<AsmOp> FlattenTerm(std::shared_ptr<Token_Term> term, std::shared_ptr<AsmOp> Rd=nullptr);
	
	//Take a factor and flatten it by generating AsmLines and return an AsmOp to refer to the term
	std::shared_ptr<AsmOp> FlattenFactor(std::shared_ptr<Token_Factor> factor, std::shared_ptr<AsmOp> Rd=nullptr);
	
	//Hack -- Create Write()
	void CreateWriteLine(std::shared_ptr<Token_ExprList> list);		//Only supports ONE expression 
	void CreateNewProcLine(std::shared_ptr<Token_ExprList> list);
	void CreateDisposeProcLine(std::shared_ptr<Token_ExprList> list);
	
	/** Compiler Debug Methods **/
	void PrintSymbols();
	void PrintBlocks();
	
protected:
	//Lines
	std::list<std::shared_ptr<AsmLine> > CodeLines;		//Lines for normal program code
	std::vector<std::shared_ptr<AsmLine> > DataLines;		//Lines for data declaration
	std::list<std::shared_ptr<AsmLine> > FunctionLines;	//Lines for procedures and functions
	
	//Storage of labels
	std::map<std::string, std::shared_ptr<AsmLabel> > LabelList;		//List of labels

	
	//AsmRegister Registers;		//State of registers- used when generating code
	
	//Storage of blocks
	std::vector<std::shared_ptr<AsmBlock> > BlockList; 
	std::vector<std::shared_ptr<AsmBlock> > BlockStack;
	std::shared_ptr<AsmBlock> GlobalBlock;
	

};
#endif
