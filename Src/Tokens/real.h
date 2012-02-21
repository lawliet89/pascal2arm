#ifndef RealH
#define RealH
#include "../token.h"

class Token_Real: public Token{
public:
	//OCCF
	Token_Real(char const *StrValue, yytokentype type);
	~Token_Real() { }
	Token_Real(const Token_Real &obj);
	Token_Real operator=(const Token_Real &obj);
	
	double GetValue() const { return value; }
	double operator()() const { return value; }
	
protected:
	double value;
	
};

#endif /* RealH */

