#include "vpn_raw_sock.h"
#include <stdio.h>

int get_vpn_raw_socket()
{
    int fd, ret;
    struct sockaddr_ll ll;
    // SOCK_DGRAM (removed layer-2) SOCK_RAW (hv layer-2)
    if ((fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
        return -1;
    }
    memset(&ll, 0, sizeof(struct sockaddr_ll));
    ll.sll_family = AF_PACKET;
    ll.sll_protocol = IPPROTO_IP;
    ll.sll_ifindex = 2;
    if (bind(fd, (const struct sockaddr*)&ll, sizeof(struct sockaddr_ll))) {
        close(fd);
        return -1;
    }
    return fd;
}

int get_ip_frame(int fd, uint8_t *buffer, int len, struct sockaddr_ll* ll)
{
    int ret, sock_len, i;
    do {
        ret = recvfrom(fd, buffer, len, 0, (struct sockaddr*)ll, (socklen_t*)&sock_len);
    } while(ll->sll_pkttype != PACKET_HOST || ll->sll_halen <= 0);
    return ret;
}

int send_ip_frame(int fd, uint8_t *buffer, int len, struct sockaddr_ll* ll)
{
    ll->sll_hatype = 0;
    ll->sll_pkttype = 0;
    return sendto(fd, buffer, len, 0, (struct sockaddr*)ll, sizeof(struct sockaddr_ll));
}

