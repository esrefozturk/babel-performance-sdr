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

    strcpy(serial_rx,argv[1]);
    strcpy(tun_rx,argv[2]);
    strcpy(ip_rx,argv[3]);
    strcpy(netmask_rx,argv[4]);
    strcpy(route_rx,argv[5]);

    strcpy(serial_tx,argv[6]);
    strcpy(tun_tx,argv[7]);
    strcpy(ip_tx,argv[8]);
    strcpy(netmask_tx,argv[9]);
    strcpy(route_tx,argv[10]);




    dev_tx = init_bladerf(serial_tx, TX_MODULE);
    tun_tx_fd = init_tun(tun_tx, ip_tx, netmask_tx, route_tx);

    dev_rx = init_bladerf(serial_rx, RX_MODULE);
    tun_rx_fd = init_tun(tun_rx, ip_rx, netmask_rx, route_rx);




    pthread_create(&tid1, NULL, do_TX, NULL);

    pthread_create(&tid2, NULL, do_RX, NULL);



}
