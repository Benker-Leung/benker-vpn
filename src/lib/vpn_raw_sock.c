#include "vpn_raw_sock.h"

int get_vpn_raw_socket()
{
    return socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
}

int get_ip_frame(int fd, uint8_t *buffer, int len)
{
    return recvfrom(fd, buffer, len, 0, NULL, NULL);
}

