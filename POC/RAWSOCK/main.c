#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <errno.h>
#include <linux/tcp.h>

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

    // 4500002965dc400040066c13c0a87aeaac4381091f9000500000007b00000000500060b284f7000041
    // 4500002965dc400040066c13c0a87aeaac4381091f9000500000007b00000000500060b284f7000041 (wireshark)

    return ~((uint16_t)sum);
}


int 
main() {

    struct sockaddr_in src_addr, dst_addr;

    int packet_size;

    uint8_t buffer[MAX_BUF];
    buffer[40] = 65;

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    // int sock = socket(AF_PACKET, SOCK_RAW, ntohs(ETH_P_ALL));
    if (sock == -1)
    {
        perror("Fail to create socket");
        exit(1);
    }

    int one = 1;
    const int *val = &one;
    if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {
        printf ("Error setting IP_HDRINCL. Error number : %d . Error message : %s \n" , errno , strerror(errno));
        exit(0);
    }

    struct iphdr *test_packet = (struct iphdr *)buffer;

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

    struct tcphdr *test_segment = (struct tcphdr *)(buffer+(test_packet->ihl*4));
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

    int n = sendto(sock, buffer, 41, 0, (struct sockaddr *)&dst_addr, sizeof(dst_addr));
    printf("total sent [%d]\n", n);
    // printf("tcp check sum: %u\n", tcp_csum(test_packet, test_segment, buffer+40));

    printf("header: \n");
    for (packet_size=0; packet_size < 41; packet_size++) {
        printf("%02x", ((uint8_t*)buffer)[packet_size]);
    }
    printf("\n");


    return 0;

    struct iphdr *ip_header = (struct iphdr *)buffer;
    struct tcphdr *tcp_header = (struct tcphdr *)buffer;

    while(1) {
        
        packet_size = recvfrom(sock, buffer, MAX_BUF, 0, NULL, NULL);
        if (packet_size == -1) {
            printf("Failed to get packets\n");
            return 1;
        }
        // point to tcp header
        tcp_header = (struct tcphdr *)(buffer+ip_header->ihl*4);

        // some filter on ip header
        if (ip_header->protocol != IPPROTO_TCP) {
            printf("non tcp\n\n");
            continue;
        }
        if (htons(tcp_header->source) != 80) {
            continue;
        }

        memset(&src_addr, 0, sizeof(src_addr));
        src_addr.sin_addr.s_addr = ip_header->saddr;
        memset(&dst_addr, 0, sizeof(dst_addr));
        dst_addr.sin_addr.s_addr = ip_header->daddr;

        printf("\n\nIncoming Packets: \n");
        printf("Packet Size (bytes): %d\n", ntohs(ip_header->tot_len));
        printf("Src addr: %s\n", (char*)inet_ntoa(src_addr.sin_addr));
        printf("Dst addr: %s\n", (char*)inet_ntoa(dst_addr.sin_addr));
        printf("Packet Size (bytes): %d\n", ntohs(ip_header->tot_len));
        printf("ID: [%d]\n", ntohs(ip_header->id));
        printf("checksum correct: %d\n", !ip_csum(ip_header));
        printf("src port: %d\n", ntohs(tcp_header->source));
        printf("dst port: %d\n", ntohs(tcp_header->dest));

        memset(buffer, 0, MAX_BUF);

    }

    return 0;

}


