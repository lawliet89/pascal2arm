#ifndef TokenVar
#define TokenVar
#include <memory>
#include <string>
#include "../token.h"
#include "../symbols.h"
#include "../define.h"
#include "type.h"

//Forward declaration
class AsmFile;

class Token_Var: public Token{
public:
	friend class AsmFile;
	//OCCF
	Token_Var(std::string id, std::shared_ptr<Token_Type> type, bool temp=false);
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
	void SetType(std::shared_ptr<Token_Type> type){ Type = type; }
	std::shared_ptr<Token_Type> GetType(){ return Type; }
	
	//Symbol
	const Symbol* GetSymbol() const{ return Sym.get(); }
	
	bool GetTemp() const{ return Temp; }
	void SetTemp(bool val) { Temp = val;}
	
protected:
	std::string id;
	std::shared_ptr<Token> value;
	std::shared_ptr<Token_Type> Type;
	std::shared_ptr<Symbol> Sym;	//Associated Symbol
	bool Temp;			//Whether a variable is temp
	
	//Only AsmFile can set the symbol
	void SetSymbol(std::shared_ptr<Symbol> ptr){
		Sym = ptr;
	}
};
#endif