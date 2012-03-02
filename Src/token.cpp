#include "token.h"
extern unsigned LexerCharCount, LexerLineCount;	//lexer.l

//Constructors
Token::Token(const char *StrValue, T_Type type, bool IsConstant): 
	StrValue(StrValue), type(type), IsConstant(IsConstant),
	line(LexerLineCount), column(LexerCharCount)
{}
Token::Token(std::string StrValue, T_Type type, bool IsConstant): 
	StrValue(StrValue), type(type), IsConstant(IsConstant),
	line(LexerLineCount), column(LexerCharCount)
	{}
//Copy constructor
Token::Token (const Token &obj):
	StrValue(obj.StrValue), type(obj.type), IsConstant(obj.IsConstant),
	line(obj.line), column(obj.column)
{}

//Assignment operator
Token Token::operator=(const Token &obj){
	//Handles self assignment
	if (&obj != this){
		StrValue = obj.StrValue;
		type = obj.type;
		IsConstant = obj.IsConstant;
		line = obj.line;
		column = obj.column;
	}
	
	return *this;
}