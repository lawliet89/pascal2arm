#ifndef CharH
#define CharH
#include "../token.h"

class Token_Char: public Token{
public:
	//OCCF
	Token_Char(char value, bool IsConstant=true);

	~Token_Char() { }
	Token_Char(const Token_Char &obj);
	Token_Char operator=(const Token_Char &obj);
	
	virtual const void * GetValue() const { return (void *) &value; }
	virtual const void * operator()() const { return (void *) &value; }
	
	char GetChar() const { return value; }
	
	//Assembly Stuff
	virtual std::string AsmDefaultValue() {return "0"; }
	virtual std::string AsmValue();
	virtual int GetSize(){ return 4; }
	
	//Comparing operators
	bool operator==(const Token_Char &obj) const;
	bool operator!=(const Token_Char &obj) const;
	bool operator==(const Token &obj) const;		//Overridden
	bool operator!=(const Token &obj) const;		//Overridden
protected:
	char value;
	
};

#endif /* CharH */

