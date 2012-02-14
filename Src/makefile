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
$(PROGRAM): main.o utility.o lexer.o parser.o
	@$(COMPILER) $(CXXFLAGS) -o $(PROGRAM) $^
	@echo "Compiling $@"
	
main.o: main.cpp utility.h lexer.h parser.h functions.h
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	@echo "Compiling $@"
	
utility.o: utility.cpp utility.h
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	@echo "Compiling $@"

lexer.o: lexer.cpp lexer.h parser.h functions.h
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	@echo "Compiling $@"

lexer.h lexer.cpp: lexer.l
	@$(FLEX) -o $(FLEX_CPP) --header-file=$(FLEX_HEADER) $(FLEX_SRC)
	@echo "Generating lexer"
	
parser.h parser.cpp: parser.y
	@$(BISON) --defines=$(BISON_HEADER) --output=$(BISON_CPP) $(BISON_SRC)
	@echo "Generating parser"
	
parser.o: parser.cpp parser.h lexer.h functions.h
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	@echo "Compiling $@"
	
#functions.o: functions.cpp functions.h utility.h parser.h lexer.h
#	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
#	@echo "Compiling $@"
	
#-------------------------------------------------------------------------------
# PHONY TARGETS
#-------------------------------------------------------------------------------
.PHONY : clean again
clean:
	@rm -f -v *.o $(FLEX_HEADER) $(BISON_HEADER) $(PROGRAM) $(FLEX_CPP) $(BISON_CPP) 
	@echo 'Cleaned.'
again:
	@make clean
	@make