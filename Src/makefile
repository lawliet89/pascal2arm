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
FLEX_HEADER := $(GEN_DIR)lexer.h

# Header file for Bison
BISON_HEADER := $(GEN_DIR)parser.h

# CPP File for Flex
FLEX_CPP := $(GEN_DIR)lexer.cpp

# CPP File for Bison
BISON_CPP := $(GEN_DIR)parser.cpp

DEBUG_BUILD := true

BISON_DEBUG := true

OBJ_LIST := $(GEN_DIR)main.o $(GEN_DIR)utility.o $(GEN_DIR)lexer.o $(GEN_DIR)parser.o $(GEN_DIR)token.o $(GEN_DIR)int.o \
$(GEN_DIR)real.o $(GEN_DIR)symbols.o $(GEN_DIR)asm.o $(GEN_DIR)op.o $(GEN_DIR)idList.o $(GEN_DIR)var.o \
$(GEN_DIR)type.o $(GEN_DIR)func.o $(GEN_DIR)char.o $(GEN_DIR)expression.o $(GEN_DIR)term.o $(GEN_DIR)factor.o\
$(GEN_DIR)exprlist.o $(GEN_DIR)formalparam.o
#-------------------------------------------------------------------------------
# END USER SETTINGS
#-------------------------------------------------------------------------------

COMPILER := g++
FLEX := flex
BISON := bison

#Flags for compiler
ifdef DEBUG_BUILD
CXXFLAGS := -g -pedantic -std=c++0x -Wall -Wextra -Werror=return-type -Wno-reorder
else
CXXFLAGS := -pedantic -std=c++0x -Wall -Wextra -Werror=return-type -Wno-reorder
endif

ifdef BISON_DEBUG
BISON_FLAGS := --report=itemset --debug -g
else
BISON_FLAGS := 
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
$(PROGRAM): $(OBJ_LIST)
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -o $(PROGRAM) $^
	@echo "Make complete."
	
$(GEN_DIR)main.o: main.cpp utility.h compiler.h  op.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)utility.o: utility.cpp utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)op.o: op.cpp op.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)lexer.o: $(FLEX_CPP) $(BISON_HEADER) compiler.h utility.h op.h define.h token.h $(GEN_DIR)all.h asm.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(FLEX_CPP) $(FLEX_HEADER): $(FLEX_SRC)
	@echo "Generating lexer"
	@$(FLEX) -o $(FLEX_CPP) --header-file=$(FLEX_HEADER) $(FLEX_SRC)
	
$(BISON_CPP) $(BISON_HEADER): $(BISON_SRC)
	@echo "Generating parser"
	@$(BISON) --defines=$(BISON_HEADER) --output=$(BISON_CPP) $(BISON_FLAGS) $(BISON_SRC)
	
$(GEN_DIR)parser.o: $(BISON_CPP) $(FLEX_HEADER) compiler.h  asm.h utility.h op.h define.h token.h $(GEN_DIR)all.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)token.o: token.cpp token.h 
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
	
$(GEN_DIR)asm.o: asm.cpp asm.h symbols.h utility.h 
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<

$(GEN_DIR)symbols.o: symbols.cpp symbols.h token.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<

asm.h: symbols.h define.h token.h $(GEN_DIR)all.h
	@touch $@
symbols.h: utility.h token.h
	@touch $@

#Tokens specialisation header generation	
token.h: utility.h define.h
	@touch $@

$(GEN_DIR)all.h: $(GEN_DIR)all_gen $(wildcard Tokens/*.h) 
	@echo "Regenerating all.h"
	@ls ./Tokens/*.h | $(GEN_DIR)all_gen $(GEN_DIR)all.h
	
$(GEN_DIR)all_gen: all_gen.cpp
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -o $@ $<

#Tokens/...
$(GEN_DIR)int.o: Tokens/int.cpp Tokens/int.h token.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<

$(GEN_DIR)char.o: Tokens/char.cpp Tokens/char.h token.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<

$(GEN_DIR)real.o: Tokens/real.cpp Tokens/real.h token.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	
	
$(GEN_DIR)idList.o: Tokens/idList.cpp Tokens/idList.h token.h define.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	
	
$(GEN_DIR)var.o: Tokens/var.cpp Tokens/var.h token.h define.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)type.o: Tokens/type.cpp Tokens/type.h token.h define.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)func.o: Tokens/func.cpp Tokens/func.h token.h asm.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)expression.o: Tokens/expression.cpp Tokens/expression.h token.h Tokens/term.h Tokens/type.h utility.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)term.o: Tokens/term.cpp Tokens/term.h token.h Tokens/factor.h Tokens/type.h Tokens/var.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)factor.o: Tokens/factor.cpp Tokens/factor.h token.h Tokens/expression.h Tokens/type.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)exprlist.o: Tokens/exprlist.cpp Tokens/exprlist.h token.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

$(GEN_DIR)formalparam.o: Tokens/formalparam.cpp Tokens/formalparam.h token.h
	@echo "Compiling $@"
	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	

#$(GEN_DIR)varList.o: Tokens/varList.cpp Tokens/varList.h token.h Tokens/var.h define.h
#	@echo "Compiling $@"
#	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<	
	
#functions.o: functions.cpp functions.h utility.h parser.h lexer.h
#	@$(COMPILER) $(CXXFLAGS) -c -o $@ $<
#	@echo "Compiling $@"
	
#-------------------------------------------------------------------------------
# PHONY TARGETS
#-------------------------------------------------------------------------------
.PHONY : clean again clean-temp sample
clean:
	@rm -f -v *.o $(FLEX_HEADER) $(BISON_HEADER) $(PROGRAM) $(FLEX_CPP) $(BISON_CPP) 
	@rm -f -v Gen/*
	@echo 'Cleaned.'
again:
	@make clean
	@make
clean-temp:
	@rm -f -v *~
	@rm -f -v Tokens/*~
	@rm -f -v Asm/*~
	
sample:
	./a -H -p -o Output/sample1.s Sample/sample1.pas
	./a -H -p -o Output/sample2.s Sample/sample2.pas
	./a -H -p -o Output/sample3.s Sample/sample3.pas
	./a -H -p -o Output/sample4.s Sample/sample4.pas
	./a -H -p -o Output/sample5.s Sample/sample5.pas
	./a -H -p -o Output/sample6.s Sample/sample6.pas
	./a -H -p -o Output/sample7.s Sample/sample7.pas
	./a -H -p -o Output/sample8.s Sample/sample8.pas
