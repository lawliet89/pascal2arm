/*******************************************************************************
 * 
 * Operation functions for the compiler. Not related to Flex or Bison
 * 
 ******************************************************************************/

#include "op.h"
//Parse input args - http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html#Getopt-Long-Options
#include <unistd.h>
#include <getopt.h>	
#include <fstream>
#include <iostream>
#include <cstdio>

//Global flags
Flags_T Flags;
//Output String
std::stringstream OutputString;

extern bool LexerSyntaxError, ParseError; 	//lexer.l and parser.y
extern unsigned LexerCharCount, LexerLineCount;
extern FILE *yyin;	

/**
 * Handle SIGABRT
 * 
 * */

void HandleAbort(int param){
	//Clean up
	
	std::cout << "Compilation Terminated." << std::endl;
	exit(param);
}

/**
 * 	Command Arguments Parsing
 * */

//Parse arguments. Based on example @ http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html#Getopt-Long-Option-Example
void ParseArg(int argc, char **argv){
	int c;
	
	//Define options
	static option long_options[] =
	{
		/* These options set a flag. */
		//{"verbose", no_argument,       &verbose_flag, 1},
		//{"brief",   no_argument,       &verbose_flag, 0},
		/* These options don't set a flag.
		 *        We distinguish them by their indices. */
		//{"add",     no_argument,       0, 'a'},
		//{"append",  no_argument,       0, 'b'},
		//{"delete",  required_argument, 0, 'd'},
		//{"create",  required_argument, 0, 'c'},
		{"output",    required_argument, 0, 'o'},
		{"hint", no_argument, 0, 'H'},
		{"pedantic", no_argument, 0 , 'p'},
		{ NULL, no_argument, NULL, 0 }			//Not entirely sure what this is for
	};	
	
	
	/*
	 * Initialise some flags
	 * */
	//Default output is -o is not set
	Flags.Output = &std::cout;
	Flags.OutputPath = "std::cout";
	
	Flags.ShowHints = false;
	Flags.Pedantic = false;
	
	Flags.AsmHeaderPath = "Asm/header.s";		//TODO-Handle later
	Flags.AsmStdLibPath = "Asm/stdlib.s";
	Flags.AsmStackPath = "Asm/stack.s";
	
	/** Handle Args **/
	
	while (1){		//TODO - Deal with no arguments & options
		//Index of the option in the long_options array
		int option_index = 0;		
		
		c = getopt_long (argc, argv, "Hpo:",	//Change short options here!
				 long_options, &option_index);
		
		if (c == -1) //getopt_long will return -1 at the end of options
			break;
		
		switch (c){
			case 0:		//Long option detected
				//Flag
				//if (long_options[option_index].flag)
				//	break;
				//Option
				//printf ("option %s", long_options[option_index].name);
				//if (optarg)		//Option target
					//printf (" with arg %s", optarg);
				break;
			//Other Flags
			case 'o':	//Output file flag
				Flags.OutputPath = optarg;
				//Check for stuff
				break;
			case 'H':
				Flags.ShowHints = true;
				break;
			case 'p':
				Flags.Pedantic = true;
				break;
			case '?':
				/* getopt_long already printed an error message. */
				break;
			default:	//Shouldn't happen.
				abort ();
		}
	}
	
	/* For any remaining command line arguments (not options). */
	if (optind < argc)		//optind is set via getopt_long: see http://www.gnu.org/software/libc/manual/html_node/Using-Getopt.html#Using-Getopt
	{
		Flags.Input = fopen(argv[optind],"r");
		Flags.InputPath = argv[optind];
		
	}
	else{
		Flags.Input = stdin;
		Flags.InputPath = "std::cin";
	}
}

/**
 *  Compile error handling
 * 
 * 	- All Syntax Errors are fatal
 * */

void HandleError(const char* message, ErrorClass_T ErrorClass, ErrorLevel_T level, unsigned line, unsigned character){
	switch(level){
		case E_NOTICE:
			std::cout << "Notice";
			break;
		case E_WARNING:
			std::cout << "Warning";
			break;
		case E_FATAL:
			std::cout << "Fatal Error";
			break;		
		case E_ERROR:
		default:
			std::cout << "Error";
	}
	
	//Line and chara no
	if (line || character)
		std::cout << "(";
		
	if (line)
		std::cout << line;
	if (character)
		std::cout << ":" << character-1;
	//Line and chara no
	if (line || character)
		std::cout << ")";
	
	std::cout << ": ";
	
	switch(ErrorClass){
		case E_SYNTAX:
			std::cout << "Syntax Error - ";
			LexerSyntaxError = true;
			break;
		case E_PARSE:
			std::cout << "Parse Error - ";
			ParseError = true;
			break;
		default:
			//std::cout << "Error - ";
			break;
	}
	
	std::cout << message;
	
	std::cout << std::endl;

	if (level == E_FATAL)
		abort();
}

//Initialise Lexer and parser
void LexerInit(){
	//Setup yyin
	yyin = INPUT;
	LexerCharCount = 1;
	LexerLineCount = 1;
	
	//CurrentToken = NULL;
	LexerSyntaxError = false;
	ParseError = false;
}