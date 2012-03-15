#ifndef ExprList
#define ExprList

#include "../token.h"
#include <vector>
class Token_Expression;

class Token_ExprList: public Token{
public:
	//OCCF
	Token_ExprList();
	Token_ExprList(std::shared_ptr<Token_Expression> expr);
	
	~Token_ExprList() { }
	Token_ExprList(const Token_ExprList &obj);
	Token_ExprList operator=(const Token_ExprList &obj);
	
	const void * GetValue() const { return (void *) &list; }
	const void * operator()() const { return (void *) &list; }
	
	std::vector<std::shared_ptr<Token_Expression> > GetList() { return list; }
	
	void AddExpression(std::shared_ptr<Token_Expression> token){ list.push_back(token); }
	
	
protected:
	std::vector<std::shared_ptr<Token_Expression> > list;		//TODO store as tokens so as to find previous declaration location?
	
};

#endif