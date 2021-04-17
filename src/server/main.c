#include <stdlib.h>
#include <stdio.h>
#include "udp.h"
#include "vpn_raw_sock.h"
#include <stdlib.h>

#define MAX_BUF 3000
#define LOCAL_PORT 8080


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

    // int fd = get_udp_socket_server(LOCAL_PORT);
    // if (fd == -1) {
    //     perror("fail to get udp socket client");
    //     exit(-1);
    // }

    // char buf[MAX_BUF];
    // struct sockaddr_in raddr;
    // int ret;

    // while (1) {
    //     ret = read_msg_udp(fd, buf, MAX_BUF, &raddr);
    //     if (ret < 0) {
    //         perror("read_msg_udp");
    //         exit(-1);
    //     }
    //     printf("read %d bytes\n", ret);
    // }

    struct sockaddr_ll _ll;
    struct sockaddr_ll* ll = &_ll;
    int rawfd = get_vpn_raw_socket();
    if (rawfd < 0) {
        perror("fail to get raw socket fd");
        exit(-1);
    }

    int ret, i;
    uint8_t buf[4096];
    uint8_t send_buf[4096];

    while (1) {
        ret = get_ip_packet(rawfd, buf, 4096, ll);
        if (ret == 0) {
            continue;
        } else if (ret < 0) {
            perror("read err");
            exit(-1);
        }
        printf("read %d bytes\n", ret);
        for (i=0; i<ret; i++) {
            printf("%02X ", buf[i]);
        }
        printf("\n\n");
        // send_ip_packet(rawfd, send_buf, 40, ll);
        // exit(0);
    }


    return 0;
}