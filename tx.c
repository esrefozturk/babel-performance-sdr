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
    int tun_tx_fd;
    int nread;
    char serial[100];
    struct bladerf *dev_tx;
    flexframegenprops_s ffp;
    int i;




    flexframegen fg;
    unsigned char header[8];

    if(argc < 6)
    {
        perror("Usage: TBD");
        exit(1);
    }

    strcpy(serial,argv[1]);
    strcpy(tun,argv[2]);
    strcpy(ip,argv[3]);
    strcpy(netmask,argv[4]);
    strcpy(route,argv[5]);


    dev_tx = init_bladerf(serial, TX_MODULE);
    tun_tx_fd = init_tun(tun, ip, netmask, route);



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




        flexframegenprops_init_default(&ffp);

        ffp.fec0 = LIQUID_FEC_NONE;
        ffp.fec1 = LIQUID_FEC_NONE;
        ffp.mod_scheme = LIQUID_MODEM_QAM4;

        flexframegen fg = flexframegen_create(&ffp);
        


        for (i = 0; i < 8; i++)
            header[i] = i;




        transmit_bladerf_packet(fg, header, dev_tx, buffer);

    }


}
