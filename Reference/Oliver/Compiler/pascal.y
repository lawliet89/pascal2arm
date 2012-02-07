%{ 
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

%}


%token AND ARRAY ASSIGNMENT CASE COMMENT COLON COMMA CONST DIGIT
%token DIV DO DOT DOTDOT DOWNTO ELSE END EQUAL EOLINE EXTERNAL FOR FORWARD FUNCTION
%token GE GOTO GT IDENTIFIER IF IN INTEGER LABEL LBRAC LE LPAREN LT MINUS MOD NIL NOT
%token NOTEQUAL OF OR OTHERWISE PACKED PBEGIN PFILE PLUS PROCEDURE PROGRAM RBRAC
%token RECORD REPEAT RPAREN SEMICOLON SET SLASH STAR STARSTAR THEN
%token TO TYPE UNTIL UPARROW VAR WHILE WITH WRITE


%%

/* PROGRAM STRUCTURE */	
program : program_header var_declaration function PBEGIN {output << "\n.main" << endl;} line end_prg {output << "\n\tSWI SWI_FINISH\n\tEXIT"; cout << "Executed successfully";}
    ;
	
program_header : PROGRAM IDENTIFIER SEMICOLON {output << "\tAREA " <<  varStr << ", CODE, READONLY\n\n\tENTRY" << endl;}
	;

/* VAR DECLARATIONS */		
var_declaration : VAR variables COLON var_type SEMICOLON
	| var_declaration VAR variables COLON var_type SEMICOLON
	;
	
variables : IDENTIFIER 				{addTable(varStr)}
	| variables COMMA IDENTIFIER 	{addTable(varStr)}
	;
	
var_type : INTEGER
	;

/* Body of functions */		
function : func_type IDENTIFIER {addFuncTable(varStr); addFuncVarTable(varStr);} LPAREN func_var COLON var_type RPAREN COLON var_type SEMICOLON PBEGIN {output << "\tSTMED\tR13!, {R14}" << endl;} line END SEMICOLON		{output << "\tLDMED\tR13!, {R15}" << endl; is_function = false;}
	| function func_type IDENTIFIER {addFuncTable(varStr); addFuncVarTable(varStr);} LPAREN func_var COLON var_type RPAREN COLON var_type SEMICOLON PBEGIN {output << "\tSTMED\tR13!, {R14}" << endl;} line END SEMICOLON	{output << "\tLDMED\tR13!, {R15}" << endl; is_function = false;}
	|
	;
	
func_type : FUNCTION 	{is_function = true;}
	| PROCEDURE			{is_function = true;}
	;

func_var : IDENTIFIER 				{addFuncVarTable(varStr)}	
	| func_var COMMA IDENTIFIER 	{addFuncVarTable(varStr)}
	;

/* MAIN BODY */		
line : statement						
	| line statement								
	| if_loop			{if_flag = "";}
	| line if_loop		{if_flag = "";}
	| for_loop
	| line for_loop
	| while_loop		{while_flag = "";}
	| line while_loop	{while_flag = "";}
	| func_call
	| line func_call
	;

/* FUNCTION CALLS */		
func_call : assignment func SEMICOLON 	{codeGen("MOV" + if_flag, 2, "R" + intToStr(param_dest), ""); op_loc = 0;
										output << "\tBL" + if_flag << "\t." << func_table[func_dest].id << endl;
										codeGen("MOV" + if_flag, dest, "R1", ""); op_loc = 0;}
	;
	
func : IDENTIFIER {func_dest = searchFunc(varStr);} LPAREN IDENTIFIER {param_dest = searchLocation(varStr);} RPAREN
	;
	
/* WHILE LOOPS */	
while_loop : WHILE LPAREN while_conditions {loop = while_loop; output << "\n.while" + intToStr(while_loop) << endl; while_loop++} RPAREN DO PBEGIN line END SEMICOLON
				{codeGen("CMP", dest, while_op[0], while_op[1]);
				codeGen(("B"+while_flag), dest, operand[0], operand[1]); op_loc = 0;}
	;
	
while_conditions : factor while_op factor {while_op[0] = operand[0]; while_op[1] = operand[1]; op_loc = 0;}

