all : winsuport.o tennis0

winsuport.o : winsuport.c winsuport.h
	gcc -Wall -c winsuport.c -o winsuport.o

tennis0 : tennis0.c winsuport.o winsuport.h
	gcc -Wall tennis0.c winsuport.o -o tennis0 -lcurses

clean: 
	rm winsuport.o tennis0
