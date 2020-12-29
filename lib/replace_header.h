#ifndef __REPLACE_HEADER_H
#define __REPLACE_HEADER_H

#include <string.h>
#include <netinet/ip.h>
#include <linux/tcp.h>

/**
 * replace the source port with given port, and the tcp checksum will be recalculated
 * 
 * @param buffer the buffer should start with and contains ip header, tcp header, tcp payload
 * @param the port to be put into, e.g "80" means port 80
 * 
*/
void replace_src_port(uint8_t *buffer, uint16_t port);

/**
 * replace the source ip addr with given new ip addr, and the ip checksum will be recalculated
 * 
 * @param buffer the buffer should start with ip header
 * @param the ip int number, e.g 1 means ip 0.0.0.1
 * 
*/
void replace_src_ip(uint8_t *buffer, uint32_t ip);

#endif