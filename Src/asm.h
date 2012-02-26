#ifndef ASMH
#define ASMH
#include <vector>
#include <string>
#include <fstream>
#include <memory>	//C++11
//#include <map>
#include "symbols.h"

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
/*
 * 	Assembly file class
 * 
 * */
class AsmFile{
public:
	//OCCF
	AsmFile();
	~AsmFile();	//Destroy all the pointers
	AsmFile(const AsmFile &obj);
	AsmFile operator=(const AsmFile &obj);
	
	
protected:
	//Lines
	std::vector<AsmLine> CodeLines;		//Lines for normal program code
	std::vector<AsmLine> DataLines;		//Lines for data declaration
	std::vector<AsmLine> FunctionLines;	//Lines for procedures and functions
	
	//Storage of labels
	//std::vector<std::shared_ptr<AsmLabel> > LabelList;		//List of labels
	
	//std::vector<std::shared_ptr<Symbol> > SymbolList;		//Storage of all symbols
	//std::vector<std::shared_ptr<Scope> > ScopeList;		//List of all scopes
	//std::vector<std::shared_ptr<ScopeStack> > ScopeStackList;	//List of scope stacks
	
	//AsmRegister Registers;		//State of registers- used when generating code
	
	//Symbols
	
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
	
	
	/*
	 * 	Type definition
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
	
	
	//Lookup table method to translate enum int codes to string
	//std::string TranslateStr(Qualifier_T);
	//Overloaded of course
	
protected:
	OpCode_T OpCode;
	CC_T Condition;
	Qualifier_T Qualifier;
	OpType_T Type;
	
	std::shared_ptr<AsmLabel> Label;
	
	//AsmOp Rd, Rm, Rn;
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
};

/** AsmRegister
 * 
 * 
 * */
class AsmRegister{
	
};

#endif