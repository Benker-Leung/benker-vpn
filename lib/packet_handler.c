#include "packet_handler.h"

int handle_client_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, int *rst_size) {
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    // TODO, remove the || 1
    if (validate_ip_tcp(buffer, buffer_len, &ip_header, &tcp_header) == INVALID_IP_TCP) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    struct tcp_session* session;
    // get session
    if (tcp_header->syn) {
        session = add_tcp_session(table, ntohl(ip_header->saddr), ntohs(tcp_header->source), 1);
    } else {
        session = get_tcp_session_by_client(table, ntohl(ip_header->saddr), ntohs(tcp_header->source));
    }
    if (session == NULL) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    // TODO: handle tcp flags (ignore the validation & update of expire first)
    if (tcp_header->syn && session->syn_state == 0) {

    }
    else {
        if (tcp_header->ack) {
            if (session->syn_state == 2) {
                session->syn_state = 3;
            }
            else if (session->client_fin_state == 1) {
                session->client_fin_state = 2;
            }
        }
        if (tcp_header->fin) {
            if (session->server_fin_state == 0) {
                session->server_fin_state = 1;
            }
        }
    }

    // replace ip & port
    // TODO: replace ip here
    replace_src_port(buffer, session->server_port);
    return VALID_SESSION;

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


int handle_world_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, int *rst_size) {
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    // TODO, remove the || 1
    if (validate_ip_tcp(buffer, buffer_len, &ip_header, &tcp_header) == INVALID_IP_TCP) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    struct tcp_session* session;
    // get session
    session = get_tcp_session_by_server(table, ntohs(tcp_header->dest));
    if (session == NULL) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    // TODO: handle tcp flags (ignore the validation & update of expire first)
    if (tcp_header->syn && tcp_header->ack && session->syn_state == 0) {
        session->syn_state = 2;
    }
    else {
        if (tcp_header->ack) {
            if (session->server_fin_state == 1) {
                session->server_fin_state = 2;
            }
        }
        if (tcp_header->fin) {
            if (session->client_fin_state == 0) {
                session->client_fin_state = 1;
            }
        }
    }

    // replace ip & port
    // TODO: replace ip here
    replace_src_port(buffer, session->server_port);
    return VALID_SESSION;

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




