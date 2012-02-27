#ifndef IntH
#define IntH
#include "../token.h"

class Token_Int: public Token{
public:
	//OCCF
	Token_Int(const char *StrValue, int type, bool IsConstant=true);
	Token_Int(std::string StrValue, int type, bool IsConstant=true);
	Token_Int(int value, int type);
	~Token_Int() { }
	Token_Int(const Token_Int &obj);
	Token_Int operator=(const Token_Int &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	
protected:
	int value;	//We use Long so that up to MaxInt can be supported.
	
};

#endif /* IntH */

