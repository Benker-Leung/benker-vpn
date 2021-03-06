#ifndef __VPN_RAW_SOCK
#define __VPN_RAW_SOCK

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <string.h>
#include <errno.h>

/**
 * get the raw socket
 * 
 * @return the file descriptor if success, else -1 for errors
*/
int get_vpn_raw_socket();


/**
 * get an packet with IP protocol (0x0800), non-blocking
 * 
 * @param fd the raw sock file descriptor
 * @param buffer the buffer(uint8_t*) to store the packet 
 * @param len the max length of the buffer, e.g if len=10, means maximum 10 bytes of data
 * @param ll pointer to struct sockaddr_ll
 * 
 * @return num. bytes read, no msg will return 0, error with -1 (check errno for details if -1)
*/
int get_ip_packet(int fd, uint8_t *buffer, int len, struct sockaddr_ll* ll);

/**
 * send an ip packet, blocking
 * 
 * @param fd the raw sock file descriptor
 * @param buffer the buffer(uint8_t*) to store the packet 
 * @param len of the packet
 * @param ll pointer to struct sockaddr_ll (assumed ll is the one returned from get_ip_packet, which contains send info)
 * 
 * @return num. bytes sent
*/
int send_ip_packet(int fd, uint8_t *buffer, int len, struct sockaddr_ll* ll);

#endif