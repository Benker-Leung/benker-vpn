// gcc main.c -lnetlink

#define _GNU_SOURCE
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/eventfd.h>
// #include <net/if_arp.h>
// #include <sched.h>
// #include <limits.h>

#include <unistd.h> // for sleep
#include <errno.h>
#include <string.h> // strlcpy
#include <libnetlink.h>

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>



#define TUN_NAME "benker-vpn-tun"

// helper func
static int get_ctl_fd(void)
{
	int s_errno;
	int fd;

	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	s_errno = errno;
	fd = socket(PF_PACKET, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd >= 0)
		return fd;
	errno = s_errno;
	perror("Cannot create control socket");
	return -1;
}
// helper func
static int do_chflags(const char *dev, __u32 flags, __u32 mask)
{
	struct ifreq ifr;
	int fd;
	int err;

	strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	fd = get_ctl_fd();
	if (fd < 0)
		return -1;
	err = ioctl(fd, SIOCGIFFLAGS, &ifr);
	if (err) {
		perror("SIOCGIFFLAGS");
		close(fd);
		return -1;
	}
	if ((ifr.ifr_flags^flags)&mask) {
		ifr.ifr_flags &= ~mask;
		ifr.ifr_flags |= mask&flags;
		err = ioctl(fd, SIOCSIFFLAGS, &ifr);
		if (err)
			perror("SIOCSIFFLAGS");
	}
	close(fd);
	return err;
}

// equivalent to "ip tuntap add dev benker-vpn-tun mode tun"
int add_tun() {

    int fd;
    struct ifreq _ifr;
    struct ifreq *ifr = &_ifr;

    ifr->ifr_flags |= IFF_NO_PI | IFF_TUN ;
    strncpy(ifr->ifr_name, TUN_NAME, IFNAMSIZ);  // actual length is 15 (including NULL char)

#ifdef IFF_TUN_EXCL
	ifr->ifr_flags |= IFF_TUN_EXCL;
#endif

    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }
	if (ioctl(fd, TUNSETIFF, ifr)) {
		perror("ioctl(TUNSETIFF)");
		close(fd);
        return -1;
	}
    return fd;    
}

// equivalent to "ip link set dev tun0 up"
int set_int_up(const char *dev) {
    return do_chflags(dev, IFF_UP, IFF_UP);
}

// equivalent to "ip route add default via 10.0.0.1"
int set_default_route() {

	int ret;

	struct rtnl_handle rth = { .fd = -1 };
	if (rtnl_open(&rth, 0) < 0) {
		return -1;
	}

	struct {
		struct nlmsghdr	n;
		struct rtmsg r;
		char buf[4096];
	} req = { 0	};

	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL;
	req.n.nlmsg_type = RTM_NEWROUTE;
	req.r.rtm_family = AF_INET;
	req.r.rtm_table = RT_TABLE_MAIN;
	req.r.rtm_protocol = RTPROT_BOOT;
	req.r.rtm_scope = RT_SCOPE_UNIVERSE;
	req.r.rtm_type = RTN_UNICAST;
	req.r.rtm_dst_len = 0;

	__u32 tunip[2];
	__u8* data = &tunip;
	data[0] = 10;
	data[1] = 0;
	data[2] = 0;
	data[3] = 1;

	ret = addattr_l(&req.n, sizeof(req), RTA_GATEWAY, &tunip, 4);
	ret = rtnl_talk(&rth, &req.n, NULL);

	rtnl_close(&rth);
	return ret;
}


int main() {

    
    // create tun
    int tun_fd;
    if ((tun_fd = add_tun()) < 0) {
        printf("fail to add tun\n");
        exit(-1);
    }
    // set tun up
    if (set_int_up(TUN_NAME) < 0) {
        printf("fail to set dev up\n");
        exit(-1);
    }

	// sleep 10 seconds
	printf("set the ip addr\n");
	sleep(5);
	printf("assume already set ip addr, call ip route now\n");
	
	printf("default route %d\n", set_default_route());


    char buf[4096];
    int i, n;
    while (1) {
        n = read(tun_fd, buf, 4096);
        if (n < 0) {
            printf("n < 0\n");
            break;
        }
        printf("len [%d]\n", n);
        for (i=0; i<n; ++i) {
            printf("%02x", buf[i]);
        }
        printf("\n\n\n");
    }

}
