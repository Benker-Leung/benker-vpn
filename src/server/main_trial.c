#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "udp.h"
#include "packet_handler.h"
#include "session_struct.h"
#include <netinet/ether.h>
#include <netpacket/packet.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdint.h>

#define MAX_BUF 3000
#define LOCAL_PORT 8080

// takes 2 character
uint8_t decode_to_bytes(char* str) {
    uint8_t result = 0;
    int i=0;
    while (i<2) {
        // printf("Debug char: %c\n", *str);
        switch (*str) {
            case '0':
                result += 0;
                break;
            case '1':
                result += 1;
                break;
            case '2':
                result += 2;
                break;
            case '3':
                result += 3;
                break;
            case '4':
                result += 4;
                break;
            case '5':
                result += 5;
                break;
            case '6':
                result += 6;
                break;
            case '7':
                result += 7;
                break;
            case '8':
                result += 8;
                break;
            case '9':
                result += 9;
                break;
            case 'a':
                result += 10;
                break;
            case 'b':
                result += 11;
                break;
            case 'c':
                result += 12;
                break;
            case 'd':
                result += 13;
                break;
            case 'e':
                result += 14;
                break;
            case 'f':
                result += 15;
                break;
        }
        i++; str++;
        if (i==1) {
            result <<= 4;
        }
    }
    return result;
}

// Trial tun
int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
        return -1;

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_TUN; 
    if( *dev )
        strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
        close(fd);
        return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}   

int main()
{
    /*
# run this for the first time
sudo ip tuntap add tun0 mode tun
# start the program
sudo ./server
# turn on tun0
sudo ip link set tun0 up
# set ip for the tun0
sudo ip addr add 10.10.1.2/24 dev tun0
# set arp for tun0
sudo arp -s 10.10.1.1 0a:0b:0c:0d:0e:0f -i tun0
# set default route via tun0
sudo ip route add 123.123.123.123 via 10.10.1.1 dev tun0
    */
    char dev[] = "tun0";
    int fd = tun_alloc(dev);
    if (fd <= 0) {
        perror("fail create tun");
    }

    struct session_hash_table table;
    int rst_size;
    init_hash_table(&table, 10, 10000, 10002);
    
    uint8_t buf[2048];
    uint8_t *payload = buf + 4;
    

    int i, n;
    while(1) {
        n = read(fd, buf, 2048);
        if (n < 2) {
            break;
        }
        n -= 4;
        printf("len [%d]\n", n);
        for (i=0; i<n; ++i) {
            printf("%02x", payload[i]);
        }
        if (handle_client_packet(&table, payload, n, &rst_size) == INVALID_SESSION) {
            printf("session invalid\n");
        } else {
            printf("session is valid\n");
        }
        printf("\n\n\n");

        print_session_hash_table(&table);
    }

    close(fd);
}



