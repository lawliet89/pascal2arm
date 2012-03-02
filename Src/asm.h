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
	AsmBlock(Type_T type);		//TODO associate with a line
	~AsmBlock(){}
	AsmBlock (const AsmBlock &obj);
	AsmBlock operator=(const AsmBlock &obj);
	
	//Assign one symbol to the list
	void AssignSymbol(std::shared_ptr<Symbol>) throw(AsmCode);		//Assign one symbol
	
	//Call the function with iterator or pointers of type iterator
	/*
	template <typename iterator> void AssignSymbols(iterator start, iterator end){
		SymbolList.insert(start, end);
	}
	*/
	
	std::shared_ptr<Symbol> GetSymbol(std::string) throw(AsmCode);
	AsmCode CheckSymbol(std::string) throw();
	
protected:
	Type_T Type;		//Type of block & scope
	//List of symbols defined in this block
	std::map<std::string, std::shared_ptr<Symbol> > SymbolList; 
	

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
	
	//Create Type symbol
	std::pair<std::shared_ptr<Symbol>, AsmCode> CreateTypeSymbol(std::string ID, Token_Type::P_Type pri, int sec=0) throw(AsmCode);
	std::shared_ptr<Symbol> GetTypeSymbol(std::string id) throw(AsmCode);
	
	//Create variable symbols from Identifier List Tokens
	//void CreateVarSymbolsFromList(const Token_IDList &list, int PrimaryType, int SecondaryType, std::shared_ptr<Token> value=nullptr, std::shared_ptr<AsmBlock> block=nullptr) throw(AsmCode);
	
	/** Block Related Methods **/
	//Block Stack
	std::shared_ptr<AsmBlock> GetCurrentBlock(); //Current blocks
	void PushBlock(std::shared_ptr<AsmBlock>);
	void PopBlock();
	
	//Blocks
	std::shared_ptr<AsmBlock> CreateBlock(AsmBlock::Type_T type);	//TODO Block name/ID
	
	/** Code Related Methods **/
	//Generate Code
	void GenerateCode(std::stringstream &output);
		
protected:
	//Lines
	std::vector<std::shared_ptr<AsmLine> > CodeLines;		//Lines for normal program code
	std::vector<std::shared_ptr<AsmLine> > DataLines;		//Lines for data declaration
	std::vector<std::shared_ptr<AsmLine> > FunctionLines;	//Lines for procedures and functions
	
	//Storage of labels
	//std::map<std::string, std::shared_ptr<AsmLabel> > LabelList;		//List of labels
	
	//Symbols storage
	//std::multimap<std::string, std::shared_ptr<Symbol> > SymbolList;		//Storage of all symbols
	//std::map<std::string, std::shared_ptr<Symbol> > TypeList;	
	
	//AsmRegister Registers;		//State of registers- used when generating code
	
	//Storage of blocks
	std::vector<std::shared_ptr<AsmBlock> > BlockList; 
	
	//Context related - use iterators to handle removing and adding of stuff during context switch
	//Maybe stack?
	std::vector<std::shared_ptr<AsmBlock> > BlockStack;
	//std::vector<std::shared_ptr<Symbol> > SymbolStack;	
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
		Interrupt		//SWI etc.
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
	~AsmLine();
	AsmLine(const AsmLine &obj);
	AsmLine operator=(const AsmLine &obj);

	
protected:
	/** Data Members **/
	OpCode_T OpCode;
	CC_T Condition;
	Qualifier_T Qualifier;
	OpType_T Type;
	
	std::shared_ptr<AsmLabel> Label;
	
	//AsmOp Rd, Rm, Rn;
	
	/** Constructor Protected **/
	AsmLine();
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
	
	enum Type_T{
		Destination, OP1, OP2
	};
	
protected:
	Type_T Type;
	std::shared_ptr<Symbol> Sym;	//Symbol associated, if any
	
};

/** AsmLabel
 *	List of labels linked to lines and symbols and scope**/
class AsmLabel{
public:
	friend class AsmFile;
protected:
	std::shared_ptr<Symbol> Sym;	//Symbol associated, if any
	std::shared_ptr<AsmLine> Line;	//Associated Line
};

/** AsmRegister
 * 
 * State of registers during run time generation
 * */
class AsmRegister{
	
};
#endif