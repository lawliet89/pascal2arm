#ifndef ExpressionH
#define ExpressionH
#include "../token.h"

class Token_Expression;
class Token_SimExpression;
class Token_Factor;
class Token_Term;
class Symbol;

#include "term.h"
#include <memory>
#include "type.h"

/** Simple Expression **/
class Token_SimExpression: public Token{
public:	
	Token_SimExpression(std::shared_ptr<Token_Term> Term, Op_T Operator=None, std::shared_ptr<Token_SimExpression> SimExpression = nullptr);
	Token_SimExpression(const Token_SimExpression &obj);
	~Token_SimExpression() { }
	Token_SimExpression operator=(const Token_SimExpression &obj);
	
	void SetTerm(std::shared_ptr<Token_Term> Term){ this -> Term = Term; }
	void SetOp(Op_T Operator){ this -> Operator = Operator; }
	void SetSimExpression(std::shared_ptr<Token_SimExpression> SimExpression){ this -> SimExpression = SimExpression; }
	void CheckType() throw(AsmCode);		//Calculate the type of the Term. Throws exceptions on LHS and RHS being incompatible
	void SetTempVar(std::shared_ptr<Symbol> sym){ TempVar = sym; }
	
	std::shared_ptr<Token_Term> GetTerm(){ return Term; }
	Op_T GetOp() const { return Operator; }
	std::shared_ptr<Token_SimExpression> GetSimExpression(){ return SimExpression; }
	std::shared_ptr<Token_Type> GetType(){ return Type; }
	bool IsSimple(){ return SimExpression==nullptr && Operator==None; }
	std::shared_ptr<Symbol> GetTempVar(){ return TempVar; }
	
protected:
	//Form: SimExpression OP Term where SimExpression can contain another SimExpression indefinitely
	//When reduced to Term, SimExpression will be set to nullptr
	std::shared_ptr<Token_SimExpression> SimExpression;	
	Op_T Operator;
	std::shared_ptr<Token_Term> Term;
	std::shared_ptr<Token_Type> Type;
	std::shared_ptr<Symbol> TempVar;

};

/** Expression **/
class Token_Expression: public Token{
public:	
	Token_Expression(std::shared_ptr<Token_SimExpression> SimExpression, Op_T Operator=None, std::shared_ptr<Token_Expression> Expression = nullptr);
	Token_Expression(const Token_Expression &obj);
	~Token_Expression() { }
	Token_Expression operator=(const Token_Expression &obj);
	
	void SetExpression(std::shared_ptr<Token_Expression> Expression){ this -> Expression = Expression; }
	void SetOp(Op_T Operator){ this -> Operator = Operator; }
	void SetSimExpression(std::shared_ptr<Token_SimExpression> SimExpression){ this -> SimExpression = SimExpression; }
	void CheckType() throw(AsmCode);		//Calculate the type of the Expression. Throws exceptions on LHS and RHS being incompatible
	void SetTempVar(std::shared_ptr<Symbol> sym){ TempVar = sym; }
	
	std::shared_ptr<Token_Expression> GetExpression(){ return Expression; }
	Op_T GetOp() const { return Operator; }
	std::shared_ptr<Token_SimExpression> GetSimExpression(){ return SimExpression; }
	std::shared_ptr<Token_Type> GetType(){ return Type; }
	int GetID() const { return ExpressionID; }
	std::string GetIDStr();
	std::shared_ptr<Symbol> GetTempVar(){ return TempVar; }
	bool IsSimple(){ return Expression==nullptr && Operator==None; }		//Check to see if expression is simple
	
	/** Flattening Related Methods **/
	//An expression is strictly simple iff, brackets not withstanding, it only contains one factor. This method will return that factor 
	std::shared_ptr<Token_Factor> GetSimple();		
	
	//static methods
	static void SetBoolType(std::shared_ptr<Token_Type>) throw(AsmCode);

protected:
	//Form: Expression OP SimExpression where Expression can contain another Expression indefinitely
	//When reduced to SimExpression, Expression will be set to nullptr
	std::shared_ptr<Token_SimExpression> SimExpression;	
	Op_T Operator;
	std::shared_ptr<Token_Expression> Expression;
	std::shared_ptr<Token_Type> Type;
	int ExpressionID;
	std::shared_ptr<Symbol> TempVar;
	
	static std::shared_ptr<Token_Type> BoolType;	//Stored type for bool.
	static int IDCount;
};


#endif