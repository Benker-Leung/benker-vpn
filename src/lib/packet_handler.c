#include "packet_handler.h"

int handle_client_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, uint32_t vpn_server_ip, int *rst_size) {
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    if (validate_ip_tcp(buffer, buffer_len, &ip_header, &tcp_header) == INVALID_IP_TCP) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    struct tcp_session* session;
    // get session
    if (tcp_header->syn) {
        session = add_tcp_session(table, ntohl(ip_header->saddr), ntohs(tcp_header->source), 10);
    } else {
        session = get_tcp_session_by_client(table, ntohl(ip_header->saddr), ntohs(tcp_header->source));
    }
    if (session == NULL || session_expired(session)) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    // handle tcp flags
    if (tcp_header->syn && session->syn_state == 0) {
        session->client_last_state_seq = htonl(tcp_header->seq);
    }
    else {
        if (tcp_header->ack && htonl(tcp_header->ack_seq) > session->server_last_state_seq) {
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
                session->client_last_state_seq = htonl(tcp_header->seq);
            }
        }
    }

    // replace ip & port
    replace_src_ip_port(buffer, vpn_server_ip, session->server_port);
    return VALID_SESSION;
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


int handle_world_packet(struct session_hash_table* table, uint8_t *buffer, int buffer_len, int *rst_size, uint32_t *client_ip) {
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;
    if (validate_ip_tcp(buffer, buffer_len, &ip_header, &tcp_header) == INVALID_IP_TCP) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    struct tcp_session* session;
    // get session
    printf("Debug searching world, tcp->dest %d\n", ntohs(tcp_header->dest));
    session = get_tcp_session_by_server(table, ntohs(tcp_header->dest));
    if (session == NULL || session_expired(session)) {
        printf("session is nil %d\n", session == NULL);
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }

    // handle tcp flags
    if (tcp_header->syn && session->syn_state == 0 && tcp_header->ack && htonl(tcp_header->ack_seq) > session->client_last_state_seq) {
        session->syn_state = 2;
        session->server_last_state_seq = htonl(tcp_header->seq);
    }
    else {
        if (tcp_header->ack && session->server_fin_state == 1 && htonl(tcp_header->ack_seq) > session->client_last_state_seq) {
            session->server_fin_state = 2;
        }
        if (tcp_header->fin && session->client_fin_state == 0) {
            session->client_fin_state = 1;
            session->server_last_state_seq = htonl(tcp_header->seq);
        }
    }

    *client_ip = session->client_ip;
    // replace ip & port
    replace_dst_ip_port(buffer, session->client_ip, session->client_port);
    return VALID_SESSION;

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

        replace_src(packet, client_ip, session.client_port)

        return VALID_SESSION        

    */
}




