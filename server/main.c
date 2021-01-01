#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "../lib/udp.h"
#include "../lib/tcp_session.h"
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <linux/if.h>

#define MAX_BUF 3000
#define LOCAL_PORT 8080

// Trial reset
int main() {
    // struct sockaddr_in src_addr, dst_addr;
    struct sockaddr_ll ll_addr;
    struct ifreq ifreq_c;
    memset(&ifreq_c, 0, sizeof(struct ifreq));
    strncpy(ifreq_c.ifr_ifru.ifru_newname, "eth0", IFNAMSIZ-1);
    memset(&ll_addr, 0, sizeof(struct sockaddr_ll));
    ll_addr.sll_family = AF_PACKET;
    ll_addr.sll_halen = ETH_ALEN;
    ll_addr.sll_ifindex = 2;
    ll_addr.sll_protocol = ETH_P_IP;

    uint8_t buffer[MAX_BUF];

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sock == -1) {
        perror("Fail to create socket");
        return(1);
    }

    struct ethhdr* ether_header;

    int frame_size;
    int temp;
    uint8_t tempChar[6];
    while (1) {
        frame_size = recvfrom(sock, buffer, MAX_BUF, 0, NULL, NULL);
        if (frame_size == -1) {
            printf("frame size -1\n");
            return(1);
        }
        ether_header = (struct ethhdr*)buffer;
        if (htons(ether_header->h_proto) == ETH_P_IP) {
            if (INVALID_SESSION == handle_client_packet(buffer+sizeof(struct ethhdr), frame_size, &temp)) {
                // swap mac addr
                memcpy(tempChar, ether_header->h_source, 6);
                memcpy(ether_header->h_source, ether_header->h_dest, 6);
                memcpy(ether_header->h_dest, tempChar, 6);
                // update
                memcpy(ll_addr.sll_addr, ether_header->h_dest, 6);
                printf("going to send %d bytes\n", sizeof(struct ethhdr)+temp);
                frame_size = sendto(sock, buffer, sizeof(struct ethhdr)+temp, 0, (struct sockaddr*)&ll_addr, sizeof(ll_addr));
                printf("sent %d\n", frame_size);
            }
        }
    }

}

int main2()
{
    struct sockaddr_in laddr;
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = INADDR_ANY;
    laddr.sin_port = htons(LOCAL_PORT);
    int fd = get_udp_socket_server(&laddr);
    
    struct sockaddr_in raddr;
    int total_read;
    memset(&raddr, 0, sizeof(struct sockaddr_in));
    unsigned char buffer[MAX_BUF];
    while (1) {
        sleep(10);
        if ((total_read = read_msg_udp(fd, buffer, MAX_BUF, &raddr)) >= 0) {
            printf("dst ip: %s\n", inet_ntoa(raddr.sin_addr));
            printf("dst port: %d\n", ntohs(raddr.sin_port));
            printf("msg read in bytes: %d\n", total_read);
            printf("\n");
            if (total_read > 10) {
                close(fd);
            }
        } else {
            perror("read less than 0");
            break;
        }
    }

    return 0;
}