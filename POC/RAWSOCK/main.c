#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <errno.h>
#include <linux/tcp.h>
#include <netpacket/packet.h>
#include <linux/if.h>


#define MAX_BUF 4096

uint16_t
summing (uint8_t *buffer, int bytes, uint32_t sum)
{
    int i = 0;
    while (bytes > 1) {
        sum += buffer[i] << 8;
        sum += buffer[i+1];
        i += 2;
        bytes -= 2;
    }
    if (bytes == 1) {
        sum += buffer[i] << 8;
    }
    while(1) {
        if (sum <= 0xffff) {
            break;
        }
        sum = (sum >> 16) + (uint16_t)sum;
    }
    return sum;
}

uint16_t
ip_csum (struct iphdr *ip_header)
{
    uint16_t sum = summing((uint8_t*)ip_header, ip_header->ihl*4, 0);
    return ~sum;
}

uint16_t
tcp_csum(struct iphdr* ip_header, struct tcphdr* tcp_header, uint8_t* tcp_data) {

    uint8_t pseudo_header[12];
    uint32_t sum = 0;
    int tcp_data_bytes = ntohs(ip_header->tot_len) - ip_header->ihl*4 - tcp_header->doff*4;

    printf("tcp data bytes: %d\n", tcp_data_bytes);

    // ((uint8_t*)ip_header)+12
    memset(pseudo_header, 0, 12);
    memcpy(pseudo_header, ((uint8_t*)ip_header)+12, 4);
    memcpy(pseudo_header+4, ((uint8_t*)ip_header)+16, 4);
    pseudo_header[9] = IPPROTO_TCP;
    *((unsigned short*)(pseudo_header+10)) = htons(tcp_header->doff*4 + tcp_data_bytes); // tcp header + tcp data  in bytes

    sum = summing(pseudo_header, 12, 0);
    sum = summing((uint8_t*)tcp_header, tcp_header->doff*4, sum);
    if (tcp_data_bytes > 0)
        sum = summing(tcp_data, tcp_data_bytes, sum);

    return ~((uint16_t)sum);
}


