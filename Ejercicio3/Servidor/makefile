.PHONY: clean

comp=gcc

cliente: servidor.o
	$(comp) -o servidor servidor.o -lpthread

cliente.o: servidor.c
	$(comp) -c servidor.c

clean:
	rm *.o
