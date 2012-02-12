%{
#include <iostream>
#include "utility.h"

extern Flags_T Flags;	

%}

%option noyywrap
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

%%

\{.*\}		|

\/\/[^\n]*\n	|
\/\*.*\*\/	{
			/* Comments */
			OUTPUT << "{Comments found}";
		};

.	{
		OUTPUT << yytext;
	}
%%