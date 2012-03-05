#ifndef ASMH
#define ASMH
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <memory>	//C++11
#include <map>
#include "token.h"
#include "Gen/all.h"
#include "symbols.h"
#include "define.h"

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
	
	//Getters
	std::shared_ptr<Symbol> GetSymbol(std::string) throw(AsmCode);
	AsmCode CheckSymbol(std::string) throw();
	std::map<std::string, std::shared_ptr<Symbol> > GetList(){ return SymbolList; }
	std::vector<std::shared_ptr<AsmBlock> > GetChildBlocks(){ return ChildBlocks; }
	std::shared_ptr<Token> GetToken(){ return TokenAssoc; }
	std::string GetID() const{ return ID; }
	Type_T GetType() const{ return Type; }
	
protected:
	Type_T Type;		//Type of block & scope
	//List of symbols defined in this block
	std::map<std::string, std::shared_ptr<Symbol> > SymbolList; 
	std::vector<std::shared_ptr<AsmBlock> > ChildBlocks;	//List of child blocks declared in this block - this is different from BlockStack!
	std::shared_ptr<Token> TokenAssoc;		//Associated token, if any
	std::string ID;
	
	static int count;
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
	void CreateVarSymbolsFromList(std::shared_ptr<Token_IDList> IDList, std::shared_ptr<Token_Type> type, std::shared_ptr<Token> InitialValue=nullptr);
	
	//Create temporary variables for complex expressions
	void CreateTempVarSymbol();	//TODO
	
	//Function and procedures
	std::pair<std::shared_ptr<Symbol>, AsmCode> CreateProcSymbol(std::string ID); //Create for the current block
	
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
	
	/** Label Related Methods **/
	std::shared_ptr<AsmLabel> CreateLabel(std::string ID, std::shared_ptr<Symbol> sym=nullptr, std::shared_ptr<AsmLine> Line = nullptr) throw(AsmCode);
	
	/** Compiler Debug Methods **/
	void PrintSymbols();
	void PrintBlocks();
	
protected:
	//Lines
	std::vector<std::shared_ptr<AsmLine> > CodeLines;		//Lines for normal program code
	std::vector<std::shared_ptr<AsmLine> > DataLines;		//Lines for data declaration
	std::vector<std::shared_ptr<AsmLine> > FunctionLines;	//Lines for procedures and functions
	
	//Storage of labels
	std::map<std::string, std::shared_ptr<AsmLabel> > LabelList;		//List of labels

	
	//AsmRegister Registers;		//State of registers- used when generating code
	
	//Storage of blocks
	std::vector<std::shared_ptr<AsmBlock> > BlockList; 
	std::vector<std::shared_ptr<AsmBlock> > BlockStack;
	std::shared_ptr<AsmBlock> GlobalBlock;
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
		Multiply,
		Floating,
		Data,			//Load store etc
		Branch,
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
		EQU	//http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0041c/Caccddic.html
		//INCLUDE	?
	};
	enum CC_T{	//Condition Code
		CS, SQ, VS, GT, GE, PL, HI, HS, CC, NE, VC, LT, LE, MI, LO, LS,
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
	void SetRd(std::shared_ptr<AsmOp> op){ Rd = op; }
	void SetRm(std::shared_ptr<AsmOp> op){ Rm = op; }
	void SetRn(std::shared_ptr<AsmOp> op){ Rn = op; }
	void SetComment(std::string val){ Comment = val; }
	
	//Getters
	OpCode_T GetOpCode() const { return OpCode; }
	CC_T GetCC() const { return Condition; }
	Qualifier_T GetQualifier() const { return Qualifier; }
	OpType_T GetType() const { return Type; }
	std::shared_ptr<AsmLabel> GetLabel(){ return Label; }
	std::shared_ptr<AsmOp> GetRd(){ return Rd; }
	std::shared_ptr<AsmOp> GetRm(){ return Rm; }
	std::shared_ptr<AsmOp> GetRn(){ return Rn; }
	std::string GetComment() const{ return Comment; }
	
	std::string GetOpCodeStr() const;
	
protected:
	/** Data Members **/
	OpCode_T OpCode;
	CC_T Condition;
	Qualifier_T Qualifier;
	OpType_T Type;
	
	std::shared_ptr<AsmLabel> Label;
	
	std::shared_ptr<AsmOp> Rd, Rm, Rn;
	
	std::string Comment;	
	
	/** Constructor Protected **/
	AsmLine(OpType_T Type, OpCode_T OpCode);
	
	/** Static Members **/
	static std::map<AsmLine::OpCode_T, std::string> OpCodeStr;
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
		Destination, OP1, OP2
	};
	
	enum Type_T{
		Register, Immediate, Address, OffsetAddr
		//Note Complicated shit like LDR R0, [R1,R2,LSL #2]!
	};
	
	enum Scale_T{
		NoScale,
		LSL, LSR, ASR, ROR, RRX
	};
	
	
	AsmOp(Type_T Type, Position_T Position);
	~AsmOp() { }
	AsmOp(const AsmOp &obj);
	AsmOp operator=(const AsmOp &obj);
	
	void SetScale(Scale_T scale){ Scale = scale; }
	void SetWriteBack(bool val) { WriteBack = val;}
	void SetSymbol(std::shared_ptr<Symbol> val) { sym = val; }
	void SetOffsetAddressOp(std::shared_ptr<AsmOp> val) { OffsetAddressOp = val; }
	void SetScaleOp(std::shared_ptr<AsmOp> val) { ScaleOp = val; }
	void SetImmediate(std::string val) { ImmediateValue = val; }
	
	//Getters
	Type_T GetType() const { return Type; }
	Position_T GetPosition() const { return Position; }
	Scale_T GetScale() const { return Scale; }
	bool GetWriteBack() const { return WriteBack; }
	std::shared_ptr<Symbol> GetSymbol() { return sym; }
	std::shared_ptr<AsmOp> GetOffsetAddressOp() { return OffsetAddressOp; }
	std::shared_ptr<AsmOp> GetScaleOp() { return ScaleOp; }
	std::string GetImmediate() const { return ImmediateValue; }
	
protected:
	Type_T Type;
	Position_T Position;
	Scale_T Scale;
	bool WriteBack;			//The ! thing
	std::shared_ptr<Symbol> sym;	//Symbol associated, if any
	
	std::shared_ptr<AsmOp> OffsetAddressOp;		//If the type is OffsetAddr -initialise to nullptr
	std::shared_ptr<AsmOp> ScaleOp;
	std::string ImmediateValue;			//Value of immediate
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
	
};
#endif