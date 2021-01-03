CC := gcc
CFLAGS := -Wall

all: client

.PHONY: client

server: objects/udp.o objects/protocol_headers.o objects/packet_handler.o objects/session_struct.o server/main.c
	$(CC) $(CFLAGS) -c server/main.c -o objects/server.o
	$(CC) $(CFLAGS) -o server.exe objects/server.o objects/udp.o objects/protocol_headers.o objects/packet_handler.o objects/session_struct.o

client: objects/main.o objects/udp.o
	$(CC) $(CFLAGS) -o client.exe objects/main.o objects/udp.o

objects/main.o: client/main.c lib/udp.h
	$(CC) $(CFLAGS) -c client/main.c -o objects/main.o

# TODO: find a way to reduce multiple ".o"
objects/session_struct.o: lib/session_struct.h lib/session_struct.c
	$(CC) $(CFLAGS) -c lib/session_struct.c -o objects/session_struct.o

objects/packet_handler.o: lib/packet_handler.h lib/packet_handler.c lib/protocol_headers.h
	$(CC) $(CFLAGS) -c lib/packet_handler.c -o objects/packet_handler.o

objects/protocol_headers.o: lib/protocol_headers.c lib/protocol_headers.h
	$(CC) $(CFLAGS) -c lib/protocol_headers.c -o objects/protocol_headers.o

objects/udp.o: lib/udp.c lib/udp.h
	$(CC) $(CFLAGS) -c lib/udp.c -o objects/udp.o

objects/vpn_raw_sock.o: lib/vpn_raw_sock.c lib/vpn_raw_sock.h
	$(CC) $(CFLAGS) -c lib/vpn_raw_sock.c -o objects/vpn_raw_sock.o

clean: clean_objects
	rm -f *.exe

clean_objects:
	rm -f objects/*.o

