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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <libbladeRF.h>
#include <math.h>
#include <sys/time.h>


int init_bladerf()
{
    struct bladerf_devinfo dev_info;

    bladerf_init_devinfo(&dev_info);

    return 0;
}

int init_tun(char* tun)
{
    char tun_path[100] = "/dev/";
    char syscall1[100];
    char syscall2[100];
    char syscall3[100];
    char syscall4[100];

    strcat(tun_path,tun);

    sprintf(syscall1, "ifconfig %s 10.10.10.1 10.10.10.255", tun);
    sprintf(syscall2, "ifconfig %s up", tun);
    sprintf(syscall3, "ifconfig %s", tun);
    sprintf(syscall4, "sudo route add 20.20.20/24 -interface %s", tun);


    int fd;
    if((fd = open(tun_path, O_RDWR)) == -1)
    {
            perror("open /dev/tun6");
            exit(1);
    }

    system(syscall1);
    system(syscall2);
    system(syscall3);
    system(syscall4);

    return fd;
}

char* bladerf_packet_to_tun_packet(char* bladerf_packet)
{
    return NULL;
}

char* tun_packet_to_bladerf_packet(char* tun_packet)
{
    return NULL;
}

char* create_bladerf_packet(char* payload)
{
    return NULL;
}

char * create_tun_packet(char* payload)
{
    return NULL;
}

void show_tun_packet(char* buffer)
{
    struct ip* packet = (struct ip*)buffer;

    printf("%s -> %s : %d\n", inet_ntoa(packet->ip_src), inet_ntoa(packet->ip_dst), ntohs(packet->ip_len));
}