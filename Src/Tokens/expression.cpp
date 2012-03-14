#include "expression.h"
#include "../utility.h"

std::shared_ptr<Token_Type> Token_Expression::BoolType;
int Token_Expression::IDCount = 0;

/** SimExpression **/
Token_SimExpression::Token_SimExpression(std::shared_ptr<Token_Term> Term, Op_T Operator, std::shared_ptr<Token_SimExpression> SimExpression):
	Token("simexpression", T_Type::SimExpression, false), SimExpression(SimExpression), Operator(Operator), Term(Term)
{
	CheckType();
}

Token_SimExpression::Token_SimExpression(const Token_SimExpression& obj): 
	Token(obj), SimExpression(obj.SimExpression), Operator(obj.Operator), Term(obj.Term),
	Type(obj.Type), TempVar(obj.TempVar)
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
		TempVar = obj.TempVar;
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
		if (*LHS != *RHS)
			throw TypeIncompatible;
		if (LHS -> IsPointer() || RHS -> IsPointer() || LHS -> IsArray() || RHS -> IsArray())
			throw TypeIncompatible;
		
		//Simple operator check - TODO
		if ((int) Operator < (int) Op_T::Add || (int) Operator > (int) Op_T::Xor)
			throw OperatorIncompatible;
		
		//TODO Type promotion?
		Type = LHS;
	}
}
/*
bool Token_SimExpression::operator==(const Token_SimExpression& obj) const
{	//Check for nullptrs first
	bool SimExpressionNullHere = SimExpression == nullptr, SimExpressionNullThere = obj.SimExpression == nullptr;
	if (SimExpressionNullHere ^ SimExpressionNullThere)
		return false;
	
	if (!SimExpressionNullHere && *SimExpression != *(obj.SimExpression))
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

bool Token_SimExpression::operator!=(const Token_SimExpression& obj) const
{
	return !operator==(obj);
}


bool Token_SimExpression::operator==(const Token& obj) const
{
    return operator==(dynamic_cast<const Token_SimExpression &>(obj));
}

bool Token_SimExpression::operator!=(const Token& obj) const
{
	return operator!=(dynamic_cast<const Token_SimExpression &>(obj));
}

*/
/** Expression **/
Token_Expression::Token_Expression(std::shared_ptr<Token_SimExpression> SimExpression, Op_T Operator, std::shared_ptr<Token_Expression> Expression):
	Token("expression", T_Type::Expression, false), SimExpression(SimExpression), Operator(Operator), Expression(Expression), TempVar(nullptr)
{
	//if (Operator == None || Term == nullptr)
		CheckType();
		if (Expression == nullptr){
			ExpressionID = IDCount;
			IDCount++;
		}
		else{
			ExpressionID = Expression->GetID();
		}
}

Token_Expression::Token_Expression(const Token_Expression& obj): 
	Token(obj), SimExpression(obj.SimExpression), Operator(obj.Operator), Expression(obj.Expression),
	Type(obj.Type), ExpressionID(obj.ExpressionID), TempVar(obj.TempVar)
{

}

Token_Expression Token_Expression::operator=(const Token_Expression& obj)
{
	if (this != &obj){
		Token::operator=(obj);
		SimExpression = obj.SimExpression;
		Operator = obj.Operator;
		Expression = obj.Expression;
		Type = obj.Type;
		ExpressionID = obj.ExpressionID;
		TempVar = obj.TempVar;
	}
	
	return *this;
}

void Token_Expression::CheckType() throw(AsmCode){
	if (BoolType == nullptr)
		throw BoolTypeUndefined;
	if (Operator == None || Expression == nullptr){
		Type = SimExpression -> GetType();
	}
	else{
		//Check for Type compatibility
		
		std::shared_ptr<Token_Type> LHS, RHS;
		
		Expression -> CheckType();
		LHS = Expression -> GetType();
		
		SimExpression -> CheckType();
		RHS = SimExpression -> GetType();
		
		
		//For now simple equivalence - TODO
		if (*LHS != *RHS)
			throw TypeIncompatible;
		if (LHS -> IsPointer() || RHS -> IsPointer() || LHS -> IsArray() || RHS -> IsArray())
			throw TypeIncompatible;		
		//Simple operator check - TODO
		//if ((int) Operator > (int) Op_T::In)
		//	throw OperatorIncompatible;
		
		//With relational operators, this has become a boolean type 
		Type = BoolType;
	}
}

void Token_Expression::SetBoolType(std::shared_ptr<Token_Type> tok) throw(AsmCode){
	if (tok -> GetPrimary() != Token_Type::Boolean)
		throw BoolTypeIncorrect;
	
	BoolType = tok;
}

std::string Token_Expression::GetIDStr(){
	return ToString<int>(ExpressionID) + "_temp";
}

/** Flattening **/
std::shared_ptr<Token_Factor> Token_Expression::GetSimple(){
	std::shared_ptr<Token_Factor> result(nullptr);
	
	//Check expr
	if (!IsSimple())
		return result;
	
	//Check simple expression
	if (!SimExpression -> IsSimple())
		return result;
	
	//Check for term
	std::shared_ptr<Token_Term> Term = SimExpression -> GetTerm();
	if (!Term -> IsSimple())
		return result;
	
	//Factor
	std::shared_ptr<Token_Factor> Factor = Term -> GetFactor();
	
	//More complicated
	if (Factor -> IsSimple()){
		result = Factor;
		return result;
	}
	else{
		//Check that form is type
		if (Factor -> GetForm() == Token_Factor::Expression){
			return std::static_pointer_cast<Token_Expression>(Factor -> GetValueToken()) -> GetSimple();
		}
	}
	
	return result;
}
/*
bool Token_Expression::operator==(const Token_Expression& obj) const
{	//Check for nullptrs first
	bool SimExpressionNullHere = SimExpression == nullptr, SimExpressionNullThere = obj.SimExpression == nullptr;
	if (SimExpressionNullHere ^ SimExpressionNullThere)
		return false;
	
	if (!SimExpressionNullHere && *SimExpression != *(obj.SimExpression))
		return false;
	
	if (Operator != obj.Operator)
		return false;
	
	bool ExprNullHere = Expression == nullptr, ExprNullThere = obj.Expression == nullptr;
	if (ExprNullHere ^ ExprNullThere)
		return false;
	
	if (!ExprNullHere && *Expression != *(obj.Expression))
		return false;
	
	return true;
}

bool Token_Expression::operator!=(const Token_Expression& obj) const
{
	return !operator==(obj);
}


bool Token_Expression::operator==(const Token& obj) const
{
    return operator==(dynamic_cast<const Token_Expression &>(obj));
}

bool Token_Expression::operator!=(const Token& obj) const
{
	return operator!=(dynamic_cast<const Token_Expression &>(obj));
}*/