all: library client server clean

library: biblioteca.c
	gcc -c -g biblioteca.c

client: client.o
	gcc -o client client.o biblioteca.o -lpthread

client.o: client.c
	gcc -c -g client.c

server: server.o
	gcc -o server server.o biblioteca.o -lpthread

server.o: server.c
	gcc -c -g server.c 

clean: 
	rm *.o

