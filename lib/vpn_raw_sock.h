#ifndef __VPN_RAW_SOCK
#define __VPN_RAW_SOCK

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>

/**
 * get the raw socket with AF_PACKET level
 * 
 * @return the file descriptor if success, else -1 for errors
*/
int get_vpn_raw_socket();


/**
 * get an frame with protocol (0x0800) in L2 (blocking), if other frame(s) is/are received, it will ignore them, until a ip one
 * 
 * @param fd the raw sock file descriptor
 * @param buffer the buffer(uint8_t*) to store the packet 
 * @param len the max length of the buffer, e.g if len=10, means maximum 10 bytes of data
 * 
 * @return num. bytes read
*/
int get_ip_frame(int fd, uint8_t *buffer, int len);



#endif