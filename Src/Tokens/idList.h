#ifndef IdListH
#define IdListH
#include "../token.h"
#include "../define.h"
#include <map>
#include <string>
#include <memory>

class Token_IDList: public Token{
public:
	//OCCF
	Token_IDList(std::shared_ptr<Token> token);
	
	~Token_IDList() { }
	Token_IDList(const Token_IDList &obj);
	Token_IDList operator=(const Token_IDList &obj);
	
	const void * GetValue() const { return (void *) &list; }
	const void * operator()() const { return (void *) &list; }
	
	std::map <std::string, std::shared_ptr<Token> > GetList() { return list; }
	
	void AddID(std::shared_ptr<Token> token) throw(AsmCode);
	
	
protected:
	std::map <std::string, std::shared_ptr<Token> > list;		//TODO store as tokens so as to find previous declaration location?
	
};

#endif /* IdListH */

