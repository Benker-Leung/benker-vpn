#include <stdio.h>
#include "udp.h"

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

    int fd = get_udp_socket_server(LOCAL_PORT);
    if (fd == -1) {
        perror("fail to get udp socket client");
        exit(-1);
    }

    char buf[MAX_BUF];
    struct sockaddr_in raddr;
    int ret;

    while (1) {
        ret = read_msg_udp(fd, buf, MAX_BUF, &raddr);
        if (ret < 0) {
            perror("read_msg_udp");
            exit(-1);
        }
        printf("read %d bytes\n", ret);
    }

    return 0;
}