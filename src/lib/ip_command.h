#ifndef __IP_COMMAND_H
#define __IP_COMMAND_H

#define _GNU_SOURCE
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


/**
 * set up the tun and return the file descriptor for receiving & sending packets
 * equivalent to the commands follow:
 * $> ip tuntap dev benker-vpn-tun mode tun
 * $> ip link set dev benker-vpn-tun up
 * $> ip addr add 10.0.0.1/24 benker-vpn-tun
 * $> ip route add default via 10.0.0.1
 * 
 * @return tunnel-fd if success, -1 if any parts fail
 */
int setup_tun();

#endif