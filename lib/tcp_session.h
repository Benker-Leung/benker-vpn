#ifndef __TCP_SESSION_H
#define __TCP_SESSION_H

#include <stdint.h>
#include <time.h>
#include "protocol_headers.h"

// tcp_session describe a tcp session for vpn client & real world, be aware of the endian issue
struct tcp_session {
    uint32_t client_ip;     // client ip address, 1 means 0.0.0.1
    uint16_t client_port;   // client port, 80 means port 80
    uint16_t server_port;   // server port, 80 means port 80
    uint32_t last_state_seq;// last state (SYN or FIN) packet's sequence number, 123 means 123
    uint8_t state;
    /*
        state:
        0   --> 1st SYN received
        1   --> 1st SYN ACK received
        2   --> 2nd SYN received
        3   --> 2nd SYN ACK received
        4   --> 1st FIN received
        5   --> 1st FIN ACK received
        6   --> 2nd FIN received
        7   --> 2nd FIN ACK received
    */
    time_t expire;           // this session expires if difftime(now, expire) gives positive
};

#define VALID_SESSION 1
#define INVALID_SESSION 0
/**
 * handle the packet from vpn client (ip + tcp)
 * 
 * @param buffer the pointer to the first byte of ip header
 * @param len the buffer size
 * @param rst_size storing the number of bytes of "reset" packet in case of session problems (wrong packet, no such session etc)
 * 
 * @return VALID_SESSION (1) means successfully replaced the packet with source port & source ip
 *         INVALID_SESSION (0), which the buffer will be filled with "reset" packet, and rst_size will be set to the length of the reset packet
*/
int handle_client_packet(uint8_t *buffer, int buffer_len, int *rst_size);

/**
 * handle the packet from real world (ip + tcp)
 * 
 * @param buffer the pointer to the first byte of ip header
 * @param len the buffer size
 * @param rst_size storing the number of bytes of "reset" packet in case of session problems (wrong packet, no such session etc)
 * 
 * @return 1 (VALID_SESSION) means successfully replaced the packet with source port & source ip
 *         0 (INVALID_SESSION), which the buffer will be filled with "reset" packet, and rst_size will be set to the length of the reset packet
*/
int handle_world_packet(uint8_t *buffer, int buffer_len, int *rst_size);



#endif