#ifndef __SESSION_HANDLER_H
#define __SESSION_HANDLER_H

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "protocol_headers.h"
#include "session_struct.h"

#define VALID_SESSION 1
#define INVALID_SESSION 0
/**
 * handle the packet from vpn client (ip + tcp)
 * 
 * @param table the hash table for tcp sessions
 * @param buffer the pointer to the first byte of ip header
 * @param buffer_len the buffer size
 * @param vpn_server_ip the ip addr of vpn server used to place inside the packet
 * @param rst_size storing the number of bytes of "reset" packet in case of session problems (wrong packet, no such session etc)
 * 
 * @return VALID_SESSION (1) means successfully replaced the packet with source port & source ip
 *         INVALID_SESSION (0), which the buffer will be filled with "reset" packet, and rst_size will be set to the length of the reset packet
*/
int handle_client_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, uint32_t vpn_server_ip, int *rst_size);

/**
 * handle the packet from real world (ip + tcp)
 * 
 * @param table the hash table for tcp sessions
 * @param buffer the pointer to the first byte of ip header
 * @param len the buffer size
 * @param rst_size storing the number of bytes of "reset" packet in case of session problems (wrong packet, no such session etc)
 * @param client_ip storing the client ip addr
 * 
 * @return 1 (VALID_SESSION) means successfully replaced the packet with source port & source ip
 *         0 (INVALID_SESSION), which the buffer will be filled with "reset" packet, and rst_size will be set to the length of the reset packet
*/
int handle_world_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, int *rst_size, uint32_t *client_ip);



#endif