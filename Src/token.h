#ifndef TokenH
#define TokenH

#define WORD_SIZE 4

#include <string>
#include <memory>
#include "utility.h"
#include "define.h"

//Forward declaration
class Symbol;
/*
 * Token base class that can be derived for the various data types
 * 
 * */

class Token{		//Base class is sufficient for normal strings. Does not contain ID field
public:
	//OCCF
	
	//Constructor
	Token(const char *StrValue, T_Type type, bool IsConstant=true);	//Type is the token type as defined by parser.h (generated from parser.y)
	Token(std::string StrValue, T_Type type, bool IsConstant=true);	
	virtual ~Token(){ }				//Destructor
	Token(const Token &obj);		//Copy Constructor
	virtual Token operator=(const Token &obj);	//Assignment operator
	
	//Virtual methods
	virtual std::string GetStrValue() const { return StrValue; }
	virtual const char* GetCStrValue() const { return StrValue.c_str(); }
	T_Type GetType() const { return type; }
	
	//Return type is set to void * so that derived classes can override this
	//Use the utility function DereferenceVoidPtr<T> to dereference this
	//where T is the intended type
	virtual const void * GetValue() const { return (void *) &StrValue; }
	virtual const void * operator()() const { return (void *) &StrValue; }
	
	void SetSymbol(std::shared_ptr<Symbol> symbol){ Sym = symbol; }
	std::shared_ptr<Symbol> GetSymbol(){ return Sym; }
	
	bool GetIsConstant() const { return IsConstant; }
	unsigned GetLine() const { return line; }
	unsigned GetColumn() const { return column; }
	
	/** For Assembly Use - Immediates for ASM output **/
	//If the size is > 1 word, make sure that the default value reflects this
	virtual std::string AsmDefaultValue();
	virtual std::string AsmValue(){ return "\"" + GetStrValue() + "\"";  }
	
	
	virtual int GetSize(){ return StrValue.length(); }
	
protected:
	std::string StrValue;
	T_Type type;			//Base Type of token
	std::shared_ptr<Symbol> Sym;	//Associated symbol, if any
	bool IsConstant;		//Is this token a constant?
	
	//Record down the line and column automatically
	int line, column;
};

/**
 * 	Utility Function
 * 
 * */

template <typename T> T GetValue(std::shared_ptr<Token> token){
	return DereferenceVoidPtr<T>(token -> GetValue());
}

template <typename T> T GetValue(Token *token){
	return DereferenceVoidPtr<T>(token -> GetValue());
}

#endif