/* FOR LOOPS */		
for_loop : FOR for_ini for_to for_end DO {loop = for_loop; output << "\n.for" + intToStr(for_loop) << endl; for_loop++; for_dest = dest;} PBEGIN line END SEMICOLON	
				{codeGen(for_instruct, for_dest, ("R" + intToStr(for_dest)), for_inc); 
				codeGen("CMP", dest, ("R" + intToStr(for_dest)), for_op);
				codeGen("BNE", dest, operand[0], operand[1]); op_loc = 0;}
	;

for_ini: assignment factor	{codeGen("MOV", dest, operand[0], operand[1]); op_loc = 0;} 		
	;
	
for_to : TO			{for_instruct = "ADD";}
	| DOWNTO		{for_instruct = "SUB";}
	;
	
for_end : DIGIT 			{for_op = "#" + varStr;}
	| IDENTIFIER 			{for_op = "R" + intToStr(searchLocation(varStr)); }
	;

/* IF LOOPS */	
if_loop : if_statement statement else_loop							
	| if_statement PBEGIN line END SEMICOLON else_loop						
	;
	
else_loop : else statement
	| else PBEGIN statement END SEMICOLON
	| else PBEGIN line statement END SEMICOLON
	|
	;
	
else : ELSE			{if (if_flag == "EQ") if_flag = "NE";
					 else if (if_flag == "NE") if_flag = "EQ";
					 else if (if_flag == "GE") if_flag = "LT";
					 else if (if_flag == "LE") if_flag = "GT";
					 else if (if_flag == "GT") if_flag = "LE";
					 else if (if_flag == "LT") if_flag = "GE";}
	;
	
if_statement : IF if_condition THEN 
	;
	
if_condition : factor if_op factor 			{codeGen("CMP", dest, operand[0], operand[1]); op_loc = 0;}
	| LPAREN if_condition RPAREN 	
	| if_condition boolop if_condition  
	;
	
statement : WRITE LPAREN IDENTIFIER RPAREN SEMICOLON	{codeGen("WRITE", searchLocation(varStr), "", ""); op_loc = 0;}
	| assignment expr SEMICOLON 	{codeGen(("MOV" + if_flag), dest, "R" + (intToStr(dest+(location-5))), operand[0]); op_loc = 0;}
	;
	
assignment: IDENTIFIER ASSIGNMENT {dest = searchLocation(varStr);}
	;

/* Arithmetics - NOTE: UNABLE TO USE 'PROPER' GRAMMAR SUCH AS IN SAMPLE CALCULATOR. Did not have enough time to solve...*/
expr : expr addop factor		{codeGen(instruct, dest+(location-5), "R" + (intToStr(dest+(location-5))), operand[0]); op_loc = 0;}
	| expr mulop factor			{codeGen(instruct, dest+(location-5), "R" + (intToStr(dest+(location-5))), operand[0]); op_loc = 0;}
	| factor					{codeGen(("MOV" + if_flag), dest+(location-5), operand[0], operand[1]); op_loc = 0;}
	; 
	
factor : DIGIT 		{operand[op_loc] = "#" + varStr; op_loc++;}
	| IDENTIFIER 	{operand[op_loc] = "R" + intToStr(searchLocation(varStr)); op_loc++;}
	;

/* Operators - sets instruction opcode for ADD, SUB, MUL, DIV*/	
addop : PLUS		{instruct = "ADD" + if_flag;}
	| MINUS 		{instruct = "SUB" + if_flag;}
	;
	
mulop : STAR 		{instruct = "MUL" + if_flag;}
	| SLASH 		{instruct = "DIV" + if_flag;}
	;	
	
boolop : AND	
	| OR		
	;
	
/* if conditions - sets flag accordingly */
if_op : EQUAL		{if_flag = "EQ";}
	| NOTEQUAL		{if_flag = "NE";}
	| GE 			{if_flag = "GE";}
	| LE			{if_flag = "LE";}
	| GT			{if_flag = "GT";}
	| LT			{if_flag = "LT";}
	;

/* while conditions - sets flag accordingly */	
while_op : EQUAL	{while_flag = "EQ";}
	| NOTEQUAL		{while_flag = "NE";}
	| GE 			{while_flag = "GE";}
	| LE			{while_flag = "LE";}
	| GT			{while_flag = "GT";}
	| LT			{while_flag = "LT";}
	;
	
end_prg : END DOT 
	;
	
%%

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