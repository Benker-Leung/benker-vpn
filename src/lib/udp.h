#ifndef __UDP_H
#define __UDP_H

// for function inet_aton()
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/**
 * get the udp socket fd for client
 * 
 * @return the file descriptor if success, else -1 for errors
 * 
*/
int get_udp_socket_client();

/**
 * get the udp socket fd for server, also bind with the laddr given
 * 
 * @param port the port used (0.0.0.0:port)
 * 
 * @return the file descriptor if success, else -1 for errors (fail to create socket or bind to laddr)
*/
int get_udp_socket_server(int port);

/**
 * send message via udp
 * 
 * @param fd file descriptor of the socket
 * @param buffer pointer to the message
 * @param size size of the message in bytes
 * @param dst destination ip & port data structure
 * 
 * @return total bytes sent, -ve for errors
 * 
*/
int send_msg_udp(int fd, unsigned char* buffer, int size, struct sockaddr_in* dst);

/**
 * receive message via udp
 * 
 * @param fd file descriptor of the socket
 * @param buffer pointer to buffer for storing the msg
 * @param size size of the buffer in bytes
 * @param raddr the remote address
 * 
 * @return total bytes read, -ve for errors
*/
int read_msg_udp(int fd, unsigned char* buffer, int size, struct sockaddr_in* raddr);

/**
 * fill in the struct sockaddr_in
 * 
 * @param addr struct sockaddr_in to be filled in
 * @param port port number
 * @param ip ip-addr e.g "10.0.0.1"
 * 
 * @return 0 if success, -1 if fail
 *
*/
int fill_port_ip(struct sockaddr_in* addr, int port, const char* ip);

#endif