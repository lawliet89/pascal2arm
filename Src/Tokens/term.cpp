#include "term.h"

Token_Term::Token_Term(std::shared_ptr<Token_Factor> Factor, Op_T Operator, std::shared_ptr<Token_Term> Term):
	Token("term", T_Type::Term, false), Factor(Factor), Operator(Operator), Term(Term)
{
	//if (Operator == None || Term == nullptr)
		CheckType();
}

Token_Term::Token_Term(const Token_Term& obj): 
	Token(obj), Factor(obj.Factor), Operator(obj.Operator), Term(obj.Term),
	Type(obj.Type)
{

}

Token_Term Token_Term::operator=(const Token_Term& obj)
{
	if (this != &obj){
		Token::operator=(obj);
		Factor = obj.Factor;
		Operator = obj.Operator;
		Term = obj.Term;
		Type = obj.Type;
	}
	
	return *this;
}

void Token_Term::CheckType() throw(AsmCode){
	if (Operator == None || Term == nullptr){
		Type = Factor -> GetType();
	}
	else{
		//Check for Type compatibility
		std::shared_ptr<Token_Type> LHS, RHS;
		Term -> CheckType();
		LHS = Term -> GetType();
		RHS = Factor -> GetType();
		
		//For now simple equivalence - TODO
		if (LHS != RHS)
			throw TypeIncompatible;
		
		//Simple operator check - TODO
		if ((int) Operator < (int) Op_T::Multiply)
			throw OperatorIncompatible;
	}
}