#ifndef TokenVar
#define TokenVar
#include <memory>
#include "../token.h"
#include "../symbols.h"

//Forward declaration
class AsmFile;

/** Types **/
/*
 Primary Type (mutually exclusive):
	- Integer
	- Real
	- Boolean
	- Record
	- Enum
	- Char
	- String
	- File
	- Set

 Secondary Type (not mutually exclusive):
	- Subrange
	- Array
	- Pointer
 * 
 */

class Token_Var: public Token{
public:
	friend class AsmFile;
	//OCCF
	Token_Var(std::string id);
	Token_Var(const Token_Var &obj);
	
	~Token_Var() { }
	Token_Var operator=(const Token_Var &obj);
	
	//Token Value
	void SetValue(std::shared_ptr<Token> value){ this->value = value; }
	
	virtual const void * GetValue() const { return (void *) value.get(); }
	virtual const void * operator()() const { return (void *) value.get(); }
	
	//ID
	std::string GetID() const { return GetStrValue(); }
	
	//Type
	int GetPrimaryType() const{ return PrimaryType; }
	int GetSecondaryType() const{ return SecondaryType; }
	
	//This is because of Pascal's Syntax
	void SetPrimaryType(int pri){ PrimaryType = pri; }
	void SetSecondaryType(int sec){ SecondaryType = sec; }
	
	//Symbol
	const Symbol* GetSymbol() const{ return Sym.get(); }
	
protected:
	std::shared_ptr<Token> value;
	int PrimaryType, SecondaryType;
	std::shared_ptr<Symbol> Sym;	//Associated Symbol
	
	//Only AsmFile can set the symbol
	void SetSymbol(std::shared_ptr<Symbol> ptr){
		Sym = ptr;
	}
};
#endif