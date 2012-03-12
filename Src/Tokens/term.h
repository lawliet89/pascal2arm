#ifndef TermnH
#define TermnH
#include "../token.h"

class Token_Expression;
class Token_SimExpression;
class Token_Factor;
class Token_Term;
class Symbol;

#include "factor.h"
#include "type.h"
#include <memory>

class Token_Term: public Token{
	
public:	
	Token_Term(std::shared_ptr<Token_Factor> Factor, Op_T Operator=None, std::shared_ptr<Token_Term> Term = nullptr);
	Token_Term(const Token_Term &obj);
	~Token_Term() { }
	Token_Term operator=(const Token_Term &obj);
	
	void SetTerm(std::shared_ptr<Token_Term> Term){ this -> Term = Term; }
	void SetOp(Op_T Operator){ this -> Operator = Operator; }
	void SetFactor(std::shared_ptr<Token_Factor> Factor){ this -> Factor = Factor; }
	void CheckType() throw(AsmCode);		//Calculate the type of the Term. Throws exceptions on LHS and RHS being incompatible
	void SetTempVar(std::shared_ptr<Symbol> sym){ TempVar = sym; }
	
	std::shared_ptr<Token_Term> GetTerm(){ return Term; }
	Op_T GetOp() const { return Operator; }
	std::shared_ptr<Token_Factor> GetFactor(){ return Factor; }
	std::shared_ptr<Token_Type> GetType(){ return Type; }
	bool IsSimple(){ return Term==nullptr && Operator==None; }
	std::shared_ptr<Symbol> GetTempVar(){ return TempVar; }
	
	//Comparing operators
// 	bool operator==(const Token_Term &obj) const;
// 	bool operator!=(const Token_Term &obj) const;
// 	bool operator==(const Token &obj) const;		//Overridden
// 	bool operator!=(const Token &obj) const;		//Overridden
	
protected:
	//Form: Term OP Factor where Term can contain another term indefinitely
	//When reduced to Factor, Term will be set to nullptr
	std::shared_ptr<Token_Term> Term;	
	Op_T Operator;
	std::shared_ptr<Token_Factor> Factor;
	std::shared_ptr<Token_Type> Type;
	std::shared_ptr<Symbol> TempVar;
};

#endif