#include <linux/if.h>
#include <linux/if_tun.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>


int tap_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if ( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
        return -1;

    memset(&ifr, 0  , sizeof(ifr));

    ifr.ifr_ifru.ifru_flags = IFF_TAP;
    if (*dev)
        strncpy(ifr.ifr_ifrn.ifrn_name, dev, IFNAMSIZ);

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
        close(fd);
        return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}

int main()
{
    char dev[] = "tap0";
    int fd = tap_alloc(dev);
    if (fd <= 0) {
        perror("fail create tap");
    }

    u_int8_t buf[2048];

    int i, n;
    while(1) {
        n = read(fd, buf, 2048);
        if (n < 0) {
            break;
        }
        printf("len [%d]\n", n);
        for (i=0; i<n; ++i) {
            printf("%02x", buf[i]);
            // printf("%d ", buf[i]);
        }
        printf("\n\n\n");
    }

    close(fd);
}

