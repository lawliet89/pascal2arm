
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "pascal.y"
 
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

// YYLEX declarations
int yylex();
extern FILE* yyin;

// Symbol Table declaration
struct symbolTable 
{
	string id;
};

// Global objects
fstream output; 				//output file
extern int linenum; 			//debug - line of error
extern int position; 			//debug - col of error
extern string varStr; 			//string containg name of IDENTIFIERS

// Variables for Code Generation
bool is_function =  false;
int location = 5, func_location = 0, func_var_location = 1, dest = 0, op_loc = 0, for_dest = 0, for_loop = 0, while_loop = 0, loop = 0;
int func_dest = 0, param_dest = 0;
string instruct = "", for_instruct = "", if_flag = "", for_inc = "#1", for_op = "", while_flag = "";
string operand[3] = {""}, while_op[2] = {""};

//Symbol Tables
vector<struct symbolTable> var_table;			//table with global pascal variables
vector<struct symbolTable> func_var_table;		//table containing local variables in functions
vector<struct symbolTable> func_table;			//table with names of all the functions

// Function Prototypes
void yyerror(const char* msg);				//error reporting
void addTable(string temp_id);				//add a variable to table
void addFuncTable(string temp_id);			//add a function to table
void addFuncVarTable(string temp_id);		//add a function variable to table		
int searchLocation(string temp_id);			//search for the register location of an IDENTIFIER
int searchFunc (string temp_id);			//search for the function label
string intToStr (int temp_int);				//convert integer to string

void codeGen (string instruct, int dest, string operand1, string operand2);		//code generation



