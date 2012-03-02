#ifndef SymbolsH
#define SymbolsH

#include <string>
#include <memory>
#include "token.h"
#include "utility.h"

/**
*	Symbols function and classes
* 		- Note: IDs are always stored in lowercase because Pascal is not case sensitive
*
**/
//Forward declaration
class AsmFile;	//Declared in asm.h - forward delcared so that we can define friend relations
class AsmBlock;
class Symbol; //Symbols definition

/*
 * Symbols
 * 
 * */
class Symbol{
public:
	friend class AsmFile;
	
	/** Enum Definition **/
	//Type of symbols
	enum Type_T{
		Constant,
		Variable,
		Function,
		Procedure, 
		Typename
	};
	
	/** Public Methods **/
	~Symbol();
	Symbol(const Symbol &obj);
	Symbol operator=(const Symbol &obj);	

	std::shared_ptr<Token> GetValue(){ return Value; }
	Token *GetToken() { return Value.get(); }

	template <typename T> T GetTokenDerived(){
		return dynamic_cast<T> (Value.get());
	}
	/*
	template <typename T> T GetValue(){
		return DereferenceVoidPtr<T>(Value -> GetValue());
		
	}
	*/
	
	void SetToken(std::shared_ptr<Token> token){
		Value = token;
	}
	
	void SetBlock(std::shared_ptr<AsmBlock> block){
		Block = block;
	}
	
	Type_T GetType() const{ return Type; }
	std::shared_ptr<AsmBlock> GetBlock(){ return Block; }
	std::string GetID() const {return ID; }
	bool IsReserved() const { return Reserved; }
	
protected:	//Consturctor is protected so that no one but AsmFile can instantiate
	/** Methods **/
	//OCCF
	Symbol(Type_T type, std::string id, std::shared_ptr<Token> value=nullptr, std::shared_ptr<AsmBlock> block=nullptr);
	void SetReserved(){ Reserved = true; }	//Only AsmFile can define reserved

	
	/** Data Members **/
	std::string ID;		//Identifier
	//bool IDUser;		//Whether ID is user defined
	Type_T Type;
	std::shared_ptr<Token> Value;	//Token storing the value
	bool Reserved;		//Reserved symbol
	std::shared_ptr<AsmBlock> Block;	//Block in which symbol was defined
};

#endif