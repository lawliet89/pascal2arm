/*******************************************************************************
 * 
 * Utility functions for the compiler. Not related to Flex or Bison
 * 
 ******************************************************************************/

#include "utility.h"
//Parse input args - http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Options.html#Getopt-Long-Options
#include <unistd.h>
#include <getopt.h>	
#include <fstream>
#include <iostream>

/**
 * 	Command Arguments Parsing
 * */

//Global flags
Flags_T Flags;

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
		{ NULL, no_argument, NULL, 0 }			//Not entirely sure what this is for
	};	
	
	
	/*
	 * Initialise some flags
	 * */
	Flags.Output = &std::cout;
	
	while (1){		//TODO - Deal with no arguments & options
		//Index of the option in the long_options array
		int option_index = 0;		
		
		c = getopt_long (argc, argv, "o:",	//Change short options here!
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
				Flags.Output = new std::ofstream(optarg);
				//Check for stuff
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
		//printf ("non-option ARGV-elements: ");
		//while (optind < argc)
		//	printf ("%s ", argv[optind++]);
		//putchar ('\n');
	}
}