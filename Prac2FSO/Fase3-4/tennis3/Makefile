all: winsuport2.o tennis3 pal_ord3 memoria.o
	
winsuport2.o: winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

memoria.o: memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o 

tennis3: tennis3.c winsuport2.o winsuport2.h
	gcc tennis3.c winsuport2.o -o tennis3 -lcurses -lpthread

pal_ord3: pal_ord3.c winsuport2.o winsuport2.h
	gcc pal_ord3.c winsuport2.o -o pal_ord3 -lcurses
	
clean: 
	rm winsuport2.o memoria.o tennis3 pal_ord3
