#include "token.h"
#include <sstream>

extern unsigned LexerCharCount, LexerLineCount;	//lexer.l

//Constructors
Token::Token(const char *StrValue, T_Type TokenType, bool IsConstant): 
	StrValue(StrValue), TokenType(TokenType), IsConstant(IsConstant),
	line(LexerLineCount), column(LexerCharCount)
{}
Token::Token(std::string StrValue, T_Type TokenType, bool IsConstant): 
	StrValue(StrValue), TokenType(TokenType), IsConstant(IsConstant),
	line(LexerLineCount), column(LexerCharCount)
	{}
//Copy constructor
Token::Token (const Token &obj):
	StrValue(obj.StrValue), TokenType(obj.TokenType), IsConstant(obj.IsConstant),
	line(obj.line), column(obj.column)
{}

//Assignment operator
Token Token::operator=(const Token &obj){
	//Handles self assignment
	if (&obj != this){
		StrValue = obj.StrValue;
		TokenType = obj.TokenType;
		IsConstant = obj.IsConstant;
		line = obj.line;
		column = obj.column;
	}
	
	return *this;
}

std::string Token::AsmDefaultValue(){
	int i = GetSize();
	int n = i/4;
	if (i % 4 != 0)
		n++;
	std::stringstream result;
	result << "\"";
	
	for (int j = 0; j < n; j++){
		result << '\0' << '\0' << '\0' << '\0';
	}
	result << "\"";
	return result.str();
	
}

bool Token::operator==(const Token &obj) const{
	return StrValue == obj.StrValue;
}
bool Token::operator!=(const Token &obj) const{
	return StrValue != obj.StrValue;
}
bool Token::operator==(const std::shared_ptr<Token> &obj) const{
	return *this == *obj;
}
bool Token::operator!=(const std::shared_ptr<Token> &obj) const{
	return *this != *obj;
}