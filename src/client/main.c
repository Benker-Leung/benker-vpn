#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "udp.h"
#include "ip_command.h"


#define MAX_BUF 9000
#define LOCAL_PORT 8080

struct vpn_client {

    // buffer for packets
    uint8_t buf[MAX_BUF];
    int buf_len;
    // udp socket related
    struct sockaddr_in raddr;
    int udpfd;
    // tun socket related
    int tunfd;    

};

void init_vpn_client(struct vpn_client* client) {

    client->buf_len = MAX_BUF;
    fill_port_ip(&client->raddr, LOCAL_PORT, "127.0.0.1");
    client->udpfd = get_udp_socket_client();
    client->tunfd = setup_tun();

}

int main()
{

    /*
    
        main logic:

        send:
        1. setup tun, which redirect all packets to the tun intfc
        2. receive packets from tun, directly sent the whole packet to vpn server

        receive:
        1. receive a udp packet from vpn server
        2. directly put the whole packet to the tun intfc

    */

   struct vpn_client client;
   init_vpn_client(&client);
   int ret;

    while (1) {
        
        // get ip packet from tun
        ret = read(client.tunfd, client.buf, client.buf_len);
        if (ret < 0) {
            if (errno != EAGAIN || errno != EWOULDBLOCK) {
                perror("tun read");
                exit(-1);
            }
            goto NEXT;
        }
        printf("tun read %d bytes\n", ret);

        // send ip packet via udp socket
        ret = send_msg_udp(client.udpfd, client.buf, ret, &client.raddr);
        if (ret < 0) {
            perror("udp send");
            exit(-1);
        }
        printf("udp sent %d bytes\n", ret);

NEXT:
        // get ip packet from udp socket
        ret = read_msg_udp(client.udpfd, client.buf, client.buf_len, NULL);
        if (ret < 0) {
            perror("udp read");
            exit(-1);
        } else if (ret == 0) {
            continue;
        }
        printf("udp read %d bytes\n", ret);

        // put ip packet to tun socket
        ret = write(client.tunfd, client.buf, ret);
        if (ret < 0) {
            perror("tun send");
            exit(-1);
        }
        printf("tun sent %d bytes\n", ret);

    }

    // if (setup_tun()) {
    //     perror("fail to setup tunnel");
    //     exit(-1);
    // }

    // int fd = get_udp_socket_client();
    // if (fd == -1) {
    //     perror("fail to get udp socket client");
    //     exit(-1);
    // }

    // char buf[MAX_BUF] = {100, 101, 102, 103, 0};
    // struct sockaddr_in raddr;

    // fill_port_ip(&raddr, 8080, "127.0.0.1");

    // while (1) {
    //     ret = send_msg_udp(fd, buf, 5, &raddr);
    //     if (ret < 0) {
    //         perror("send_msg_udp");
    //         exit(-1);
    //     }
    //     printf("sent %d bytes\n", ret);
    //     sleep(5);
    // }

    // return 0;
}