#ifndef TokenVar
#define TokenVar
#include <memory>
#include "../token.h"

class Token_Var: public Token{
public:
	//OCCF
	Token_Var(std::shared_ptr<Token> value, int type);
	Token_Var(const Token_Var &obj);
	
	~Token_Var() { }
	Token_Var operator=(const Token_Var &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	
protected:
	std::shared_ptr<Token> value;
};
#endif