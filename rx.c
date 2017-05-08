#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/if_ether.h>
#include <netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<netinet/udp.h>
#include<netinet/tcp.h>
#include<netinet/ip.h>
#include<netinet/if_ether.h>
#include<net/ethernet.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <libbladeRF.h>


#include "utils.c"

int main(int argc, char *argv[])
{
    char tun[100];
    char ip[100];
    char netmask[100];
    char route[100];
    char buffer[1500];

    int nread;
    char serial[100];
    struct bladerf *dev_rx;
    unsigned int frame_counter = 0;

    if(argc < 3)
    {
        perror("Usage: TBD");
        exit(1);
    }

    strcpy(serial,argv[1]);
    strcpy(tun,argv[2]);
    strcpy(ip,argv[3]);
    strcpy(netmask,argv[4]);
    strcpy(route,argv[5]);

    dev_rx = init_bladerf(serial, RX_MODULE);
    tun_rx_fd = init_tun(tun, ip, netmask, route);


    fs = flexframesync_create(receive_bladerf_packet, (void *) &frame_counter);
    sync_rx(dev_rx, &process_samples);
}