// Trial handle tcp flag
int main5() {
    struct session_hash_table table;
    init_hash_table(&table, 100, 32928, 33000);

    // initialize the packets (from syn to fin)
    // const char packet1[] = "4510003ce73b400040066479c0a80102ac43810980a00050c6510d3d00000000a002faf0f41e0000020405b40402080a3b0fda6a0000000001030307";
    // const char packet2[] = "4500003400004000370654cdac438109c0a80102005080a01a8f34f6c6510d3e8012ffffdc3d000002040578010104020103030a";
    // const char packet3[] = "45100028e73c40004006648cc0a80102ac43810980a00050c6510d3e1a8f34f7501001f61ae10000";
    // const char packet4[] = "4510002de73d400040066486c0a80102ac43810980a00050c6510d3e1a8f34f7501801f614e10000fff4fffd06";
    // const char packet5[] = "450000283159400037062380ac438109c0a80102005080a01a8f34f7c6510d43501000401c920000";
    // const char packet6[] = "45000164315a400037062243ac438109c0a80102005080a01a8f34f7c6510d435018004026fb0000485454502f312e31203430302042616420526571756573740d0a5365727665723a20636c6f7564666c6172650d0a446174653a2053756e2c203130204a616e20323032312031333a34363a343720474d540d0a436f6e74656e742d547970653a20746578742f68746d6c0d0a436f6e74656e742d4c656e6774683a203135350d0a436f6e6e656374696f6e3a20636c6f73650d0a43462d5241593a202d0d0a0d0a3c68746d6c3e0d0a3c686561643e3c7469746c653e3430302042616420526571756573743c2f7469746c653e3c2f686561643e0d0a3c626f64793e0d0a3c63656e7465723e3c68313e3430302042616420526571756573743c2f68313e3c2f63656e7465723e0d0a3c68723e3c63656e7465723e636c6f7564666c6172653c2f63656e7465723e0d0a3c2f626f64793e0d0a3c2f68746d6c3e0d0a";
    // const char packet7[] = "45100028e73e40004006648ac0a80102ac43810980a00050c6510d431a8f3633501001f419a20000";
    // const char packet8[] = "45000028315b40003706237eac438109c0a80102005080a01a8f3633c6510d43501100401b550000";
    // const char packet9[] = "45100028e73f400040066489c0a80102ac43810980a00050c6510d431a8f3634501101f5199f0000";
    // const char packet10[] = "45000028315c40003706237dac438109c0a80102005080a01a8f3634c6510d44501000401b540000";
    const char *packets[] = {
        /*0. client -> server SYN*/    "4510003ce73b400040066479c0a80102ac43810980a00050c6510d3d00000000a002faf0f41e0000020405b40402080a3b0fda6a0000000001030307",
        /*1. server -> client SYN+ACK*/    "4500003400004000370654cdac438109c0a80102005080a01a8f34f6c6510d3e8012ffffdc3d000002040578010104020103030a",
        /*2. client -> server ACK*/    "45100028e73c40004006648cc0a80102ac43810980a00050c6510d3e1a8f34f7501001f61ae10000",
        /*3. client -> server Normal*/    "4510002de73d400040066486c0a80102ac43810980a00050c6510d3e1a8f34f7501801f614e10000fff4fffd06",
        /*4. server -> client Normal*/    "450000283159400037062380ac438109c0a80102005080a01a8f34f7c6510d43501000401c920000",
        /*5. server -> client Normal*/    "45000164315a400037062243ac438109c0a80102005080a01a8f34f7c6510d435018004026fb0000485454502f312e31203430302042616420526571756573740d0a5365727665723a20636c6f7564666c6172650d0a446174653a2053756e2c203130204a616e20323032312031333a34363a343720474d540d0a436f6e74656e742d547970653a20746578742f68746d6c0d0a436f6e74656e742d4c656e6774683a203135350d0a436f6e6e656374696f6e3a20636c6f73650d0a43462d5241593a202d0d0a0d0a3c68746d6c3e0d0a3c686561643e3c7469746c653e3430302042616420526571756573743c2f7469746c653e3c2f686561643e0d0a3c626f64793e0d0a3c63656e7465723e3c68313e3430302042616420526571756573743c2f68313e3c2f63656e7465723e0d0a3c68723e3c63656e7465723e636c6f7564666c6172653c2f63656e7465723e0d0a3c2f626f64793e0d0a3c2f68746d6c3e0d0a",
        /*6. client -> server Normal*/    "45100028e73e40004006648ac0a80102ac43810980a00050c6510d431a8f3633501001f419a20000",
        /*7. server -> client FIN*/    "45000028315b40003706237eac438109c0a80102005080a01a8f3633c6510d43501100401b550000",
        /*8. client -> server FIN+ACK*/    "45100028e73f400040066489c0a80102ac43810980a00050c6510d431a8f3634501101f5199f0000",
        /*9. server -> client ACK*/    "45000028315c40003706237dac438109c0a80102005080a01a8f3634c6510d44501000401b540000"
    };

    uint8_t buffer[MAX_BUF];
    int rst_size, i;
    
    // first SYN
    char* pos = packets[0];
    for (i=0; i<strlen(packets[0])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_client_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for 1st SYN\n");
    }
    // second SYN
    pos = packets[1];
    for (i=0; i<strlen(packets[1])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_world_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for 2nd SYN\n");
    }
    // second SYN ACK
    pos = packets[2];
    for (i=0; i<strlen(packets[2])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_client_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for 2nd SYN ACK\n");
    }
    // normal data packets
    pos = packets[3];
    for (i=0; i<strlen(packets[3])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_client_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for normal data [%d]\n", 3);
    }
    pos = packets[4];
    for (i=0; i<strlen(packets[4])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_world_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for normal data [%d]\n", 4);
    }
    pos = packets[5];
    for (i=0; i<strlen(packets[5])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_world_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for normal data [%d]\n", 5);
    }
    pos = packets[6];
    for (i=0; i<strlen(packets[6])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_client_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for normal data [%d]\n", 6);
    }


    // 1st FIN
    pos = packets[7];
    for (i=0; i<strlen(packets[7])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_world_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for 1st FIN\n");
    }
    // 2nd FIN
    pos = packets[8];
    for (i=0; i<strlen(packets[8])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_client_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for 2nd FIN\n");
    }
    // final ACK
    pos = packets[9];
    for (i=0; i<strlen(packets[9])/2; i++) {
        buffer[i] = decode_to_bytes(pos);
        pos += 2;
    }
    if (handle_world_packet(&table, buffer, MAX_BUF, &rst_size) != VALID_SESSION) {
        printf("invalid session for final ACK\n");
    }

    // TODO: continue to test the handling

    print_session_hash_table(&table);

    delete_hash_table(&table);
    
    return 0;

}

