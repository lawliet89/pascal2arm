%{
#include <iostream>
#include <string>
#include <cstring>
#include "pascal.tab.h"

using namespace std;

// These are global variables
string varStr = "", opStr = "", condStr = "", boolStr = "";
int position, linenum = 1;
%}
%option noyywrap

%%

and   				{position += yyleng; return AND ;}
array				{position += yyleng; return ARRAY ;}
case				{position += yyleng; return CASE ;}
const				{position += yyleng; return CONST ;}
div					{position += yyleng; return DIV ;}
do					{position += yyleng; return DO ;}
downto				{position += yyleng; return DOWNTO ;}
else            	{position += yyleng; return ELSE ;}
end   				{position += yyleng; return END ;}
extern | external 	{position += yyleng; return EXTERNAL ;}
for   				{position += yyleng; return FOR ;}
forward  			{position += yyleng; return FORWARD ;}
function 			{position += yyleng; return FUNCTION ;}
goto   				{position += yyleng; return GOTO ;}
if    				{position += yyleng; return IF ;}
in    				{position += yyleng; return IN ;}
label   			{position += yyleng; return LABEL ;}
mod   				{position += yyleng; return MOD ;}
nil   				{position += yyleng; return NIL ;}
not   				{position += yyleng; return NOT ;}
of    				{position += yyleng; return OF ;}
or    				{position += yyleng; return OR ;}
otherwise 			{position += yyleng; return OTHERWISE ;}
packed  			{position += yyleng; return PACKED ;}
begin   			{position += yyleng; return PBEGIN ;}
file   				{position += yyleng; return PFILE ;}
procedure 			{position += yyleng; return PROCEDURE ;}
program  			{position += yyleng; return PROGRAM ;}
record  			{position += yyleng; return RECORD ;}
repeat  			{position += yyleng; return REPEAT ;}
set   				{position += yyleng; return SET ;}
then   				{position += yyleng; return THEN ;}
to    				{position += yyleng; return TO ;}
type   				{position += yyleng; return TYPE ;}
until   			{position += yyleng; return UNTIL ;}
var   				{position += yyleng; return VAR ;}
while   			{position += yyleng; return WHILE ;}
with   				{position += yyleng; return WITH ;}
write				{position += yyleng; return WRITE ;}
integer				{position += yyleng; return INTEGER ;}

":="    			{position += yyleng; return ASSIGNMENT ;}
":"    				{position += yyleng; return COLON ;}
","    				{position += yyleng; return COMMA ;}
"."    				{position += yyleng; return DOT ;}
".."    			{position += yyleng; return DOTDOT ;}
"="    				{position += yyleng; return EQUAL ;}
">="    			{position += yyleng; return GE ;}
">"    				{position += yyleng; return GT ;}
"["    				{position += yyleng; return LBRAC ;}
"<="    			{position += yyleng; return LE ;}
"("    				{position += yyleng; return LPAREN ;}
"<"    				{position += yyleng; return LT ;}
"-"    				{position += yyleng; return MINUS ;}
"<>"    			{position += yyleng; return NOTEQUAL ;}
"+"    				{position += yyleng; return PLUS ;}
"]"    				{position += yyleng; return RBRAC ;}
")"    				{position += yyleng; return RPAREN ;}
";"		  			{position += yyleng; return SEMICOLON ;}
"/"    				{position += yyleng; return SLASH ;}
"*"    				{position += yyleng; return STAR ;}
"**"    			{position += yyleng; return STARSTAR ;}
"->" | "^"    		{position += yyleng; return UPARROW ;}

"/*""/"*([^*/]|[^*]"/"|"*"[^/])*"*"*"*/"	{position += yyleng; return COMMENT;} 

[\n]				{position = 1; linenum++;}
[[:BLANK:]]	 	    {position += yyleng;} 	
"{"[^}]*"}" 	
"//"[^\n]*(\n) 	

	

[0-9]+    									{position += yyleng; varStr = yytext; return DIGIT;}
[a-zA-Z][a-zA-Z0-9]*   						{position += yyleng; varStr = yytext; return IDENTIFIER;}


%%


