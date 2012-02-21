#ifndef IntH
#define IntH
#include "../token.h"

class Token_Int: public Token{
public:
	//OCCF
	Token_Int(const char *StrValue, yytokentype type);
	~Token_Int() { }
	Token_Int(const Token_Int &obj);
	Token_Int operator=(const Token_Int &obj);
	
protected:
	int value;
	
};

#endif /* IntH */

