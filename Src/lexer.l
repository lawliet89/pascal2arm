%{
#include <iostream>
#include <iomanip>
#include <cstring>
#include "utility.h"

extern Flags_T Flags;	

//Forward declaration
void LexerConsumeComments(const char *delimiter);
%}
%option yylineno
%option noyywrap
%option stack

/* Pascal is case-insensitive */
A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]

/* States */

%%


"{"	LexerConsumeComments("{");
"{*"	LexerConsumeComments("{*");
"/*"	LexerConsumeComments("\/*");
"//"	LexerConsumeComments("\//");

[\n]	{		/* New Line */
		//OUTPUT << std::endl << std::setw(4) << yylineno << "\t";
		ECHO;
	}

.	{
		OUTPUT << yytext;
	}
%%

//Lexer Functions
void LexerConsumeComments(const char *delimiter){
	static char endDelimiter[3];

	//Beginning of comment
	//OUTPUT << "[[COMMENT DELIM " << delimiter;
	
	//Determine end delimiter
	//A zero value indicates that both strings are equal.
	//http://www.cplusplus.com/reference/clibrary/cstring/strcmp/
	if (!strcmp(delimiter, "{"))
		strcpy(endDelimiter, "}");
	else if (!strcmp(delimiter, "{*"))
		strcpy(endDelimiter, "*}");
	else if (!strcmp(delimiter, "//"))
		strcpy(endDelimiter, "\n");
	else if (!strcmp(delimiter, "/*"))
		strcpy(endDelimiter, "*/");
	
	//OUTPUT << " END DELIM " << endDelimiter;
	
	int i = 0;
	char c;
	//Time to eat charas
	while(1){
		c = yyinput();
		
		//Unexpected EOF
		if (c == EOF){
			OUTPUT << "Syntax error: unexpected EOF in comment";
			return;
		}
		
		if (c == endDelimiter[i])
			i++;
		else
			i=0;
		
		if (i == strlen(endDelimiter)){
			//End of comment
			//OUTPUT << " COMPLETE]]";
			//if (c == '\n')
			//	unput('\n');
			return;
		}
		
		//Reached here? Still in comment
	}
}