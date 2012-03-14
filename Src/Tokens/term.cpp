#include "term.h"
#include "var.h"
Token_Term::Token_Term(std::shared_ptr<Token_Factor> Factor, Op_T Operator, std::shared_ptr<Token_Term> Term):
	Token("term", T_Type::Term, false), Factor(Factor), Operator(Operator), Term(Term)
{
	//if (Operator == None || Term == nullptr)
		CheckType();
}

Token_Term::Token_Term(const Token_Term& obj): 
	Token(obj), Factor(obj.Factor), Operator(obj.Operator), Term(obj.Term),
	Type(obj.Type), TempVar(obj.TempVar)
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
		TempVar = obj.TempVar;
	}
	
	return *this;
}

void Token_Term::CheckType() throw(AsmCode){
	if (Operator == None || Term == nullptr){
		Type = Factor -> GetType();
		if (Factor -> GetForm() == Token_Factor::VarRef && Factor -> GetTokenDerived<Token_Var>() -> GetDereference()){
			//Dereference
			Type.reset(new Token_Type(*Type));		//clone
			Type -> SetPointer(false);
		}
		//Arrays
		if (Factor -> GetForm() == Token_Factor::VarRef && Factor -> GetTokenDerived<Token_Var>() -> GetIndexExpr() != nullptr){
			Type.reset(new Token_Type(*Type));
			Type -> SetArray(false);
		}
	}
	else{
		//Check for Type compatibility
		std::shared_ptr<Token_Type> LHS, RHS;
		Term -> CheckType();
		LHS = Term -> GetType();
		RHS = Factor -> GetType();
		
		//Check for pointer
		if (Factor -> GetForm() == Token_Factor::VarRef && Factor -> GetTokenDerived<Token_Var>() -> GetDereference()){
			//Clone type for checking purposes and setting
			RHS.reset(new Token_Type(*RHS));
			RHS -> SetPointer(false);
		}
		//Arrays
		if (Factor -> GetForm() == Token_Factor::VarRef && Factor -> GetTokenDerived<Token_Var>() -> GetIndexExpr() != nullptr){
			RHS.reset(new Token_Type(*RHS));
			RHS -> SetArray(false);
		}
		
		//For now simple equivalence and must not have secondary types
		if (*LHS != *RHS)
			throw TypeIncompatible;
		
		//Simple operator check - TODO
		if ((int) Operator < (int) Op_T::Multiply)
			throw OperatorIncompatible;
		
		//TODO Type promotion?
		Type = RHS;
	}
}
/*
bool Token_Term::operator==(const Token_Term& obj) const
{	//Check for nullptrs first
	bool FactorNullHere = Factor == nullptr, FactorNullThere = obj.Factor == nullptr;
	if (FactorNullHere ^ FactorNullThere)
		return false;
	
	if (!FactorNullHere && *Factor != *(obj.Factor))
		return false;
	
	if (Operator != obj.Operator)
		return false;
	
	bool TermNullHere = Term == nullptr, TermNullThere = obj.Term == nullptr;
	if (TermNullHere ^ TermNullThere)
		return false;
	
	if (!TermNullHere && *Term != *(obj.Term))
		return false;
	
	return true;
}

bool Token_Term::operator!=(const Token_Term& obj) const
{
	return !operator==(obj);
}


bool Token_Term::operator==(const Token& obj) const
{
    return operator==(dynamic_cast<const Token_Term &>(obj));
}

bool Token_Term::operator!=(const Token& obj) const
{
	return operator!=(dynamic_cast<const Token_Term &>(obj));
}
*/