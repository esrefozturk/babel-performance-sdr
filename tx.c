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


int main(int argc, char *argv[])
{
    char tun[100];
    char buffer[1500];
    int tun_tx_fd;
    int nread;

    if(argc < 3)
    {
        perror("Usage: TBD");
        exit(1);
    }

    strcpy(tun,argv[2]);

    tun_tx_fd = init_tun(tun);

    while(1)
    {
        memset(buffer, 0, sizeof(buffer));
        nread = read(tun_tx_fd, buffer, sizeof(buffer));
        if (nread < 0) {
            perror("Reading from interface");
            close(tun_tx_fd);
            exit(1);
        }
        show_tun_packet(buffer);


    }


}
