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


#define  NUMBER_OF_BUFFERS 16
#define  BUFFER_SIZE    8192*sizeof(int32_t)
#define  NUMBER_OF_TRANSFERS 8
#define  TIMEOUT_IN_MS 1000
#define  FREQUENCY_USED 713000000
#define  BANDWIDTH_USED 3000000
#define  SAMPLING_RATE_USED 600000
#define  PAYLOAD_LENGTH 1400


#define TX_MODULE 0
#define RX_MODULE 1


struct module_config {
    bladerf_module module;

    unsigned int frequency;
    unsigned int bandwidth;
    unsigned int samplerate;

    /* Gains */
    bladerf_lna_gain rx_lna;
    int vga1;
    int vga2;
};

int configure_module(struct bladerf *dev, struct module_config *c)
{
    int status;

    status = bladerf_set_frequency(dev, c->module, c->frequency);
    if (status != 0) {
        fprintf(stderr, "Failed to set frequency = %u: %s\n",
                c->frequency, bladerf_strerror(status));
        return status;
    }

    status = bladerf_set_sample_rate(dev, c->module, c->samplerate, NULL);
    if (status != 0) {
        fprintf(stderr, "Failed to set samplerate = %u: %s\n",
                c->samplerate, bladerf_strerror(status));
        return status;
    }

    status = bladerf_set_bandwidth(dev, c->module, c->bandwidth, NULL);
    if (status != 0) {
        fprintf(stderr, "Failed to set bandwidth = %u: %s\n",
                c->bandwidth, bladerf_strerror(status));
        return status;
    }

    switch (c->module) {
        case BLADERF_MODULE_RX:
            /* Configure the gains of the RX LNA, RX VGA1, and RX VGA2  */
            status = bladerf_set_lna_gain(dev, c->rx_lna);
            if (status != 0) {
                fprintf(stderr, "Failed to set RX LNA gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }

            status = bladerf_set_rxvga1(dev, c->vga1);
            if (status != 0) {
                fprintf(stderr, "Failed to set RX VGA1 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }

            status = bladerf_set_rxvga2(dev, c->vga2);
            if (status != 0) {
                fprintf(stderr, "Failed to set RX VGA2 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            break;

        case BLADERF_MODULE_TX:
            /* Configure the TX VGA1 and TX VGA2 gains */
            status = bladerf_set_txvga1(dev, c->vga1);
            if (status != 0) {
                fprintf(stderr, "Failed to set TX VGA1 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }

            status = bladerf_set_txvga2(dev, c->vga2);
            if (status != 0) {
                fprintf(stderr, "Failed to set TX VGA2 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            break;

        default:
            status = BLADERF_ERR_INVAL;
            fprintf(stderr, "%s: Invalid module specified (%d)\n",
                    __FUNCTION__, c->module);

    }


    return status;
}

int init_sync_rx(struct bladerf *dev)
{
    int status;
    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     *
     * It is important to remember that TX buffers will not be submitted to
     * the hardware until `buffer_size` samples are provided via the
     * bladerf_sync_tx call.  Similarly, samples will not be available to
     * RX via bladerf_sync_rx() until a block of `buffer_size` samples has been
     * received.
     */
    const unsigned int num_buffers   = NUMBER_OF_BUFFERS;
    const unsigned int buffer_size   = BUFFER_SIZE;  /* Must be a multiple of 1024 */
    const unsigned int num_transfers = NUMBER_OF_TRANSFERS;
    const unsigned int timeout_ms    = TIMEOUT_IN_MS;
    /* Configure both the device's RX and TX modules for use with the synchronous
     * interface. SC16 Q11 samples *without* metadata are used. */
    status = bladerf_sync_config(dev,
                                 BLADERF_MODULE_RX,
                                 BLADERF_FORMAT_SC16_Q11,
                                 num_buffers,
                                 buffer_size,
                                 num_transfers,
                                 timeout_ms);
    if (status != 0) {
        fprintf(stderr, "Failed to configure RX sync interface: %s\n", bladerf_strerror(status));
        return status;
    }
    status = bladerf_enable_module(dev, BLADERF_MODULE_RX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable RX module: %s\n", bladerf_strerror(status));
        return status;
    }
    return status;
}


int init_sync_tx(struct bladerf *dev)
{
    int status;
    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     *
     * It is important to remember that TX buffers will not be submitted to
     * the hardware until `buffer_size` samples are provided via the
     * bladerf_sync_tx call.  Similarly, samples will not be available to
     * RX via bladerf_sync_rx() until a block of `buffer_size` samples has been
     * received.
     */
    const unsigned int num_buffers   = NUMBER_OF_BUFFERS;
    const unsigned int buffer_size   = BUFFER_SIZE;  /* Must be a multiple of 1024 */
    const unsigned int num_transfers = NUMBER_OF_TRANSFERS;
    const unsigned int timeout_ms    = TIMEOUT_IN_MS;
    /* Configure both the device's RX and TX modules for use with the synchronous
     * interface. SC16 Q11 samples *without* metadata are used. */

    status = bladerf_sync_config(dev,
                                 BLADERF_MODULE_TX,
                                 BLADERF_FORMAT_SC16_Q11,
                                 num_buffers,
                                 buffer_size,
                                 num_transfers,
                                 timeout_ms);
    if (status != 0) {
        fprintf(stderr, "Failed to configure TX sync interface: %s\n",
                bladerf_strerror(status));
    }
    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable RX module: %s\n",
                bladerf_strerror(status));
        return status;
    }

    return status;
}




int calibrate(struct bladerf *dev)
{
	int status = 0 ;
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_LPF_TUNING);
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_TX_LPF);
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_RX_LPF);
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_RXVGA2);


	return status;
}







int init_bladerf(char* serial, int MODULE_TYPE)
{
    struct bladerf_devinfo dev_info;
    struct bladerf *dev = NULL;
    struct module_config config;
    int status;

    bladerf_init_devinfo(&dev_info);
    strncpy(dev_info.serial, serial, sizeof(dev_info.serial) - 1);

    status = bladerf_open_with_devinfo(&dev, &dev_info);
    if (status != 0)
    {
        fprintf(stderr, "Unable to open device: %s\n",
                bladerf_strerror(status));

        exit(1);
    }

    bladerf_load_fpga(dev, "./hostedx115-latest.rbf");
    

    if( MODULE_TYPE == TX_MODULE )
    {
        config.module     = BLADERF_MODULE_TX;
        config.frequency  = FREQUENCY_USED;
        config.bandwidth  = BANDWIDTH_USED;
        config.samplerate = SAMPLING_RATE_USED;
        config.vga1       = 10;
        config.vga2       = 0;
    }
    else
    {
        config.module     = BLADERF_MODULE_RX;
        config.frequency  = 910000000;
        config.bandwidth  = 2000000;
        config.samplerate = 300000;
        config.rx_lna     = BLADERF_LNA_GAIN_MAX;
        config.vga1       = 30;
        config.vga2       = 3;
    }

    status = configure_module(dev, &config);
    if (status != 0) {
        fprintf(stderr, "Failed to configure module. Exiting.\n");
        bladerf_close(dev);
        exit(1);
    }

    if( MODULE_TYPE == TX_MODULE )
    {
        init_sync_tx(dev);
    }
    else
    {
        init_sync_rx(dev);
    }


    calibrate(dev);

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
            perror("Cannot open");
            exit(1);
    }

    if(system(syscall1))
        exit(1);
    if(system(syscall2))
        exit(1);
    if(system(syscall3))
        exit(1);
    if(system(syscall4))
        exit(1);

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