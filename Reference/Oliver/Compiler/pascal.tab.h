
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     AND = 258,
     ARRAY = 259,
     ASSIGNMENT = 260,
     CASE = 261,
     COMMENT = 262,
     COLON = 263,
     COMMA = 264,
     CONST = 265,
     DIGIT = 266,
     DIV = 267,
     DO = 268,
     DOT = 269,
     DOTDOT = 270,
     DOWNTO = 271,
     ELSE = 272,
     END = 273,
     EQUAL = 274,
     EOLINE = 275,
     EXTERNAL = 276,
     FOR = 277,
     FORWARD = 278,
     FUNCTION = 279,
     GE = 280,
     GOTO = 281,
     GT = 282,
     IDENTIFIER = 283,
     IF = 284,
     IN = 285,
     INTEGER = 286,
     LABEL = 287,
     LBRAC = 288,
     LE = 289,
     LPAREN = 290,
     LT = 291,
     MINUS = 292,
     MOD = 293,
     NIL = 294,
     NOT = 295,
     NOTEQUAL = 296,
     OF = 297,
     OR = 298,
     OTHERWISE = 299,
     PACKED = 300,
     PBEGIN = 301,
     PFILE = 302,
     PLUS = 303,
     PROCEDURE = 304,
     PROGRAM = 305,
     RBRAC = 306,
     RECORD = 307,
     REPEAT = 308,
     RPAREN = 309,
     SEMICOLON = 310,
     SET = 311,
     SLASH = 312,
     STAR = 313,
     STARSTAR = 314,
     THEN = 315,
     TO = 316,
     TYPE = 317,
     UNTIL = 318,
     UPARROW = 319,
     VAR = 320,
     WHILE = 321,
     WITH = 322,
     WRITE = 323
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


