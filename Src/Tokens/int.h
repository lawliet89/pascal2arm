#ifndef IntH
#define IntH
#include "../token.h"

class Token_Int: public Token{
public:
	//OCCF
	Token_Int(const char *StrValue, T_Type type, bool IsConstant=true);
	Token_Int(std::string StrValue, T_Type type, bool IsConstant=true);
	Token_Int(int value, T_Type type);
	~Token_Int() { }
	Token_Int(const Token_Int &obj);
	Token_Int operator=(const Token_Int &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	
	int GetInt() const { return value; }
	
	//Assembly Stuff
	virtual std::string AsmDefaultValue() {return "0"; }
	virtual std::string AsmValue();
	virtual int GetSize(){ return 4; }
protected:
	int value;	//We use Long so that up to MaxInt can be supported.
	
};

#endif /* IntH */

