#ifndef __SESSION_STRUCT_H
#define __SESSION_STRUCT_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

/*
    The functions here mainly just handle the operation of 
    1. add
    2. search
    3. clean expired session
*/

// tcp_session describe a tcp session for vpn client & real world, be aware of the endian issue
struct tcp_session {
    uint32_t client_ip;     // client ip address, 1 means 0.0.0.1, the ip of the vpn-client
    uint16_t client_port;   // client port, 80 means port 80, the port used in vpn-client for this tcp session
    uint16_t server_port;   // server port, 80 means port 80, the port used in vpn-server for this tcp session
    uint32_t client_last_state_seq; // last state (SYN or FIN) packet's sequence number, 123 means 123, if ack larger than this, then the state is acked
    uint32_t server_last_state_seq; // last state (SYN or FIN) packet's sequence number, 123 means 123, if ack larger than this, then the state is acked
    /*
        state:
        0   --> 1st SYN received
        1   --> 1st SYN ACK received
        2   --> 2nd SYN received
        3   --> 2nd SYN ACK received (session created)
    */
    uint8_t syn_state;
    /*
        state:
        1   --> FIN sent by server
        2   --> ACK received from client
    */
    uint8_t client_fin_state;
    /*
        state:
        1   --> FIN sent by client
        2   --> ACK received from server
    */
    uint8_t server_fin_state;
    time_t expire;           // this session expires if difftime(now, expire) gives positive
};

// simple session linked list
struct session_linked_list {
    struct tcp_session* session;            // pointer to the tcp session
    struct session_linked_list* next;       // pointer to the next linked list
};

// simple session hash table
struct session_hash_table {
    // the two list are storing same element, but key is different
    struct session_linked_list** list_client_key;       // array of pointers to session_linked_list (key: server_port)
    struct session_linked_list** list_server_key;       // array of pointers to session_linked_list (key: client_ip+client_port)
    int len;                                // total length of the list
    uint16_t min_port;                      // ports smaller than this will not be used
    uint16_t mod_num;                       // mod_num = (max_port - min_port) + 1
    uint32_t next_num;                      // next port to be used cal as: (next_num % mod_num) + min_port
};

/**
 * check if a session is expired
 * 
 * @param session pointer to the session
 * 
 * @return non-zero if expired, else 0
*/
int session_expired(struct tcp_session* session);

/**
 * print the session linked list, mainly for debug usage
 * 
 * @param head the pointer to the first element
*/
void print_session_linked_list(struct session_linked_list* head);

/**
 * print the session hash table, mainly for debug usage
 * 
 * @param table the pointer to the hash table
*/
void print_session_hash_table(struct session_hash_table* table);


/**
 * initialize hash table
 * 
 * @param hash_table pointer to the hash table
 * @param num_entry number of entries
 * @param min_port smallest avaiable port num
 * @param max_port largest avaiable port num
 * 
*/
void init_hash_table(struct session_hash_table* hash_table, int num_entry, uint16_t min_port, uint16_t max_port);

/**
 * delete the hash table, remove all entries (linked list), and free all memory allocated
 * 
 * @param hash_table pointer to the hash table
 * 
*/
void delete_hash_table(struct session_hash_table* hash_table);

/**
 * clean the expired session in the table (remove entry & free memory)
 * 
 * @param hash_table the pointer to the hash table
 * 
*/
void clean_expired_session(struct session_hash_table* hash_table);

/**
 * add a new tcp session (simply return if already exist)
 * default will have expire 1 minute
 * 
 * @param hash_table the pointer to the hash table
 * @param client_ip the client ip addr (1 means 0.0.0.1)
 * @param client_port the client port (80 means port 80)
 * @param default_timeout_in_second timeout after x second
 * 
 * @return tcp_session pointer to the session
 *         NULL if all ports used up
 * 
*/
struct tcp_session* add_tcp_session(struct session_hash_table* hash_table, uint32_t client_ip, uint16_t client_port, int default_timeout_in_second);

/**
 * find a tcp session from the table
 * 
 * @param hash_table the pointer to the hash table
 * @param client_ip the client ip addr (1 means 0.0.0.1)
 * @param client_port the client port (80 means port 80)
 * 
 * @return tcp_session pointer if found
 *         NULL if not exist
*/
struct tcp_session* get_tcp_session_by_client(struct session_hash_table* hash_table, uint32_t client_ip, uint16_t client_port);

/**
 * find a tcp session from the table
 * 
 * @param hash_table the pointer to the hash table
 * @param server_port the server port (80 means port 80)
 * 
 * @return tcp_session pointer if found
 *         NULL if not exist
*/
struct tcp_session* get_tcp_session_by_server(struct session_hash_table* hash_table, uint16_t server_port);

#endif