#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>

#include "udp.h"
#include "vpn_raw_sock.h"
#include "protocol_headers.h"
#include "session_struct.h"
#include "packet_handler.h"

#define MAX_BUF 9000            // for buffer size
#define LOCAL_PORT 8080         // for udp server port listen


struct vpn_server {

    // vpn server info
    uint32_t local_ip;
    unsigned char mac_addr[6];
    int net_dev_ind;
    uint32_t client_ip;
    // buffer for packets
    uint8_t buf[MAX_BUF];
    int buf_len;
    // udp socket related
    struct sockaddr_in raddr;   // for reading/sending via udp-socket
    int udpfd;
    // raw socket related
    struct sockaddr_ll ll;      // for reading/sending via raw-socket
    int rawfd;
    // mapping related
    struct session_hash_table table;

};

void init_vpn_server(struct vpn_server* s) {

    // 192.168.122.234
    s->local_ip = 3232266986;                       // TODO: update to dynamic search
    // 52:54:00:48:1c:20
    s->mac_addr[0] = 82; s->mac_addr[1] = 84; s->mac_addr[2] = 0; s->mac_addr[3] = 72; s->mac_addr[4] = 28; s->mac_addr[5] = 32;
    s->net_dev_ind = 2;                             // TODO: update to dynamic search

    // init struct sockaddr_ll
    s->ll.sll_family = AF_PACKET;
    s->ll.sll_ifindex = s->net_dev_ind;
    s->ll.sll_protocol = IPPROTO_IP;
    s->ll.sll_halen = 6;
    memcpy(s->ll.sll_addr, s->mac_addr, 6);

    init_hash_table(&s->table, 200, 40000, 41000);
    s->buf_len = MAX_BUF;
    int ret;
    ret = get_udp_socket_server(LOCAL_PORT);
    if (ret < 0) {
        perror("fail udp socket");
        exit(-1);
    }
    s->udpfd = ret;
    ret = get_vpn_raw_socket();
    if (ret < 0) {
        perror("fail raw socket");
        exit(-1);
    }
    s->rawfd = ret;
}

int main()
{

    /*

        main logic:

        receive: (from vpn client)
        1. update the src-ip & port from vpn-client -> vpn-server (re-calculate the checksums)
        2. put the packet to raw socket
        
        receive: (from outside world)
        1. change the dst-ip & port from vpn-server -> vpn-client (re-calculate the checksums)
        2. put the packet to udp payload and send to vpn-client
    
    */

    int ret, i, reset_packet_size, read_len;
    struct vpn_server s;
    init_vpn_server(&s);

    while (1) {
        
        // get ip packet from udp
        ret = read_msg_udp(s.udpfd, s.buf, s.buf_len, &s.raddr);
        if (ret < 0) {
            perror("udp read");
            exit(-1);
        } else if (ret == 0) {
            goto NEXT;
        }
        printf("udp read %d bytes\n", ret);

        // handle the ip packet (replace the src ip + port)
        read_len = ret;
        ret = handle_client_packet(&s.table, s.buf, s.buf_len, s.local_ip, &reset_packet_size);
        if (ret == INVALID_SESSION) {
            goto NEXT;
        }
        printf("handled 1 client packet\n");

        // send the handled ip packet
        ret = send_ip_packet(s.rawfd, s.buf, read_len, &s.ll);
        if (ret < 0) {
            perror("udp send");
            exit(-1);
        }
        printf("sent client packet %d bytes to real world\n", ret);

NEXT:
        // get ip packet from real world
        ret = get_ip_packet(s.rawfd, s.buf, s.buf_len, NULL);
        if (ret < 0) {
            perror("raw read");
            exit(-1);
        } else if (ret == 0) {
            continue;
        }
        printf("raw read %d bytes\n", ret);

        read_len = ret;
        ret = handle_world_packet(&s.table, s.buf, s.buf_len, &reset_packet_size, &s.client_ip);
        if (ret == INVALID_SESSION) {
            continue;
        }
        printf("handled 1 real-world packet\n");

        // send the handled ip packet
        // assume only 1 client
        ret = send_msg_udp(s.udpfd, s.buf, read_len, &s.raddr);
        if (ret < 0) {
            perror("raw send");
            exit(-1);
        }
        printf("sent real-world packet %d bytes to client\n", ret);

        // // code for trial
        // // get ip packet from raw
        // ret = get_ip_packet(s.rawfd, s.buf, s.buf_len, &s.ll);
        // if (ret == 0) {
        //     continue;
        // } else if (ret < 0) {
        //     perror("read err");
        //     exit(-1);
        // }
        // printf("read %d bytes\n", ret);
        // replace_src_ip_port(s.buf, 1, 22);
        // ret = send_ip_packet(s.rawfd, s.buf, ret, &s.ll);
        // if (ret < 0) {
        //     perror("write err");
        //     exit(-1);
        // }
        // printf("sent %d bytes\n", ret);
    }


    return 0;
}