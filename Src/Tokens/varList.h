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
public:
	//OCCF
	Token_VarList(std::shared_ptr<Token_Var> &ptr);
	Token_VarList();
	Token_VarList(const Token_VarList &obj);
	
	~Token_VarList() { }
	Token_VarList operator=(const Token_VarList &obj);
	
	const void * GetValue() const { return (void *) &list; }
	const void * operator()() const { return (void *) &list; }
	
	const std::vector<std::shared_ptr<Token_Var> > GetList() const { return list; }
	
	//Add elements
	void AddVariable(std::shared_ptr<Token_Var> &ptr);
	
protected:
	std::vector<std::shared_ptr<Token_Var> > list;
};

#endif