#ifndef ExpressionH
#define ExpressionH
#include "../token.h"

class Token_Expression;
class Token_SimExpression;
class Token_Factor;
class Token_Term;

#include "term.h"
#include <memory>
#include "type.h"

class Token_SimExpression: public Token{
public:	
	Token_SimExpression(std::shared_ptr<Token_Term> Term, Op_T Operator=None, std::shared_ptr<Token_SimExpression> SimExpression = nullptr);
	Token_SimExpression(const Token_SimExpression &obj);
	~Token_SimExpression() { }
	Token_SimExpression operator=(const Token_SimExpression &obj);
	
	void SetTerm(std::shared_ptr<Token_Term> Term){ this -> Term = Term; }
	void SetOp(Op_T Operator){ this -> Operator = Operator; }
	void SetExpression(std::shared_ptr<Token_SimExpression> SimExpression){ this -> SimExpression = SimExpression; }
	void CheckType() throw(AsmCode);		//Calculate the type of the Term. Throws exceptions on LHS and RHS being incompatible
	
	std::shared_ptr<Token_Term> GetTerm(){ return Term; }
	Op_T GetOp() const { return Operator; }
	std::shared_ptr<Token_SimExpression> GetExpression(){ return SimExpression; }
	std::shared_ptr<Token_Type> GetType(){ return Type; }
	
protected:
	//Form: SimExpression OP Term where SimExpression can contain another SimExpression indefinitely
	//When reduced to Term, SimExpression will be set to nullptr
	std::shared_ptr<Token_SimExpression> SimExpression;	
	Op_T Operator;
	std::shared_ptr<Token_Term> Term;
	std::shared_ptr<Token_Type> Type;
};

#endif