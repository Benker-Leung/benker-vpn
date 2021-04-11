#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "udp.h"
#include "ip_command.h"


#define MAX_BUF 3000
#define LOCAL_PORT 8080


// void
// test()
// {
//     struct epoll_event ev;
// }

int 
main()
{
    if (setup_tun()) {
        printf("fail to setup tun\n");
        return -1;
    }
    struct sockaddr_in laddr;
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = INADDR_ANY;
    laddr.sin_port = htons(LOCAL_PORT);
    int fd = get_udp_socket_server(&laddr);
    
    struct sockaddr_in raddr;
    int total_read;
    memset(&raddr, 0, sizeof(struct sockaddr_in));
    unsigned char buffer[MAX_BUF];
    while (1) {
        sleep(2);
        if ((total_read = read_msg_udp(fd, buffer, MAX_BUF, &raddr)) >= 0) {
            printf("dst ip: %s\n", inet_ntoa(raddr.sin_addr));
            printf("dst port: %d\n", ntohs(raddr.sin_port));
            printf("msg read in bytes: %d\n", total_read);
            printf("\n");
            if (total_read > 10) {
                close(fd);
            }
        } else {
            perror("read less than 0");
            break;
        }
    }

    return 0;
}