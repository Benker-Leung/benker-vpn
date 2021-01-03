#include "packet_handler.h"

int handle_client_packet(uint8_t *buffer, int buffer_len, int *rst_size) {
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    // TODO, remove the || 1
    if (validate_ip_tcp(buffer, buffer_len, &ip_header, &tcp_header) == INVALID_IP_TCP || 1) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }
    // TODO
    /*
        // pseudocode for the handling logic

        if new_session(packet):
            session = create_session(packet)
        else:
            session = get_session(packet)

        if session_expire(session):
            delete_session(hash_table, session)
            return  INVALID_SESSION
        
        handle_flag(packet, session)    // update the session info, and expire time

        replace_src(packet, server_ip, session.server_port)

        return VALID_SESSION        

    */

    
}


int handle_world_packet(uint8_t *buffer, int buffer_len, int *rst_size) {
    // TODO
    return INVALID_SESSION;
}




