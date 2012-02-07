pascal2arm: pascal.tab.o lex.yy.o 
	g++ -o pascal2arm lex.yy.o pascal.tab.o 
	
lex.yy.o: lex.yy.cpp
	g++ -c lex.yy.cpp
	
lex.yy.cpp: pascal.l
	flex pascal.l
	mv lex.yy.c lex.yy.cpp
	
pascal.tab.o: pascal.tab.cpp
	g++ -c pascal.tab.cpp
	
pascal.tab.cpp: pascal.y
	bison -d pascal.y
	mv pascal.tab.c pascal.tab.cpp

clean:
	(rm -rf lex.yy.cpp pascal.tab.cpp pascal.tab.h *.o pascal pascal.exe)