/* Line 189 of yacc.c  */
#line 125 "pascal.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


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


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 235 "pascal.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   224

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  42
/* YYNRULES -- Number of rules.  */
#define YYNRULES  82
/* YYNRULES -- Number of states.  */
#define YYNSTATES  179

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,    12,    16,    22,    29,    31,    35,
      37,    38,    39,    56,    57,    58,    76,    77,    79,    81,
      83,    87,    89,    92,    94,    97,    99,   102,   104,   107,
     109,   112,   116,   117,   118,   125,   126,   137,   141,   142,
     153,   156,   158,   160,   162,   164,   168,   175,   178,   184,
     191,   192,   194,   198,   202,   206,   210,   216,   220,   223,
     227,   231,   233,   235,   237,   239,   241,   243,   245,   247,
     249,   251,   253,   255,   257,   259,   261,   263,   265,   267,
     269,   271,   273
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      70,     0,    -1,    -1,    72,    73,    76,    46,    71,    83,
     110,    -1,    50,    28,    55,    -1,    65,    74,     8,    75,
      55,    -1,    73,    65,    74,     8,    75,    55,    -1,    28,
      -1,    74,     9,    28,    -1,    31,    -1,    -1,    -1,    81,
      28,    77,    35,    82,     8,    75,    54,     8,    75,    55,
      46,    78,    83,    18,    55,    -1,    -1,    -1,    76,    81,
      28,    79,    35,    82,     8,    75,    54,     8,    75,    55,
      46,    80,    83,    18,    55,    -1,    -1,    24,    -1,    49,
      -1,    28,    -1,    82,     9,    28,    -1,   101,    -1,    83,
     101,    -1,    96,    -1,    83,    96,    -1,    91,    -1,    83,
      91,    -1,    88,    -1,    83,    88,    -1,    84,    -1,    83,
      84,    -1,   102,    85,    55,    -1,    -1,    -1,    28,    86,
      35,    28,    87,    54,    -1,    -1,    66,    35,    90,    89,
      54,    13,    46,    83,    18,    55,    -1,   104,   109,   104,
      -1,    -1,    22,    93,    94,    95,    13,    92,    46,    83,
      18,    55,    -1,   102,   104,    -1,    61,    -1,    16,    -1,
      11,    -1,    28,    -1,    99,   101,    97,    -1,    99,    46,
      83,    18,    55,    97,    -1,    98,   101,    -1,    98,    46,
     101,    18,    55,    -1,    98,    46,    83,   101,    18,    55,
      -1,    -1,    17,    -1,    29,   100,    60,    -1,   104,   108,
     104,    -1,    35,   100,    54,    -1,   100,   107,   100,    -1,
      68,    35,    28,    54,    55,    -1,   102,   103,    55,    -1,
      28,     5,    -1,   103,   105,   104,    -1,   103,   106,   104,
      -1,   104,    -1,    11,    -1,    28,    -1,    48,    -1,    37,
      -1,    58,    -1,    57,    -1,     3,    -1,    43,    -1,    19,
      -1,    41,    -1,    25,    -1,    34,    -1,    27,    -1,    36,
      -1,    19,    -1,    41,    -1,    25,    -1,    34,    -1,    27,
      -1,    36,    -1,    18,    14,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    64,    64,    64,    67,    71,    72,    75,    76,    79,
      83,    83,    83,    84,    84,    84,    85,    88,    89,    92,
      93,    97,    98,    99,   100,   101,   102,   103,   104,   105,
     106,   110,   115,   115,   115,   119,   119,   124,   127,   127,
     133,   136,   137,   140,   141,   145,   146,   149,   150,   151,
     152,   155,   163,   166,   167,   168,   171,   172,   175,   179,
     180,   181,   184,   185,   189,   190,   193,   194,   197,   198,
     202,   203,   204,   205,   206,   207,   211,   212,   213,   214,
     215,   216,   219
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "AND", "ARRAY", "ASSIGNMENT", "CASE",
  "COMMENT", "COLON", "COMMA", "CONST", "DIGIT", "DIV", "DO", "DOT",
  "DOTDOT", "DOWNTO", "ELSE", "END", "EQUAL", "EOLINE", "EXTERNAL", "FOR",
  "FORWARD", "FUNCTION", "GE", "GOTO", "GT", "IDENTIFIER", "IF", "IN",
  "INTEGER", "LABEL", "LBRAC", "LE", "LPAREN", "LT", "MINUS", "MOD", "NIL",
  "NOT", "NOTEQUAL", "OF", "OR", "OTHERWISE", "PACKED", "PBEGIN", "PFILE",
  "PLUS", "PROCEDURE", "PROGRAM", "RBRAC", "RECORD", "REPEAT", "RPAREN",
  "SEMICOLON", "SET", "SLASH", "STAR", "STARSTAR", "THEN", "TO", "TYPE",
  "UNTIL", "UPARROW", "VAR", "WHILE", "WITH", "WRITE", "$accept",
  "program", "$@1", "program_header", "var_declaration", "variables",
  "var_type", "function", "$@2", "$@3", "$@4", "$@5", "func_type",
  "func_var", "line", "func_call", "func", "$@6", "$@7", "while_loop",
  "$@8", "while_conditions", "for_loop", "$@9", "for_ini", "for_to",
  "for_end", "if_loop", "else_loop", "else", "if_statement",
  "if_condition", "statement", "assignment", "expr", "factor", "addop",
  "mulop", "boolop", "if_op", "while_op", "end_prg", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    69,    71,    70,    72,    73,    73,    74,    74,    75,
      77,    78,    76,    79,    80,    76,    76,    81,    81,    82,
      82,    83,    83,    83,    83,    83,    83,    83,    83,    83,
      83,    84,    86,    87,    85,    89,    88,    90,    92,    91,
      93,    94,    94,    95,    95,    96,    96,    97,    97,    97,
      97,    98,    99,   100,   100,   100,   101,   101,   102,   103,
     103,   103,   104,   104,   105,   105,   106,   106,   107,   107,
     108,   108,   108,   108,   108,   108,   109,   109,   109,   109,
     109,   109,   110
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     7,     3,     5,     6,     1,     3,     1,
       0,     0,    16,     0,     0,    17,     0,     1,     1,     1,
       3,     1,     2,     1,     2,     1,     2,     1,     2,     1,
       2,     3,     0,     0,     6,     0,    10,     3,     0,    10,
       2,     1,     1,     1,     1,     3,     6,     2,     5,     6,
       0,     1,     3,     3,     3,     3,     5,     3,     2,     3,
       3,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     0,     1,     0,    16,     4,     7,
       0,    17,    18,     0,     0,     0,     0,     0,     0,     2,
       0,    10,     9,     0,     8,     0,     0,    13,     0,     5,
       0,     0,     0,     0,     0,     0,     0,    29,    27,    25,
      23,     0,    21,     0,     0,     0,     6,     0,     0,    58,
      62,    63,     0,     0,     0,     0,     0,     0,    30,    28,
      26,    24,    22,     3,     0,    50,     0,    63,     0,     0,
      61,     0,    19,     0,    42,    41,     0,    40,     0,    68,
      69,    52,     0,    70,    72,    74,    73,    75,    71,     0,
      35,     0,     0,    82,     0,    51,    45,     0,     0,    31,
      65,    64,    57,    67,    66,     0,     0,     0,     0,     0,
      43,    44,     0,    54,    55,    53,     0,    76,    78,    80,
      79,    81,    77,     0,     0,     0,     0,    47,     0,    59,
      60,     0,     0,    20,    38,     0,    37,    56,    50,     0,
      21,    33,     0,     0,     0,     0,    46,    22,     0,     0,
       0,     0,     0,     0,     0,    48,    34,     0,     0,     0,
       0,    49,     0,     0,     0,     0,     0,    11,    39,    36,
      14,     0,     0,     0,     0,     0,     0,    12,    15
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,    26,     3,     7,    10,    23,    14,    28,   171,
      44,   172,    15,    73,    36,    37,    68,    98,   149,    38,
     116,    90,    39,   144,    47,    76,   112,    40,    96,    97,
      41,    53,    42,    43,    69,    54,   105,   106,    82,    89,
     123,    63
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -58
static const yytype_int16 yypact[] =
{
     -29,   -12,    18,   -42,   -25,   -58,     8,   -10,   -58,   -58,
       4,   -58,   -58,     8,   124,    29,    40,    45,    23,   -58,
      50,   -58,   -58,    -9,   -58,    40,    25,   -58,    16,   -58,
      27,    61,    21,    57,    63,    65,     6,   -58,   -58,   -58,
     -58,    31,   -58,    -8,    69,    80,   -58,    -5,    59,   -58,
     -58,   -58,    57,     7,   144,    59,    93,   108,   -58,   -58,
     -58,   -58,   -58,   -58,    25,   118,    59,    99,    81,   117,
     -58,    80,   -58,   109,   -58,   -58,    73,   -58,    -2,   -58,
     -58,   -58,    57,   -58,   -58,   -58,   -58,   -58,   -58,    59,
     -58,   157,    95,   -58,    15,   -58,   -58,    48,   120,   -58,
     -58,   -58,   -58,   -58,   -58,    59,    59,   111,    40,   133,
     -58,   -58,   149,   -58,    22,   -58,   110,   -58,   -58,   -58,
     -58,   -58,   -58,    59,   113,   122,    25,   -58,   151,   -58,
     -58,    40,   127,   -58,   -58,   170,   -58,   -58,   118,    25,
     168,   -58,   134,   179,   143,   146,   -58,   172,   139,   142,
     191,    40,    25,    25,   145,   -58,   -58,    40,   147,    20,
      84,   -58,   148,   155,   150,   152,   162,   -58,   -58,   -58,
     -58,    25,    25,   129,   138,   154,   156,   -58,   -58
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -58,   -58,   -58,   -58,   -58,   197,     2,   -58,   -58,   -58,
     -58,   -58,   198,   153,   -57,   -34,   -58,   -58,   -58,   -32,
     -58,   -58,   -30,   -58,   -58,   -58,   -58,   -28,    75,   -58,
     -58,   -37,   -36,   -22,   -58,   -26,   -58,   -58,   -58,   -58,
     -58,   -58
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -33
static const yytype_int16 yytable[] =
{
      62,    79,    58,    50,    59,    65,    60,    94,    61,    48,
      79,    74,    16,    17,    11,    78,     4,    70,     5,    66,
      67,     1,    77,     6,    57,    79,    49,    30,    31,    91,
       8,    25,    17,   125,    32,    33,     9,    31,   164,    12,
      70,    80,    31,    32,    33,   114,    29,    31,    32,    33,
      80,    45,   113,    32,    33,    13,    75,    21,    62,    32,
      58,   127,    59,   115,    60,    80,    61,    81,    50,   139,
      50,    22,    34,    24,    35,    66,    32,    64,    27,   129,
     130,    34,    46,    35,   110,    51,    34,    51,    35,    32,
     140,    34,    52,    35,   126,   159,   160,   136,    55,    35,
      56,   111,   165,   147,    71,    58,    31,    59,    72,    60,
     132,    61,    32,    33,   173,   174,    35,   108,   109,   131,
     109,    92,    93,    62,    62,    58,    58,    59,    59,    60,
      60,    61,    61,   142,   -32,    95,    99,    62,    62,    58,
      58,    59,    59,    60,    60,    61,    61,   175,    11,   124,
      34,    31,    35,   158,   100,   128,   176,    32,    33,   162,
      31,   133,   134,    83,   135,   101,    32,    33,   137,    84,
      19,    85,   102,    12,   103,   104,   117,   138,    86,   141,
      87,   143,   118,   145,   119,    88,   148,   151,   150,   152,
     154,   120,   153,   121,   155,    34,   156,    35,   122,   157,
     161,   167,   163,   166,    34,   168,    35,   169,   170,   177,
      18,   178,    20,   146,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   107
};

static const yytype_int16 yycheck[] =
{
      36,     3,    36,    11,    36,    41,    36,    64,    36,    31,
       3,    16,     8,     9,    24,    52,    28,    43,     0,    41,
      28,    50,    48,    65,    18,     3,     5,    25,    22,    55,
      55,     8,     9,    18,    28,    29,    28,    22,    18,    49,
      66,    43,    22,    28,    29,    82,    55,    22,    28,    29,
      43,    35,    54,    28,    29,    65,    61,    28,    94,    28,
      94,    97,    94,    89,    94,    43,    94,    60,    11,   126,
      11,    31,    66,    28,    68,    97,    28,    46,    28,   105,
     106,    66,    55,    68,    11,    28,    66,    28,    68,    28,
     126,    66,    35,    68,    46,   152,   153,   123,    35,    68,
      35,    28,    18,   139,    35,   139,    22,   139,    28,   139,
     108,   139,    28,    29,   171,   172,    68,     8,     9,     8,
       9,    28,    14,   159,   160,   159,   160,   159,   160,   159,
     160,   159,   160,   131,    35,    17,    55,   173,   174,   173,
     174,   173,   174,   173,   174,   173,   174,    18,    24,    54,
      66,    22,    68,   151,    37,    35,    18,    28,    29,   157,
      22,    28,    13,    19,    54,    48,    28,    29,    55,    25,
      46,    27,    55,    49,    57,    58,    19,    55,    34,    28,
      36,    54,    25,    13,    27,    41,    18,     8,    54,    46,
      18,    34,    46,    36,    55,    66,    54,    68,    41,     8,
      55,    46,    55,    55,    66,    55,    68,    55,    46,    55,
      13,    55,    14,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    71
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    50,    70,    72,    28,     0,    65,    73,    55,    28,
      74,    24,    49,    65,    76,    81,     8,     9,    74,    46,
      81,    28,    31,    75,    28,     8,    71,    28,    77,    55,
      75,    22,    28,    29,    66,    68,    83,    84,    88,    91,
      96,    99,   101,   102,    79,    35,    55,    93,   102,     5,
      11,    28,    35,   100,   104,    35,    35,    18,    84,    88,
      91,    96,   101,   110,    46,   101,   102,    28,    85,   103,
     104,    35,    28,    82,    16,    61,    94,   104,   100,     3,
      43,    60,   107,    19,    25,    27,    34,    36,    41,   108,
      90,   104,    28,    14,    83,    17,    97,    98,    86,    55,
      37,    48,    55,    57,    58,   105,   106,    82,     8,     9,
      11,    28,    95,    54,   100,   104,    89,    19,    25,    27,
      34,    36,    41,   109,    54,    18,    46,   101,    35,   104,
     104,     8,    75,    28,    13,    54,   104,    55,    55,    83,
     101,    28,    75,    54,    92,    13,    97,   101,    18,    87,
      54,     8,    46,    46,    18,    55,    54,     8,    75,    83,
      83,    55,    75,    55,    18,    18,    55,    46,    55,    55,
      46,    78,    80,    83,    83,    18,    18,    55,    55
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 64 "pascal.y"
    {output << "\n.main" << endl;;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 64 "pascal.y"
    {output << "\n\tSWI SWI_FINISH\n\tEXIT"; cout << "Executed successfully";;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 67 "pascal.y"
    {output << "\tAREA " <<  varStr << ", CODE, READONLY\n\n\tENTRY" << endl;;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 75 "pascal.y"
    {addTable(varStr);}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 76 "pascal.y"
    {addTable(varStr);}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 83 "pascal.y"
    {addFuncTable(varStr); addFuncVarTable(varStr);;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 83 "pascal.y"
    {output << "\tSTMED\tR13!, {R14}" << endl;;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 83 "pascal.y"
    {output << "\tLDMED\tR13!, {R15}" << endl; is_function = false;;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 84 "pascal.y"
    {addFuncTable(varStr); addFuncVarTable(varStr);;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 84 "pascal.y"
    {output << "\tSTMED\tR13!, {R14}" << endl;;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 84 "pascal.y"
    {output << "\tLDMED\tR13!, {R15}" << endl; is_function = false;;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 88 "pascal.y"
    {is_function = true;;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 89 "pascal.y"
    {is_function = true;;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 92 "pascal.y"
    {addFuncVarTable(varStr);}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 93 "pascal.y"
    {addFuncVarTable(varStr);}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 99 "pascal.y"
    {if_flag = "";;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 100 "pascal.y"
    {if_flag = "";;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 103 "pascal.y"
    {while_flag = "";;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 104 "pascal.y"
    {while_flag = "";;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 110 "pascal.y"
    {codeGen("MOV" + if_flag, 2, "R" + intToStr(param_dest), ""); op_loc = 0;
										output << "\tBL" + if_flag << "\t." << func_table[func_dest].id << endl;
										codeGen("MOV" + if_flag, dest, "R1", ""); op_loc = 0;;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 115 "pascal.y"
    {func_dest = searchFunc(varStr);;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 115 "pascal.y"
    {param_dest = searchLocation(varStr);;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 119 "pascal.y"
    {loop = while_loop; output << "\n.while" + intToStr(while_loop) << endl; while_loop++;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 120 "pascal.y"
    {codeGen("CMP", dest, while_op[0], while_op[1]);
				codeGen(("B"+while_flag), dest, operand[0], operand[1]); op_loc = 0;;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 124 "pascal.y"
    {while_op[0] = operand[0]; while_op[1] = operand[1]; op_loc = 0;;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 127 "pascal.y"
    {loop = for_loop; output << "\n.for" + intToStr(for_loop) << endl; for_loop++; for_dest = dest;;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 128 "pascal.y"
    {codeGen(for_instruct, for_dest, ("R" + intToStr(for_dest)), for_inc); 
				codeGen("CMP", dest, ("R" + intToStr(for_dest)), for_op);
				codeGen("BNE", dest, operand[0], operand[1]); op_loc = 0;;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 133 "pascal.y"
    {codeGen("MOV", dest, operand[0], operand[1]); op_loc = 0;;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 136 "pascal.y"
    {for_instruct = "ADD";;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 137 "pascal.y"
    {for_instruct = "SUB";;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 140 "pascal.y"
    {for_op = "#" + varStr;;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 141 "pascal.y"
    {for_op = "R" + intToStr(searchLocation(varStr)); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 155 "pascal.y"
    {if (if_flag == "EQ") if_flag = "NE";
					 else if (if_flag == "NE") if_flag = "EQ";
					 else if (if_flag == "GE") if_flag = "LT";
					 else if (if_flag == "LE") if_flag = "GT";
					 else if (if_flag == "GT") if_flag = "LE";
					 else if (if_flag == "LT") if_flag = "GE";;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 166 "pascal.y"
    {codeGen("CMP", dest, operand[0], operand[1]); op_loc = 0;;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 171 "pascal.y"
    {codeGen("WRITE", searchLocation(varStr), "", ""); op_loc = 0;;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 172 "pascal.y"
    {codeGen(("MOV" + if_flag), dest, "R" + (intToStr(dest+(location-5))), operand[0]); op_loc = 0;;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 175 "pascal.y"
    {dest = searchLocation(varStr);;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 179 "pascal.y"
    {codeGen(instruct, dest+(location-5), "R" + (intToStr(dest+(location-5))), operand[0]); op_loc = 0;;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 180 "pascal.y"
    {codeGen(instruct, dest+(location-5), "R" + (intToStr(dest+(location-5))), operand[0]); op_loc = 0;;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 181 "pascal.y"
    {codeGen(("MOV" + if_flag), dest+(location-5), operand[0], operand[1]); op_loc = 0;;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 184 "pascal.y"
    {operand[op_loc] = "#" + varStr; op_loc++;;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 185 "pascal.y"
    {operand[op_loc] = "R" + intToStr(searchLocation(varStr)); op_loc++;;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 189 "pascal.y"
    {instruct = "ADD" + if_flag;;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 190 "pascal.y"
    {instruct = "SUB" + if_flag;;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 193 "pascal.y"
    {instruct = "MUL" + if_flag;;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 194 "pascal.y"
    {instruct = "DIV" + if_flag;;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 202 "pascal.y"
    {if_flag = "EQ";;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 203 "pascal.y"
    {if_flag = "NE";;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 204 "pascal.y"
    {if_flag = "GE";;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 205 "pascal.y"
    {if_flag = "LE";;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 206 "pascal.y"
    {if_flag = "GT";;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 207 "pascal.y"
    {if_flag = "LT";;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 211 "pascal.y"
    {while_flag = "EQ";;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 212 "pascal.y"
    {while_flag = "NE";;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 213 "pascal.y"
    {while_flag = "GE";;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 214 "pascal.y"
    {while_flag = "LE";;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 215 "pascal.y"
    {while_flag = "GT";;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 216 "pascal.y"
    {while_flag = "LT";;}
    break;



/* Line 1455 of yacc.c  */
#line 2017 "pascal.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 222 "pascal.y"


void yyerror(const char *s)
{
	cout << endl << "Syntax error at line " << linenum << " and col " << position << endl;
}

void addTable(string temp_id)
{
	//Resize to vector to correct size
	var_table.resize(location);
	
	//Temporary symbol
	struct symbolTable symbol;
	
	if (location < 16) {
		symbol.id = temp_id;
		
		//Insert symbol in table
		var_table.push_back(symbol);
		
		location++;
		cout << "mem_location: " << location << endl;
	}
	else
		//Error
		cerr << "Not enough memory locations" << endl;
}

void addFuncVarTable(string temp_id)
{
	//Resize to vector to correct size
	func_var_table.resize(func_var_location);
	
	//Temporary symbol
	struct symbolTable symbol;
	
	if (func_var_location < 16) {
		symbol.id = temp_id;
		
		//Insert symbol in table
		func_var_table.push_back(symbol);
		
		func_var_location++;
		cout << "func_var_location: " << func_var_location << endl;
	}
	else
		//Error
		cerr << "Not enough memory locations" << endl;
}

void addFuncTable(string temp_id)
{
	//Resize to vector to correct size
	func_table.resize(func_location);
	
	//Temporary symbol
	struct symbolTable symbol;
	
	if (func_location < 16) {
		symbol.id = temp_id;
		
		//Insert symbol in table
		func_table.push_back(symbol);
		
		func_location++;
		cout << "func_location: " << func_location << endl;
		
		//Print name of function label
		output << "\n." + symbol.id << endl; 
	}
	else
		//Error
		cerr << "Not enough memory locations" << endl;
}

int searchLocation(string temp_id) 
{
	//Variable is global
	if (is_function == false) {
		for (int i=0; i<16; i++) {
			if (temp_id == var_table[i].id) {
				return i;
			}
		}
	}
	
	//Variable is local to a function
	else if(is_function == true){
		for (int i=0; i<16; i++) {
			if (temp_id == func_var_table[i].id) {
				return i;
			}
		}
	}
	
	//Error
	else
		cerr << "Variable " << temp_id << " has not been declared!" << endl;
}

int searchFunc (string temp_id)
{
	//Look up label of a function
	for (int i=0; i<16; i++) {
		if (temp_id == func_table[i].id) {
			return i;
		}
	}
	
	//Error
	cerr << "Variable " << temp_id << " has not been declared!" << endl;
}
		
void codeGen (string instruct, int dest, string operand1, string operand2)
{
	//MOV instruction
	if (instruct == "MOV" + if_flag)
		output << "\t" << instruct << "\tR" << dest << ", " << operand1 << endl;
	
	//CMP instruction
	else if (instruct == "CMP")
		output << "\t" << instruct << "\t" << operand1 << ", " << operand2 << endl;
	
	//B instruction (for loops)
	else if (instruct == "BNE" && while_flag != "NE")
		output << "\t" << instruct << "\t" << ".for" + intToStr(loop) << endl << endl;
	
	//B instruction (while loops)
	else if (instruct == "B" + while_flag)
		output << "\t" << instruct << "\t" << ".while" + intToStr(loop) << endl << endl;
	
	//Print variables to screen
	else if (instruct == "WRITE") {
		output << "\t" << "MOV" + if_flag <<"\tR0, R" << dest << endl;
		output << "\tBL" + if_flag << "\tPRINTR0_" << endl;
	}
	
	//ADD,SUB,MUL instructions
	else	
		output << "\t" << instruct << "\tR" << dest << ", " << operand1 << ", " << operand2 << endl;
}

string intToStr (int temp_int)
{
	stringstream ss;
	string temp_string = "";
	
	//Dump int into stringstream
	ss << temp_int;
	
	//Dump stringstream into string
	ss >> temp_string;
	
	return temp_string;
}

int main(int argc,char** argv)
{

	//Open pascal file
	yyin = fopen(argv[1],"r");
	
	//Open output file
	output.open("output.txt", ios_base::in | ios_base::out | ios_base::trunc);
	
	//Parse pascal file
	yyparse();
	
	//Close output file
	output.close();
	
	//Close pascal file
	fclose(yyin);
}
