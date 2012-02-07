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
BISON_CPP := bison.cpp


#-------------------------------------------------------------------------------
# END USER SETTINGS
#-------------------------------------------------------------------------------

COMPILER := g++
FLEX := flex
BISON := bison

#Flags for compiler
CXXFLAGS := -pedantic -std=c++0x

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
$(PROGRAM): main.o utility.o
	@$(COMPILER) $(CXXFLAGS) -o $(PROGRAM) main.o utility.o
	@echo "Compiling $@"
	
main.o: main.cpp utility.h
	@$(COMPILER) $(CXXFLAGS) -c -o main.o main.cpp
	@echo "Compiling $@"
	
utility.o: utility.cpp utility.h
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	@echo "Compiling $@"

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