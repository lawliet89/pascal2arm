#ifndef FormalParamH
#define FormalParamH

#include "../token.h"
#include <vector>

class Token_Var;

class Token_FormalParam: Token{
public:
	struct Param_T{
		std::shared_ptr<Token_Var> Variable;
		bool Required;
		std::shared_ptr<Token> DefaultValue;
		bool Reference;
		
		Param_T(bool Required=true, bool Reference=false): Required(Required), Reference(Reference){}
	};
	
	Token_FormalParam();
	Token_FormalParam(Param_T);
	~Token_FormalParam(){}
	Token_FormalParam(const Token_FormalParam &obj);
	Token_FormalParam operator=(const Token_FormalParam &obj);
	
	//Setter
	void AddParm(Param_T param){ Params.push_back(param); }
	
	std::vector<Param_T> GetParams(){ return Params; }
	
protected:
	std::vector<Param_T> Params;		//List of parameters
};

#endif