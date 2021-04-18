#include "vpn_raw_sock.h"

int get_vpn_raw_socket()
{
    int fd, ret;
    struct sockaddr_ll ll;
    // SOCK_DGRAM (removed layer-2) SOCK_RAW (hv layer-2)
    if ((fd = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP))) < 0) {
        return -1;
    }
    // bind to ip protocol, all intfc
    memset(&ll, 0, sizeof(struct sockaddr_ll));
    ll.sll_family = AF_PACKET;
    ll.sll_protocol = IPPROTO_IP;
    ll.sll_ifindex = 0;
    if (bind(fd, (const struct sockaddr*)&ll, sizeof(struct sockaddr_ll))) {
        close(fd);
        return -1;
    }
    return fd;
}

void print_sockaddr_ll(struct sockaddr_ll *ll) {
    printf("sll_family: %d\n", ll->sll_family);
    printf("sll_protocol: %d\n", ll->sll_protocol);
    printf("sll_ifindex: %d\n", ll->sll_ifindex);
    printf("sll_hatype: %d\n", ll->sll_hatype);
    printf("sll_pkttype: %d\n", ll->sll_pkttype);
    printf("sll_halen: %d\n", ll->sll_halen);
    int i;
    for (i=0; i<ll->sll_halen; ++i) {
        printf("%02x", ll->sll_addr[i]);
    }
    printf("\n\n");
}

int get_ip_packet(int fd, uint8_t *buffer, int len, struct sockaddr_ll* ll)
{
    struct sockaddr_ll _ll;
    if (!ll) {
        ll = &_ll;
    }
    int ret, sock_len, i;
    do {
        ret = recvfrom(fd, buffer, len, MSG_DONTWAIT, (struct sockaddr*)ll, (socklen_t*)&sock_len);
        if (ret < 0) {
            if (errno != EAGAIN || errno != EWOULDBLOCK) {
                return -1;
            } else {
                return 0;
            }
        }
    } while(ll->sll_pkttype != PACKET_HOST || ll->sll_halen <= 0);

    printf("Got ip pkt: \n");
    print_sockaddr_ll(ll);
    ll->sll_pkttype = 0;
    ll->sll_hatype = 0;
    return ret;
}

int send_ip_packet(int fd, uint8_t *buffer, int len, struct sockaddr_ll* ll)
{
    printf("Debug send using: \n");
    print_sockaddr_ll(ll);
    ll->sll_hatype = 0;
    ll->sll_pkttype = 0;
    return sendto(fd, buffer, len, 0, (struct sockaddr*)ll, sizeof(struct sockaddr_ll));
}

