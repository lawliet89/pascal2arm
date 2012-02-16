# Makefile for Pascal to ARM Compiler
#-------------------------------------------------------------------------------
# USER SETTINGS
#-------------------------------------------------------------------------------

# Variable that contains the name of the program
PROGRAM := a

# Source file for flex
FLEX_SRC := lexer.l
# Source file for bisonc++
BISON_SRC := parser.y

# Header file name for flex
FLEX_HEADER := lexer.h

# Header file for Bison
BISON_HEADER := parser.h

# CPP File for Flex
FLEX_CPP := lexer.cpp

# CPP File for Bison
BISON_CPP := parser.cpp

DEBUG_BUILD := true

#-------------------------------------------------------------------------------
# END USER SETTINGS
#-------------------------------------------------------------------------------

COMPILER := g++
FLEX := flex
BISON := bison

#Flags for compiler
ifdef DEBUG_BUILD
CXXFLAGS := -g -pedantic -std=c++0x
else
CXXFLAGS := -pedantic -std=c++0x
endif

# UNUSED For now
# List of all the CPP files
# http://www.gnu.org/software/make/manual/make.html#Wildcards
# The order is important. The Flex and Bison CPP files must come first.
SRC_FILES := $(FLEX_CPP) $(BISON_CPP) $(wildcard *.cpp) 

# List of all associated Object files
OBJ_FILES := $(patsubst %.cpp,%.o, $(SRC_FILES) )


#-------------------------------------------------------------------------------
# ACTUAL TARGETS
#-------------------------------------------------------------------------------
$(PROGRAM): main.o utility.o lexer.o parser.o token.o
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -o $(PROGRAM) $^
	@echo "Make complete."
	
main.o: main.cpp utility.h functions.h 
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
utility.o: utility.cpp utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
lexer.o: lexer.cpp functions.h utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
lexer.h lexer.cpp: lexer.l
	@echo "Generating lexer"
	@$(FLEX) -o $(FLEX_CPP) --header-file=$(FLEX_HEADER) $(FLEX_SRC)
	
parser.h parser.cpp: parser.y
	@echo "Generating parser"
	@$(BISON) --defines=$(BISON_HEADER) --output=$(BISON_CPP) $(BISON_SRC)
	
parser.o: parser.cpp functions.h utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
token.o: token.cpp token.h 
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
functions.h: lexer.h token.h
	
#Tokens specialisation header generation	
token.h: all.h parser.h
	@echo "Regenerating all.h"

all.h: Tokens/all_gen $(wildcard Tokens/*.h) 
	@ls ./Tokens/*.h | Tokens/all_gen all.h
	
Tokens/all_gen: Tokens/all_gen.cpp
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -o $@ $<
	
#functions.o: functions.cpp functions.h utility.h parser.h lexer.h
#	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
#	@echo "Compiling $@"
	
#-------------------------------------------------------------------------------
# PHONY TARGETS
#-------------------------------------------------------------------------------
.PHONY : clean again clean-temp
clean:
	@rm -f -v *.o $(FLEX_HEADER) $(BISON_HEADER) $(PROGRAM) $(FLEX_CPP) $(BISON_CPP) 
	@rm -f -v Tokens/all_gen all.h
	@echo 'Cleaned.'
again:
	@make clean
	@make
clean-temp:
	@rm -f -v *~