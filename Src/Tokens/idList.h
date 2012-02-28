#ifndef IdListH
#define IdListH
#include "../token.h"
#include "../define.h"
#include <set>
#include <string>
#include <memory>

class Token_IDList: public Token{
public:
	//OCCF
	Token_IDList(char const *StrValue);
	Token_IDList(std::string StrValue);
	Token_IDList(std::shared_ptr<Token> token);
	
	~Token_IDList() { }
	Token_IDList(const Token_IDList &obj);
	Token_IDList operator=(const Token_IDList &obj);
	
	const void * GetValue() const { return (void *) &list; }
	const void * operator()() const { return (void *) &list; }
	
	const std::set <std::string> GetList() const { return list; }
	
	void AddID(std::string id) throw(int);
	void AddID(std::shared_ptr<Token> token) throw(int);
	
	
protected:
	std::set <std::string> list;		//TODO store as tokens so as to find previous declaration location?
	
};

#endif /* IdListH */

