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
#include <pthread.h>


#include "utils.c"


int main(int argc, char *argv[])
{

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


    init_bladerf(serial, TX_MODULE);
    init_tun(tun, ip, netmask, route);



    pthread_create(&tid1, NULL, do_TX, NULL);
    pthread_create(&tid2, NULL, do_RX, NULL);


    while(1);
}
