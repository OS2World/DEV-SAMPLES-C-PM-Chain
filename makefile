# make makefile
#
# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::Watcom Linker
#  Compile::GNU C
#  Make: GNU make
all : chain.exe

chain.exe:  chain.obj minsubs.obj chain.res chain.def 
	gcc -Zomf chain.obj minsubs.obj chain.def -o chain.exe
	wrc chain.res

minsubs.obj: minsubs.c chain.h
	gcc -Wall -Zomf -c -O2 minsubs.c -o minsubs.obj

chain.obj: chain.c chain.h
	gcc -Wall -Zomf -c -O2 chain.c -o chain.obj

chain.res : chain.rc chain.dlg chain.h 
	wrc -r chain.rc

clean :
	rm -rf *exe *res *obj