// Trial session
int main4() {

    struct session_hash_table table;
    init_hash_table(&table, 100, 10000, 10010);

    // struct tcp_session* session = add_tcp_session(&table, 1, 1, 5);
    // struct tcp_session* result = get_tcp_session_by_client(&table, 1, 1);
    // printf("server port: %d, expire unix-time: %ld\n", result->server_port, result->expire);


    struct tcp_session* session;
    int i;
    for (i=0; i<10; i++) {
        session = add_tcp_session(&table, i, i, i);
        if (session == NULL) {
            printf("No available port\n");
            break;
        }
    }
    sleep(2);

    print_session_hash_table(&table);

    delete_hash_table(&table);
    
    return 0;
}

// Trial reset
int main3() {
    // struct sockaddr_in src_addr, dst_addr;
    struct sockaddr_ll ll_addr;
    struct ifreq ifreq_c;
    memset(&ifreq_c, 0, sizeof(struct ifreq));
    strncpy(ifreq_c.ifr_ifru.ifru_newname, "eth0", IFNAMSIZ-1);
    memset(&ll_addr, 0, sizeof(struct sockaddr_ll));
    ll_addr.sll_family = AF_PACKET;
    ll_addr.sll_halen = ETH_ALEN;
    ll_addr.sll_ifindex = 2;
    ll_addr.sll_protocol = ETH_P_IP;

    uint8_t buffer[MAX_BUF];

    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (sock == -1) {
        perror("Fail to create socket");
        return(1);
    }

    struct ethhdr* ether_header;

    int frame_size;
    int temp;
    uint8_t tempChar[6];
    while (1) {
        frame_size = recvfrom(sock, buffer, MAX_BUF, 0, NULL, NULL);
        if (frame_size == -1) {
            printf("frame size -1\n");
            return(1);
        }
        ether_header = (struct ethhdr*)buffer;
        if (htons(ether_header->h_proto) == ETH_P_IP) {
            if (INVALID_SESSION == handle_client_packet(NULL, buffer+sizeof(struct ethhdr), frame_size, &temp)) {
                // swap mac addr
                memcpy(tempChar, ether_header->h_source, 6);
                memcpy(ether_header->h_source, ether_header->h_dest, 6);
                memcpy(ether_header->h_dest, tempChar, 6);
                // update
                memcpy(ll_addr.sll_addr, ether_header->h_dest, 6);
                printf("going to send %lu bytes\n", sizeof(struct ethhdr)+temp);
                frame_size = sendto(sock, buffer, sizeof(struct ethhdr)+temp, 0, (struct sockaddr*)&ll_addr, sizeof(ll_addr));
                printf("sent %d\n", frame_size);
            }
        }
    }

}

int main2()
{
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
        sleep(10);
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