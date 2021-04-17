#include "protocol_headers.h"

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


int validate_ip_tcp(uint8_t *buffer, int buffer_size, struct iphdr** ip_header_ptr, struct tcphdr** tcp_header_ptr) {
    // ensure size at least fit ip header + tcp hedaer
    if (sizeof(struct iphdr) + sizeof(struct tcphdr) > buffer_size) {
        return INVALID_IP_TCP;
    }
    struct iphdr *ip_header = (struct iphdr*)buffer;
    // ensure it is a tcp packet
    if (ip_header->protocol != IPPROTO_TCP) {
        return INVALID_IP_TCP;
    }
    int ip_header_len = ip_header->ihl*4;
    // ensure size at least fit ip header + tcp hedaer
    if (ip_header_len + sizeof(struct tcphdr) > buffer_size) {
        return INVALID_IP_TCP;
    }
    // check total len
    if (ntohs(ip_header->tot_len) > buffer_size) {
        return INVALID_IP_TCP;
    }
    struct tcphdr *tcp_header = (struct tcphdr*)(buffer + ip_header_len);
    // check overall length
    if (ntohs(ip_header->tot_len) < ip_header_len + tcp_header->doff*4) {
        return INVALID_IP_TCP;
    }
    if (ip_header_ptr != NULL)
        *ip_header_ptr = ip_header;
    if (tcp_header_ptr != NULL)
        *tcp_header_ptr = tcp_header;
    return VALID_IP_TCP;
}

void replace_src_ip_port(uint8_t *buffer, uint32_t ip, uint16_t port) {
    replace_src_ip(buffer, ip);
    replace_src_port(buffer, port);
}

void replace_src_port(uint8_t *buffer, uint16_t port) {
    struct iphdr* ip_header = (struct iphdr*)buffer;
    int ip_header_size = ip_header->ihl*4;
    struct tcphdr* tcp_header = (struct tcphdr*)(buffer + ip_header_size);
    uint8_t* tcp_data = buffer + ip_header_size + tcp_header->doff*4;     // offset ip & tcp header
    tcp_header->source = htons(port);
    tcp_header->check = 0;
    tcp_header->check = htons(tcp_csum(ip_header, tcp_header, tcp_data));
}

void replace_src_ip(uint8_t *buffer, uint32_t src_ip) {
    struct iphdr* ip_header = (struct iphdr*)buffer;
    ip_header->check = 0;   // clear the check-sum
    ip_header->saddr = htonl(src_ip);
    ip_header->check = htons(ip_csum(ip_header));
}

int fillin_reset(uint8_t *buffer) {
    uint32_t temp32;
    uint16_t temp16;
    // prepare ip header
    struct iphdr* ip_header = (struct iphdr*)buffer;
    ip_header->ihl = sizeof(struct iphdr) >> 2;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
    ip_header->id += 1;
    ip_header->frag_off = 0;
    ip_header->ttl = 60;
    ip_header->check = 0;
    temp32 = ip_header->saddr;
    ip_header->saddr = ip_header->daddr;
    ip_header->daddr = temp32;
    ip_header->check = htons(ip_csum(ip_header));
    // prepare tcp header
    struct tcphdr* tcp_header = (struct tcphdr*)(buffer + ip_header->ihl*4);
    temp16 = tcp_header->source;
    tcp_header->source = tcp_header->dest;
    tcp_header->dest = temp16;
    temp32 = tcp_header->seq;
    tcp_header->seq = tcp_header->ack_seq;
    tcp_header->ack_seq = htonl(ntohl(temp32) + 1);
    tcp_header->doff = sizeof(struct tcphdr) >> 2;
    tcp_header->fin = 0;
    tcp_header->syn = 0;
    tcp_header->rst = 1;
    tcp_header->psh = 0;
    tcp_header->ack = 1;
    tcp_header->urg = 0;
    tcp_header->ece = 0;
    tcp_header->cwr = 0;
    tcp_header->check = 0;
    tcp_header->check = htons(tcp_csum(ip_header, tcp_header, NULL));
    return ntohs(ip_header->tot_len);
}

