#include "expression.h"

Token_SimExpression::Token_SimExpression(std::shared_ptr<Token_Term> Term, Op_T Operator, std::shared_ptr<Token_SimExpression> SimExpression):
	Token("expression", T_Type::SimExpression, false), SimExpression(SimExpression), Operator(Operator), Term(Term)
{
	//if (Operator == None || Term == nullptr)
		CheckType();
}

Token_SimExpression::Token_SimExpression(const Token_SimExpression& obj): 
	Token(obj), SimExpression(obj.SimExpression), Operator(obj.Operator), Term(obj.Term),
	Type(obj.Type)
{

}

Token_SimExpression Token_SimExpression::operator=(const Token_SimExpression& obj)
{
	if (this != &obj){
		Token::operator=(obj);
		SimExpression = obj.SimExpression;
		Operator = obj.Operator;
		Term = obj.Term;
		Type = obj.Type;
	}
	
	return *this;
}

void Token_SimExpression::CheckType() throw(AsmCode){
	if (Operator == None || SimExpression == nullptr){
		Type = Term -> GetType();
	}
	else{
		//Check for Type compatibility
		
		std::shared_ptr<Token_Type> LHS, RHS;
		
		SimExpression -> CheckType();
		LHS = SimExpression -> GetType();
		
		Term -> CheckType();
		RHS = Term -> GetType();
		
		
		//For now simple equivalence - TODO
		if (LHS != RHS)
			throw TypeIncompatible;
		
		//Simple operator check - TODO
		if ((int) Operator < (int) Op_T::Add || (int) Operator > (int) Op_T::Xor)
			throw OperatorIncompatible;
		
		//TODO Type promotion?
		Type = LHS;
	}
}