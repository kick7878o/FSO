all : winsuport.o tennis2

winsuport.o : winsuport.c winsuport.h
	gcc -c winsuport.c -o winsuport.o 

tennis2 : tennis2.c winsuport.o winsuport.h
	gcc tennis2.c winsuport.o -o tennis2 -lcurses -lpthread

clean: 
	rm winsuport.o tennis2
