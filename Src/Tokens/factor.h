#ifndef FactornH
#define FactornH
#include "../token.h"

class Token_Expression;
class Token_SimExpression;
class Token_Factor;
class Token_Term;

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
		//Identifier			//TODO Remove this eventually
	};
	
	//OCCF
    Token_Factor(Form_T Form, std::shared_ptr<Token> value, std::shared_ptr<Token_Type> Type=nullptr);
    Token_Factor(const Token_Factor& obj);
	~Token_Factor() {}
	Token_Factor operator=(const Token_Factor &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	
	void SetNegate(bool val=true){ Negate = val; }
	
	Form_T GetForm() const{ return Form; }
	bool IsNegate() const { return Negate; }
	std::shared_ptr<Token> GetToken(){ return value; }
	std::shared_ptr<Token_Type> GetType(){ return Type; }
protected:
	Form_T Form;
	std::shared_ptr<Token_Type> Type;
	
	bool Negate;		//Set if factor is to be negated
	std::shared_ptr<Token> value;		//Value of the token
};

#endif