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
 * @param len the buffer size
 * @param rst_size storing the number of bytes of "reset" packet in case of session problems (wrong packet, no such session etc)
 * 
 * @return VALID_SESSION (1) means successfully replaced the packet with source port & source ip
 *         INVALID_SESSION (0), which the buffer will be filled with "reset" packet, and rst_size will be set to the length of the reset packet
*/
int handle_client_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, int *rst_size);

/**
 * handle the packet from real world (ip + tcp)
 * 
 * @param table the hash table for tcp sessions
 * @param buffer the pointer to the first byte of ip header
 * @param len the buffer size
 * @param rst_size storing the number of bytes of "reset" packet in case of session problems (wrong packet, no such session etc)
 * 
 * @return 1 (VALID_SESSION) means successfully replaced the packet with source port & source ip
 *         0 (INVALID_SESSION), which the buffer will be filled with "reset" packet, and rst_size will be set to the length of the reset packet
*/
int handle_world_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, int *rst_size);



#endif