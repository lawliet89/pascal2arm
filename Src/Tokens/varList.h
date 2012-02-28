#ifndef VarListH
#define VarListH

#include <vector>
#include <memory>
#include "../token.h"
#include "../define.h"
#include "var.h"

//Forward declaration
class Token_Var;

class Token_VarList: public Token{
	//OCCF
	Token_VarList(std::shared_ptr<Token_Var>);
	Token_VarList();
	Token_VarList(const Token_VarList &obj);
	
	~Token_VarList() { }
	Token_VarList operator=(const Token_VarList &obj);
	
	
};

#endif