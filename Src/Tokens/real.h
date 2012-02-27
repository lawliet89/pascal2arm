#ifndef RealH
#define RealH
#include "../token.h"

class Token_Real: public Token{
public:
	//OCCF
	Token_Real(char const *StrValue, int type, bool IsConstant=true);
	Token_Real(std::string StrValue, int type, bool IsConstant=true);
	Token_Real(double value, int type);
	~Token_Real() { }
	Token_Real(const Token_Real &obj);
	Token_Real operator=(const Token_Real &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
protected:
	double value;
	
};

#endif /* RealH */

