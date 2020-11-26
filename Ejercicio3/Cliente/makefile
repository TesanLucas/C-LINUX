.PHONY: clean

comp=gcc

cliente: cliente.o
	$(comp) -o cliente cliente.o -lpthread

cliente.o: cliente.c
	$(comp) -c cliente.c

clean:
	rm *.o
