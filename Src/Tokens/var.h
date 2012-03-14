#ifndef TokenVar
#define TokenVar
#include <memory>
#include <string>
#include <vector>
#include "../token.h"
#include "../symbols.h"
#include "../define.h"
#include "type.h"

//Forward declaration
class AsmFile;
class AsmOp;
class Token_Expression;

class Token_Var: public Token{
public:
	friend class AsmFile;
	//OCCF
	Token_Var(std::string id, std::shared_ptr<Token_Type> type, bool constant=false, bool temp=false);
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
	void SetVarType(std::shared_ptr<Token_Type> type){ Type = type; }
	std::shared_ptr<Token_Type> GetVarType(){ return Type; }
	
	//Symbol
	std::shared_ptr<Symbol> GetSymbol() { return Sym; }
	void SetSymbol(std::shared_ptr<Symbol> ptr){
		Sym = ptr;
	}
	bool IsTemp() const{ return Temp; }
	void SetTemp(bool val) { Temp = val;}

	virtual std::string AsmValue(){ return value -> AsmValue(); }
	virtual std::string AsmDefaultValue(){ return Type -> AsmDefaultValue(); }
	
	//Variable modifiers
	void SetDereference(bool val = true){ Dereference = val;}
	bool GetDereference() const { return Dereference; }
	
	void SetIndexExpr(std::shared_ptr<Token_Expression>  IndexExpr) { this -> IndexExpr = IndexExpr; }
	std::shared_ptr<Token_Expression>  GetIndexExpr() { return IndexExpr; }
	
	void SetIndexFlat(std::shared_ptr<AsmOp> val) { IndexFlat = val; }
	std::shared_ptr<AsmOp> GetIndexFlat() { return IndexFlat; }
	
	//Comparing operators
	bool operator==(const Token_Var &obj) const;
	bool operator!=(const Token_Var &obj) const;
	bool operator==(const Token &obj) const;		//Overridden
	bool operator!=(const Token &obj) const;		//Overridden
	
	//If variable is safe, then it will not be assigned to R0-R4
	bool IsSafe() const { return safe; }
	void SetSafe(bool val=true) { safe = val; }
	
protected:
	std::shared_ptr<Token> value;
	std::shared_ptr<Token_Type> Type;
	std::shared_ptr<Symbol> Sym;	//Associated Symbol
	bool Temp;			//Whether a variable is temp
	
	//Variable modifiers
	bool Dereference;		//Only applicable for pointers. Set whether the variable has been dereferenced
	std::shared_ptr<Token_Expression> IndexExpr;		//Before flattening
	std::shared_ptr<AsmOp> IndexFlat;				//After flattening
	
	bool safe;		//Ask the memory manager not to use R1-R4
	
};
#endif