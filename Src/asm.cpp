#include "asm.h"

/**
 * 	AsmFile
 * */

//Constructor
AsmFile::AsmFile(){
	//Initialise lines - maybe procedure to the default ones?
	
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