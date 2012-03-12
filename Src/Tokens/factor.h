#ifndef FactornH
#define FactornH
#include "../token.h"

class Token_Expression;
class Token_SimExpression;
class Token_Factor;
class Token_Term;
class Token_Func;

#include "expression.h"
#include "type.h"

class Token_Factor: public Token{
public:
	enum Form_T{		//Form of the factor
		Expression,
		VarRef,
		FuncCall,
		//UnsignedConstant,
		//NegateFactor,			//Set negate flag to true	
		//SignedConstant,
		Constant,			//Signed and unsigned Collapsed into constant
		SetConstructors
	};
	
	//OCCF
    Token_Factor(Form_T Form, std::shared_ptr<Token> value, std::shared_ptr<Token_Type> Type=nullptr);
    Token_Factor(const Token_Factor& obj);
	~Token_Factor() {}
	Token_Factor operator=(const Token_Factor &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	std::shared_ptr<Token> GetValueToken(){ return value; }
	template <typename T> std::shared_ptr<T> GetTokenDerived(){
		return std::static_pointer_cast<T>( value );
	}
	
	void SetNegate(bool val=true){ Negate = val; }
	void SetFuncToken(std::shared_ptr<Token_Func> tok) { FuncToken = tok; }
	
	Form_T GetForm() const{ return Form; }
	bool IsNegate() const { return Negate; }
	std::shared_ptr<Token> GetToken(){ return value; }
	std::shared_ptr<Token_Type> GetType();
	bool IsSimple(){ return Form != Expression; }  //TODO for more complicated stuff
	std::shared_ptr<Token_Func> GetFuncToken() { return FuncToken; }
	
	//Comparing operators
// 	bool operator==(const Token_Factor &obj) const;
// 	bool operator!=(const Token_Factor &obj) const;
// 	bool operator==(const Token &obj) const;		//Overridden
// 	bool operator!=(const Token &obj) const;		//Overridden
protected:
	Form_T Form;
	std::shared_ptr<Token_Type> Type;
	
	bool Negate;		//Set if factor is to be negated
	std::shared_ptr<Token> value;		//Value of the token
	
	std::shared_ptr<Token_Func> FuncToken;		//token of function
};

#endif