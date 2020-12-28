CC := gcc
CFLAGS := -Wall

all: client

.PHONY: client
client: objects/main.o objects/udp.o
	$(CC) $(CFLAGS) -o client.exe objects/main.o objects/udp.o

objects/main.o: client/main.c lib/udp.h
	$(CC) $(CFLAGS) -c client/main.c -o objects/main.o

# TODO: find a way to reduce multiple ".o"
objects/udp.o: lib/udp.c lib/udp.h
	$(CC) $(CFLAGS) -c lib/udp.c -o objects/udp.o

objects/vpn_raw_sock.o: lib/vpn_raw_sock.c lib/vpn_raw_sock.h
	$(CC) $(CFLAGS) -c lib/vpn_raw_sock.c -o objects/vpn_raw_sock.o

clean: clean_objects
	rm -f *.exe

clean_objects:
	rm -f objects/*.o

