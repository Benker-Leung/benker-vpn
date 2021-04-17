#include <stdio.h>
#include "udp.h"
#include "ip_command.h"

#define MAX_BUF 3000
#define LOCAL_PORT 8080

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

   int ret;

    if (setup_tun()) {
        perror("fail to setup tunnel");
        exit(-1);
    }

    int fd = get_udp_socket_client();
    if (fd == -1) {
        perror("fail to get udp socket client");
        exit(-1);
    }

    char buf[MAX_BUF] = {100, 101, 102, 103, 0};
    struct sockaddr_in raddr;

    fill_port_ip(&raddr, 8080, "127.0.0.1");

    while (1) {
        ret = send_msg_udp(fd, buf, 5, &raddr);
        if (ret < 0) {
            perror("send_msg_udp");
            exit(-1);
        }
        printf("sent %d bytes\n", ret);
        sleep(5);
    }

    return 0;
}