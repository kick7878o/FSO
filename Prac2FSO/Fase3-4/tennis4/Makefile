all : memoria.o winsuport2.o semafor.o missatge.o tennis4 pal_ord4

winsuport2.o : winsuport2.c winsuport2.h
	gcc -c -Wall winsuport2.c -o winsuport2.o 

memoria.o : memoria.c memoria.h
	gcc -c -Wall memoria.c -o memoria.o 

semafor.o : semafor.c semafor.h
	gcc -c -Wall -c semafor.c -o semafor.o

missatge.o : missatge.c missatge.h
	gcc -c -Wall -c missatge.c -o missatge.o
	
tennis4: tennis4.c winsuport2.o winsuport2.h
	gcc -Wall tennis4.c  memoria.o winsuport2.o semafor.o missatge.o -o tennis4 -lcurses -lpthread

pal_ord4: pal_ord4.c winsuport2.o winsuport2.h
	gcc -Wall pal_ord4.c memoria.o winsuport2.o semafor.o missatge.o -o pal_ord4 -lcurses 

clean:
	rm memoria.o winsuport2.o semafor.o missatge.o
	rm pal_ord4 pal_ord4.o tennis4 tennis4.o
