#include "udp.h"


int get_udp_socket_client()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}

int get_udp_socket_server(struct sockaddr_in* laddr)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) 
        return -1;
    if (bind(fd, (const struct sockaddr*)laddr, sizeof(struct sockaddr_in)) != 0)
        return -1;
    return fd;
}

int send_msg_udp(int fd, unsigned char* buffer, int size, struct sockaddr_in* dst)
{
    return sendto(fd, buffer, size, MSG_CONFIRM, (const struct sockaddr*)dst, sizeof(struct sockaddr_in));
}

int read_msg_udp(int fd, unsigned char* buffer, int size, struct sockaddr_in* raddr)
{
    socklen_t len = sizeof(struct sockaddr_in);;
    return recvfrom(fd, buffer, size, MSG_WAITALL, (struct sockaddr*)raddr, &len);
}
