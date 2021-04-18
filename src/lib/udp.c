#include "udp.h"


int get_udp_socket_client()
{
    return socket(AF_INET, SOCK_DGRAM, 0);
}

int get_udp_socket_server(int port)
{
    struct sockaddr_in laddr;
    laddr.sin_family = AF_INET;
    laddr.sin_addr.s_addr = INADDR_ANY;
    laddr.sin_port = htons(port);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) 
        return -1;
    if (bind(fd, (const struct sockaddr*)&laddr, sizeof(struct sockaddr_in)) != 0)
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
    int ret = recvfrom(fd, buffer, size, MSG_DONTWAIT, (struct sockaddr*)raddr, &len);
    if (ret < 0) {
        if (errno != EAGAIN || errno != EWOULDBLOCK) {
            return -1;
        } else {
            return 0;
        }
    }
    return ret;
}

int fill_port_ip(struct sockaddr_in* addr, int port, const char* ip)
{
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    return inet_aton(ip, &addr->sin_addr);
}
