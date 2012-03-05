#ifndef RealH
#define RealH
#include "../token.h"

class Token_Real: public Token{
public:
	//OCCF
	Token_Real(char const *StrValue, T_Type type, bool IsConstant=true);
	Token_Real(std::string StrValue, T_Type type, bool IsConstant=true);
	Token_Real(double value, T_Type type);
	~Token_Real() { }
	Token_Real(const Token_Real &obj);
	Token_Real operator=(const Token_Real &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	
	virtual std::string AsmDefaultValue() {return "0"; }
	virtual std::string AsmValue(){ return "0"; }	//TODO
	virtual int GetSize(){ return 4; }
	
protected:
	double value;
	
};

#endif /* RealH */

