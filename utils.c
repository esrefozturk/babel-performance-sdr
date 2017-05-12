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
#include <math.h>
#include <complex.h>
#include <libbladeRF.h>
#include <liquid/liquid.h>
#include <pthread.h>


#define  NUMBER_OF_BUFFERS 16
#define  SAMPLE_SET_SIZE 1024
#define  BUFFER_SIZE    SAMPLE_SET_SIZE*sizeof(int32_t)
#define  NUMBER_OF_TRANSFERS 8
#define  TIMEOUT_IN_MS 1000
#define  FREQUENCY_USED 713000000
#define  BANDWIDTH_USED 3000000
#define  SAMPLING_RATE_USED 600000
#define  PAYLOAD_LENGTH 200


#define TX_MODULE 0
#define RX_MODULE 1

pthread_t tid1;
pthread_t tid2;

char buffer[1500];



char tun[100];
char ip[100];
char netmask[100];

char serial[100];
int tun_fd;

unsigned char header[8];


struct bladerf *dev;


flexframegenprops_s ffp;
int i;
flexframegen fg;

int incoming_packet_count=0;





unsigned int frame_counter = 0;


flexframesync fs;




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


float complex * convert_sc16q11_to_comlexfloat 	( int16_t * in, int16_t inlen )
{

int i=0;
float complex * out = NULL;
out = (float complex *)malloc(inlen * sizeof(float complex));
if (out != NULL) {
for(i = 0; i < inlen ; i++){
out[i]= in[2*i]/2048.0 + in[2*i+1]/2048.0 * I;
}
}
return (float complex *)out;
}


int16_t * 		convert_comlexfloat_to_sc16q11 	( float complex *in, unsigned int  inlen  )
{
int i=0;
int16_t * out = NULL;
out = (int16_t *)malloc(inlen * 2 * sizeof(int16_t));
if (out != NULL) {
for(i = 0; i < inlen ; i++){
out[2*i]= round( crealf(in[i]) * 2048); // Since bladeRF uses Q4.11 complex 2048=2^11
out[2*i+1]= round( cimagf(in[i]) * 2048);
if ( out[2*i] > 2047  ) out[2*i]=2047;
if ( out[2*i] < -2048  ) out[2*i]=-2048;
if ( out[2*i+1] > 2047  ) out[2*i+1]=2047;
if ( out[2*i+1] < -2048  ) out[2*i+1]=-2048;
}
}
return (int16_t *)out;
}

int process_samples(int16_t *samples, unsigned int sample_length) {
    int status = 0;
    float complex
    *y = convert_sc16q11_to_comlexfloat(samples, sample_length);
    if (y != NULL) {
        for (int i = 0; i <= sample_length; i = i + 32)
            flexframesync_execute(fs, &y[i], 32);
        free(y);
    } else {
        status = BLADERF_ERR_MEM;
    }
    return status;
}



void show_tun_packet(char* buffer)
{
    struct ip* packet = (struct ip*)buffer;

    printf("%s -> %s : %d\n", inet_ntoa(packet->ip_src), inet_ntoa(packet->ip_dst), ntohs(packet->ip_len));
}





int sync_tx(struct bladerf *dev,int16_t *tx_samples, unsigned int samples_len)
{
    int status = 0;

    bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    struct bladerf_metadata meta;
    memset(&meta, 0, sizeof(meta));

    //   meta.flags = BLADERF_META_FLAG_TX_NOW;
    meta.flags = BLADERF_META_FLAG_TX_BURST_START;


    status = bladerf_sync_tx(dev, tx_samples, samples_len, &meta, 5000);
    if (meta.status != 0) {
        fprintf(stderr, "Failed to TX samples: %s\n",bladerf_strerror(meta.status));
    }
    if (meta.actual_count > 0 )
        fprintf(stdout, "Meta Flag Actual Count = %u\n", meta.actual_count );


    return status;
}


int sync_rx(struct bladerf *dev, int (*process_samples)(int16_t *, unsigned int))
{
    int status=0, ret;
    bool done = false;
    /* "User" samples buffers and their associated sizes, in units of samples.
     * Recall that one sample = two int16_t values. */
    int16_t *rx_samples = NULL;
    unsigned int samples_len = SAMPLE_SET_SIZE; /* May be any (reasonable) size */
    /* Allocate a buffer to store received samples in */
    rx_samples = malloc(samples_len * 2 * sizeof(int16_t));
    if (rx_samples == NULL) {
        fprintf(stdout, "malloc error: %s\n", bladerf_strerror(status));
        return BLADERF_ERR_MEM;
    }

    bladerf_enable_module(dev, BLADERF_MODULE_RX, true);
    struct bladerf_metadata meta;
    memset(&meta, 0, sizeof(meta));
    /* Retrieve the current timestamp */
    if ((status=bladerf_get_timestamp(dev, BLADERF_MODULE_RX, &meta.timestamp)) != 0) {
        fprintf(stderr,"Failed to get current RX timestamp: %s\n",bladerf_strerror(status));
    }
    else
    {
        printf("Current RX timestamp: 0x%016"PRIx64"\n", meta.timestamp);
    }

    meta.flags = BLADERF_META_FLAG_RX_NOW;

    while (status == 0) {
        /* Receive samples */
        status = bladerf_sync_rx(dev, rx_samples, samples_len, &meta, 5000);
        //fprintf(stdout, "Meta Flag Actual Count = %u\n", meta.actual_count );
        if (status == 0) {
            /* TODO Process these samples, and potentially produce a response to transmit */
            done = process_samples(rx_samples, meta.actual_count);
        } else {
            fprintf(stderr, "Failed to RX samples: %s\n", bladerf_strerror(status));
        }
    }
    if (status == 0) {
        /* Wait a few seconds for any remaining TX samples to finish
         * reaching the RF front-end */
        usleep(2000000);
    }
    out:
    ret = status;
    /* Free up our resources */
    free(rx_samples);
    return ret;

}








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







