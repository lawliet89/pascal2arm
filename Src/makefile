# Makefile for Pascal to ARM Compiler
#-------------------------------------------------------------------------------
# USER SETTINGS
#-------------------------------------------------------------------------------

# Variable that contains the name of the program
PROGRAM := a

# Directory to store generated files
GEN_DIR := Gen/

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
$(PROGRAM): $(GEN_DIR)main.o $(GEN_DIR)utility.o $(GEN_DIR)lexer.o $(GEN_DIR)parser.o $(GEN_DIR)token.o
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -o $(PROGRAM) $^
	@echo "Make complete."
	
$(GEN_DIR)main.o: main.cpp utility.h functions.h 
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)utility.o: utility.cpp utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)lexer.o: $(FLEX_CPP) $(BISON_HEADER) functions.h utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(FLEX_CPP) $(FLEX_HEADER): $(FLEX_SRC)
	@echo "Generating lexer"
	@$(FLEX) -o $(FLEX_CPP) --header-file=$(FLEX_HEADER) $(FLEX_SRC)
	
$(BISON_CPP) $(BISON_HEADER): $(BISON_SRC)
	@echo "Generating parser"
	@$(BISON) --defines=$(BISON_HEADER) --output=$(BISON_CPP) $(BISON_SRC)
	
$(GEN_DIR)parser.o: $(BISON_CPP) $(FLEX_HEADER) functions.h utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)token.o: token.cpp token.h 
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
functions.h: token.h
	
#Tokens specialisation header generation	
token.h: $(GEN_DIR)all.h $(BISON_HEADER)
#	@echo "Regenerating all.h"

$(GEN_DIR)all.h: $(GEN_DIR)all_gen $(wildcard Tokens/*.h) 
	@echo "Regenerating all.h"
	@ls ./Tokens/*.h | $(GEN_DIR)all_gen $(GEN_DIR)all.h
	
$(GEN_DIR)all_gen: all_gen.cpp
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
	@rm -f -v Gen/*
	@echo 'Cleaned.'
again:
	@make clean
	@make
clean-temp:
	@rm -f -v *~