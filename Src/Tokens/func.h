#ifndef TokenFunc
#define TokenFunc
#include <memory>
#include <string>
#include "../token.h"

//Forward declaration
class Symbols;

//Token for function/procedures
class Token_Func: public Token{
public:
	enum Type_T{
		Function,
		Procedure
	};
	
	//OCCF
	Token_Func(std::string ID, Type_T type=Function);
	Token_Func(const Token_Func &obj);
	~Token_Func() { }
	Token_Func operator=(const Token_Func &obj);
	
	Type_T GetType() const{ return Type; }
	std::string GetID() const{ return GetStrValue(); }
protected:
	Type_T Type;
	std::shared_ptr<Symbol> sym;		//Associated symbol
};


#endif