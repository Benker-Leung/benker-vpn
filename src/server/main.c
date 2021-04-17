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

    // buffer for packets
    uint8_t buf[MAX_BUF];
    int buf_len;
    // udp socket related
    struct sockaddr_in raddr;   // for reading/sending via udp-socket
    int udpfd;
    // raw socket related
    struct sockaddr_ll ll;      // for reading/sending via raw-socket
    int rawfd;
    uint32_t local_ip;
    // mapping related
    struct session_hash_table table;

};

void init_vpn_server(struct vpn_server* s) {
    init_hash_table(&s->table, 200, 40000, 41000);
    s->buf_len = MAX_BUF;
    s->local_ip = 1;                                // TODO: update to dynamic search
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
        // TODO: handle the return value

        // handle the ip packet (replace the src ip + port)
        read_len = ret;
        ret = handle_client_packet(&s.table, s.buf, s.buf_len, s.local_ip, &reset_packet_size);
        // TODO: handle the return value

        // send the handled ip packet
        // TODO: fill in the ll before sending packet
        ret = send_ip_packet(s.rawfd, s.buf, read_len, &s.ll);

        // get ip packet from real world
        ret = get_ip_packet(s.rawfd, s.buf, s.buf_len, &s.ll);
        // TODO: handle the return value

        read_len = ret;
        ret = handle_world_packet(&s.table, s.buf, s.buf_len, &reset_packet_size);
        // TOOD: handle the return value

        // send the handled ip packet
        // TODO: fill in the remote addr before sending
        ret = send_msg_udp(s.udpfd, s.buf, read_len, &s.raddr);


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