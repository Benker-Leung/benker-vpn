#include "tcp_session.h"

int handle_client_packet(uint8_t *buffer, int buffer_len, int *rst_size) {
    // TODO, remove the || 1
    if (validate_ip_tcp(buffer, buffer_len) == INVALID_IP_TCP || 1) {
        *rst_size = fillin_reset(buffer);
        return INVALID_SESSION;
    }
    // TODO
}


int handle_world_packet(uint8_t *buffer, int buffer_len, int *rst_size) {
    // TODO
    return INVALID_SESSION;
}




