#ifndef TokenFunc
#define TokenFunc
#include <memory>
#include <string>
#include "../token.h"

//Forward declaration
class Symbols;
class AsmBlock;
class Token_FormalParam;
class Token_Type;

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
	
	void SetReturnType(std::shared_ptr<Token_Type> ReturnType){ this -> ReturnType = ReturnType; }
	void SetBlock(std::shared_ptr<AsmBlock> block) { this -> block = block; }
	void SetParam(std::shared_ptr<Token_FormalParam> Params) { this -> Params = Params; }
	
	//Getters
	Type_T GetType() const{ return Type; }
	std::string GetID() const{ return GetStrValue(); }
	std::shared_ptr<Token_Type> GetReturnType(){ return ReturnType; }
	std::shared_ptr<AsmBlock> GetBlock(){ return block; }
	std::shared_ptr<Token_FormalParam> GetParams() { return Params; }
	
protected:
	Type_T Type;
	std::shared_ptr<Symbol> sym;		//Associated symbol

	std::shared_ptr<Token_Type> ReturnType;	//Return type of function, if any
	std::shared_ptr<AsmBlock> block;
	std::shared_ptr<Token_FormalParam> Params;
	
};


#endif