struct bladerf* init_bladerf(char* serial, int MODULE_TYPE)
{
    struct bladerf_devinfo dev_info;
    struct module_config config_rx;
    struct module_config config_tx;

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



        config_rx.module     = BLADERF_MODULE_TX;
        config_rx.frequency  = FREQUENCY_USED;
        config_rx.bandwidth  = BANDWIDTH_USED;
        config_rx.samplerate = SAMPLING_RATE_USED;

        config_rx.vga1       = 10;
        config_rx.vga2       = 0;

        config_tx.module     = BLADERF_MODULE_RX;
        config_tx.frequency  = FREQUENCY_USED;
        config_tx.bandwidth  = BANDWIDTH_USED;
        config_tx.samplerate = SAMPLING_RATE_USED;
        config_tx.vga1       = 10;
        config_tx.vga2       = 0;
        config_tx.rx_lna     = BLADERF_LNA_GAIN_MAX;


    status = configure_module(dev, &config_rx);
    if (status != 0) {
        fprintf(stderr, "Failed to configure module. Exiting.\n");
        bladerf_close(dev);
        exit(1);
    }

    status = configure_module(dev, &config_tx);
    if (status != 0) {
        fprintf(stderr, "Failed to configure module. Exiting.\n");
        bladerf_close(dev);
        exit(1);
    }


    init_sync_tx(dev);

    init_sync_rx(dev);



    calibrate(dev);

    return dev;
}

void init_tun(char* tun, char* ip, char* netmask)
{
    char tun_path[100] = "/dev/";
    char syscall1[100];
    char syscall2[100];
    char syscall3[100];


    strcat(tun_path,tun);

    sprintf(syscall1, "ifconfig %s %s %s", tun,ip,netmask);
    sprintf(syscall2, "ifconfig %s up", tun);
    sprintf(syscall3, "ifconfig %s", tun);



    printf("%s\n",syscall1);
    printf("%s\n",syscall2);
    printf("%s\n",syscall3);




    if((tun_fd = open(tun_path, O_RDWR)) == -1)
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


    return ;
}

int transmit_bladerf_packet(flexframegen fg, unsigned char header[8], struct bladerf* dev_tx, char* buffer)
{
    unsigned char payload[PAYLOAD_LENGTH] = {0};
    unsigned int symbol_len;
    int frame_complete = 0;
    int lastpos = 0;
    unsigned int buf_len = PAYLOAD_LENGTH;
    float complex buf[buf_len];
    float complex y[BUFFER_SIZE];
    unsigned int samples_len = SAMPLE_SET_SIZE;
    int status;
    int16_t *tx_samples;

    struct ip* packet = (struct ip*)buffer;
    header[0] = ntohs(packet->ip_len);

    memset(payload, 0x00, PAYLOAD_LENGTH);
    int i;
    for(i=0;i<ntohs(packet->ip_len);i++)
        payload[i] = buffer[i];
    memset(&payload[ntohs(packet->ip_len)], 0x00, PAYLOAD_LENGTH - ntohs(packet->ip_len));


    flexframegen_assemble(fg, header, payload, PAYLOAD_LENGTH);
    symbol_len = flexframegen_getframelen(fg);

    frame_complete = 0;
    lastpos = 0;
    while (!frame_complete) {
        frame_complete = flexframegen_write_samples(fg, buf, buf_len);
        memcpy(&y[lastpos], buf, buf_len * sizeof(float complex));
        lastpos = lastpos + buf_len;
    }

    samples_len = symbol_len;
    tx_samples = convert_comlexfloat_to_sc16q11(y, symbol_len);

    status = sync_tx(dev_tx, tx_samples, samples_len);

    return status;
}

static int receive_bladerf_packet(unsigned char *_header,
                                  int _header_valid,
                                  unsigned char *_payload,
                                  unsigned int _payload_len,
                                  int _payload_valid,
                                  framesyncstats_s _stats,
                                  void *_userdata)
{
    if (_header_valid)
    {

        struct ip* packet = (struct ip*)_payload;

        incoming_packet_count++;

        printf("%d %s %s\n",incoming_packet_count, inet_ntoa(packet->ip_src), inet_ntoa(packet->ip_dst));

        //printf("Received: %s -> %s : %d\n", inet_ntoa(packet->ip_src), inet_ntoa(packet->ip_dst), ntohs(packet->ip_len));
        write(tun_fd, (void*)_payload, _header[0]);
    }
    return 0;
}






void* do_TX(void *arg)
{

    int nread;

    int count = 0;
    while(1)
    {

        memset(buffer, 0, sizeof(buffer));
        nread = read(tun_fd, buffer, sizeof(buffer));
        if (nread < 0) {
            perror("Reading from interface");
            close(tun_fd);
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




        transmit_bladerf_packet(fg, header, dev, buffer);

    }
}

void* do_RX(void *arg)
{
    fs = flexframesync_create(receive_bladerf_packet, (void *) &frame_counter);
    sync_rx(dev, &process_samples);
    return NULL;
}