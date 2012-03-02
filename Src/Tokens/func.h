#ifndef TokenFunc
#define TokenFunc
#include <memory>
#include <string>
#include <vector>
#include "../token.h"
#include "var.h"
#include "type.h"

//Forward declaration
class Symbols;

//Token for function/procedures
class Token_Func: public Token{
public:
	enum Type_T{
		Function,
		Procedure
	};
	
	struct Param_T{
		std::shared_ptr<Token_Var> Variable;
		bool Required;
		std::shared_ptr<Token> DefaultValue;
	};
	
	//OCCF
	Token_Func(std::string ID, Type_T type=Function);
	Token_Func(const Token_Func &obj);
	~Token_Func() { }
	Token_Func operator=(const Token_Func &obj);
	
	//Setter
	void AddParm(Param_T param){ Params.push_back(param); }
	void SetReturnType(std::shared_ptr<Token_Type> ReturnType){ this -> ReturnType = ReturnType; }
	
	//Getters
	Type_T GetType() const{ return Type; }
	std::string GetID() const{ return GetStrValue(); }
	std::vector<Param_T> GetParams(){ return Params; }
	std::shared_ptr<Token_Type> GetReturnType(){ return ReturnType; }
	
	
protected:
	Type_T Type;
	std::shared_ptr<Symbol> sym;		//Associated symbol
	
	std::vector<Param_T> Params;		//List of parameters
	std::shared_ptr<Token_Type> ReturnType;	//Return type of function, if any
	
};


#endif