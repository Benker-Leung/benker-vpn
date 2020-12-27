#ifndef __UDP_H
#define __UDP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h> 

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
 * @param laddr the local address bind to (ip & port)
 * 
 * @return the file descriptor if success, else -1 for errors (fail to create socket or bind to laddr)
*/
int get_udp_socket_server(struct sockaddr_in* laddr);

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

#endif