CC := gcc
CFLAGS := -Wall

all: client

.PHONY: client
client: client/main.o lib/udp.o
	$(CC) $(CFLAGS) -o client.exe client/main.o lib/udp.o

client/main.o: client/main.c lib/udp.h
	$(CC) $(CFLAGS) -c client/main.c -o client/main.o

lib/udp.o: lib/udp.c lib/udp.h
	$(CC) $(CFLAGS) -c lib/udp.c -o lib/udp.o

clean: clean_client clean_lib
	rm -f *.exe

clean_client:
	rm -f client/*.o

clean_lib:
	rm -f lib/*.o
