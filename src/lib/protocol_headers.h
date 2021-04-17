#ifndef __REPLACE_HEADER_H
#define __REPLACE_HEADER_H

#include <string.h>
#include <netinet/ip.h>
#include <linux/tcp.h>

#define VALID_IP_TCP 1
#define INVALID_IP_TCP 0
/**
 * validate the ip header len, tcp type, tcp header len, tcp data len (checksum validation is NOT done)
 * 
 * @param buffer the buffer pointing to the first byte of the ip header
 * @param buffer_size the size of buffer in bytes
 * @param ip_header store the ip header if valid & it is not NULL
 * @param tcp_header store the tcp header if valid & it is not NULL
 * 
 * @return VALID_IP_TCP if it is valid
 *         INVALID_IP_TCP if not valid
*/
int validate_ip_tcp(uint8_t *buffer, int buffer_size, struct iphdr** ip_header, struct tcphdr** tcp_header);

/**
 * replace ip & port (just the combined version of the two function below)
 * 
 * @param buffer the buffer should start with and contains ip header, tcp header, tcp payload
 * @param ip the ip, e.g 1 means ip 0.0.0.1
 * @param the port to be put into, e.g "80" means port 80
 * 
 * 
*/
void replace_src_ip_port(uint8_t *buffer, uint32_t ip, uint16_t port);

/**
 * replace the source port with given port, and the tcp checksum will be recalculated
 * buffer assume to be correct in length inside the header fields (not responsible for checking the len etc)
 * 
 * @param buffer the buffer should start with and contains ip header, tcp header, tcp payload
 * @param the port to be put into, e.g "80" means port 80
 * 
 * 
*/
void replace_src_port(uint8_t *buffer, uint16_t port);

/**
 * replace the source ip addr with given new ip addr, and the ip checksum will be recalculated
 * buffer assume to be correct in length inside the header fields (not responsible for checking the len etc)
 * 
 * @param buffer the buffer should start with ip header
 * @param the ip int number, e.g 1 means ip 0.0.0.1
 * 
 * 
*/
void replace_src_ip(uint8_t *buffer, uint32_t ip);

/**
 * modify the buffer(original received packet) into an valid tcp "reset" packet (assume buffer size enough for ip+tcp header)
 * e.g received a tcp packet from vpn-client, and need to tell vpn-client to reset connection, then the received packet is passed to this function
 * the src and dst ip, port will be handled in this function
 * after filling the buffer with reset content, and directly send the buffer out (to the vpn-client in the example above)
 * 
 * @param buffer the packet received and to be turn to a reset packet
 * 
 * @return the len of the whole "reset" packet in bytes
 * 
*/
int fillin_reset(uint8_t *buffer);


#endif