int 
main() {

    struct sockaddr_in src_addr, dst_addr;
    struct sockaddr_ll ll_addr;
    struct ifreq ifreq_c;
    memset(&ifreq_c, 0, sizeof(struct ifreq));
    strncpy(ifreq_c.ifr_ifru.ifru_newname, "eth0", IFNAMSIZ-1);

    int packet_size;

    uint8_t buffer[MAX_BUF];
    buffer[40] = 65;

    // int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sock == -1)
    {
        perror("Fail to create socket");
        exit(1);
    }

    // int one = 1;
    // const int *val = &one;
    // if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    // {
    //     printf ("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n" , errno , strerror(errno));
    //     exit(0);
    // }

    struct ethhdr *test_frame = (struct ethhdr*)buffer;
    test_frame->h_proto = htons(ETH_P_IP);
    if (ether_aton_r("52:54:0:a:19:ff", (struct ether_addr*)test_frame->h_source) == NULL) {
        printf("invalid src mac addr\n");
        exit(EXIT_FAILURE);
    }
    if (ether_aton_r("52:54:0:48:1c:20", (struct ether_addr*)test_frame->h_dest) == NULL) {
        printf("invalid dst mac addr\n");
        exit(EXIT_FAILURE);
    }

    struct iphdr *test_packet = (struct iphdr *)(buffer+sizeof(struct ethhdr));

    test_packet->version = 0x4;    // version
    test_packet->ihl = 0x5;       // ip header length
    test_packet->tos = 0x00;       // type of service
    test_packet->tot_len = htons(41); // length
    test_packet->id = htons(0x65dc);      // id
    test_packet->frag_off = htons(0x4000);   // fragment flag, fragment offset
    test_packet->ttl = 64;       // time to live
    test_packet->protocol = 6;  // protocol, UDP (17), TCP (6)
    test_packet->check = 0;      // header checksum
    test_packet->saddr = inet_addr("192.168.122.234");   // src ip
    test_packet->daddr = inet_addr("172.67.129.9");   // dst ip
    test_packet->check = htons(ip_csum(test_packet));

    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(80);
    dst_addr.sin_addr.s_addr = inet_addr("172.67.129.9");

    struct tcphdr *test_segment = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + (test_packet->ihl*4));
    test_segment->source = htons(8080);
    test_segment->dest = htons(80);
    test_segment->seq = htonl(123);
    test_segment->ack_seq = htonl(0);
    test_segment->doff = 5;
    test_segment->cwr = 0;
    test_segment->ece = 0;
    test_segment->urg = 0;
    test_segment->ack = 0;
    test_segment->psh = 0;
    test_segment->rst = 0;
    test_segment->syn = 0;
    test_segment->fin = 0;
    test_segment->res1 = 0;
    test_segment->window = htons(24754);
    // test_segment->check = htons(0xc5f5);
    test_segment->check = 0;
    test_segment->urg_ptr = 0;
    test_segment->check = htons(tcp_csum(test_packet, test_segment, buffer+40));

    // int n = sendto(sock, buffer, 41, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));     // sending IP + TCP

    memset(&ll_addr, 0, sizeof(struct sockaddr_ll));
    ll_addr.sll_family = AF_PACKET;
    memcpy(ll_addr.sll_addr, test_frame->h_dest, 6);
    ll_addr.sll_halen = ETH_ALEN;
    ll_addr.sll_ifindex = 2;
    ll_addr.sll_protocol = ETH_P_IP;
    int n = sendto(sock, buffer, 57, 0, (struct sockaddr *)&ll_addr, sizeof(ll_addr));        // sending L2 + IP + TCP
    printf("total sent [%d]\n", n);
    // printf("tcp check sum: %u\n", tcp_csum(test_packet, test_segment, buffer+40));

    printf("header: \n");
    for (packet_size=0; packet_size < 57; packet_size++) {
        printf("%02x ", ((uint8_t*)buffer)[packet_size]);
    }
    printf("\n");


    return 0;

    struct ethhdr *ether_header;
    struct iphdr *ip_header;
    struct tcphdr *tcp_header;

    while(1) {
        
        packet_size = recvfrom(sock, buffer, MAX_BUF, 0, NULL, NULL);
        if (packet_size == -1) {
            printf("Failed to get packets\n");
            return 1;
        }

        ether_header = (struct ethhdr*)buffer;
        if (htons(ether_header->h_proto) == ETH_P_IP || 1) {

            int i;
            printf("header: \n");
            for (i=0; i < packet_size; i++) {
                printf("%02x", ((uint8_t*)buffer)[i]);
            }
            printf("\n");

            printf("src mac: %s\n", ether_ntoa((struct ether_addr*)ether_header->h_source));
            printf("dst mac: %s\n", ether_ntoa((struct ether_addr*)ether_header->h_dest));
            printf("protocol: 0x%04x\n", ntohs(ether_header->h_proto));
            printf("\n");
        }

        // 0180c2000000fe54000a19ff002642420300000000008000525400481c20000000008000525400481c2080020000140002000200 (code)
        // 000200010006fe54000a19ff0000000442420300000000008000525400481c20000000008000525400481c2080020000140002000200 (wireshark)

        // // point to tcp header
        // tcp_header = (struct tcphdr *)(buffer+ip_header->ihl*4);

        // // some filter on ip header
        // if (ip_header->protocol != IPPROTO_TCP) {
        //     printf("non tcp\n\n");
        //     continue;
        // }
        // if (htons(tcp_header->source) != 80) {
        //     continue;
        // }

        // memset(&src_addr, 0, sizeof(src_addr));
        // src_addr.sin_addr.s_addr = ip_header->saddr;
        // memset(&dst_addr, 0, sizeof(dst_addr));
        // dst_addr.sin_addr.s_addr = ip_header->daddr;

        // printf("\n\nIncoming Packets: \n");
        // printf("Packet Size (bytes): %d\n", ntohs(ip_header->tot_len));
        // printf("Src addr: %s\n", (char*)inet_ntoa(src_addr.sin_addr));
        // printf("Dst addr: %s\n", (char*)inet_ntoa(dst_addr.sin_addr));
        // printf("Packet Size (bytes): %d\n", ntohs(ip_header->tot_len));
        // printf("ID: [%d]\n", ntohs(ip_header->id));
        // printf("checksum correct: %d\n", !ip_csum(ip_header));
        // printf("src port: %d\n", ntohs(tcp_header->source));
        // printf("dst port: %d\n", ntohs(tcp_header->dest));

        // memset(buffer, 0, MAX_BUF);

    }

    return 0;

}


