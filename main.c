/*main.c*/

//
// Main program to scan and parse nuPython programs. Takes in a python script
// (either manually entered by user or from a .py file), parses the statements into
// tokens, builds the program graph based on the tokens, then executes the program.
// 
// Editor: Brock Brown
//
// Original Author: Prof. Joe Hummel
// Northwestern University
// CS 211
//

// to eliminate warnings about stdlib in Visual Studio
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>  // true, false
#include <string.h>   // strcspn

#include "token.h"    // token defs
#include "scanner.h" 
#include "parser.h"

#include "programgraph.h"   // handle building + exeucting program graph
#include "ram.h"
#include "execute.h"


//
// main
//
// usage: program.exe [filename.py]
// 
// If a filename is given, the file is opened and serves as
// input to the program. If a filename is not given, then 
// input is taken from the keyboard until $ is input.
//
int main(int argc, char* argv[])
{
	FILE* input = NULL;
	bool  keyboardInput = false;
	
	//
	// where is the input coming from?
	//
	if (argc < 2) {
		//
		// no args, just the program name:
		//
		input = stdin;
		keyboardInput = true;
	}
	else {
		//
		// assume 2nd arg is a nuPython file:
		//
		char* filename = argv[1];
		
		input = fopen(filename, "r");
		
		if (input == NULL) // unable to open:
		{
			printf("**ERROR: unable to open input file '%s' for input.\n", filename);
			return 0;
		}
		
		keyboardInput = false;
	}
	
	if (keyboardInput)  // prompt the user if appropriate:
	{
		printf("nuPython input (enter $ when you're done)>\n");
	}
	
	//
	// call parser to check program syntax:
	//
	struct TokenQueue* tokens = parser_parse(input);
	
	if (tokens == NULL)
	{
		// 
		// program has a syntax error, error msg already output:
		//
		printf("**parsing failed...\n");
	}
	
	else
	{
		//
		// TODO:
		//
		printf("**parsing successful, valid syntax\n");
		printf("**building program graph...\n");

		// get graph structure via program_build
		struct STMT* program = programgraph_build(tokens);
		//programgraph_print(program);		// print out the initial program
		printf("**executing...\n");

		// for storing variables in program
		struct RAM* memory = ram_init();

		// exeucte the program
		execute(program, memory);
		
		printf("**done\n");
		ram_print(memory);			// print out memory by end of program
		
		tokenqueue_destroy(tokens);
	}
	
	//
	// done:
	//
	if (!keyboardInput) {
		fclose(input);
	}
	
	return 0;
}
