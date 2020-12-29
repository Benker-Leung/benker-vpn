#include "replace_header.h"

// TODO:
// void replace_src_port(uint8_t *buffer, uint16_t port);
// void replace_src_ip(uint8_t *buffer, uint32_t ip);

// not supposed to be called by outside
uint16_t summing (uint8_t *buffer, int bytes, uint32_t sum)
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

/**
 * it calculate the checksum of the ip header 
 * calling to hton(cs) required for putting it to buffer
 * 
 * @param ip_header struct of ip header (can cast from uint8_t*)
 * 
 * @return the value of ip checksum, hton(cs) required for putting it to buffer
 * 
*/
uint16_t ip_csum (struct iphdr *ip_header)
{
    uint16_t sum = summing((uint8_t*)ip_header, ip_header->ihl*4, 0);
    return ~sum;
}

/**
 * it calculate the checksum of the tcp header 
 * calling to hton(cs) required for putting it to buffer
 * 
 * @param ip_header struct of ip header (can cast from uint8_t*)
 * @param tcp_header struct of tcp header (can cast from uint8_t*)
 * @param tcp_data the buffer containing the data in tcp layer, the len of data will be inferred from the ip & tcp header
 * 
 * @return the value of ip checksum, hton(cs) required for putting it to buffer
 * 
*/
uint16_t tcp_csum(struct iphdr* ip_header, struct tcphdr* tcp_header, uint8_t* tcp_data) {

    uint8_t pseudo_header[12];
    uint32_t sum = 0;
    int tcp_data_bytes = ntohs(ip_header->tot_len) - ip_header->ihl*4 - tcp_header->doff*4;

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
