#include "lwip/inet.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/init.h"
#include "lwip/stats.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "netif/etharp.h"
#include <string.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "enc28j60.h"

#include "port_common.h"
#include "lwip/apps/lwiperf.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT ENC28J60_SPI_PORT
#define PIN_MISO ENC28J60_SPI_MISO
#define PIN_CS ENC28J60_SPI_CS
#define PIN_SCK ENC28J60_SPI_SCK
#define PIN_MOSI ENC28J60_SPI_MOSI

// based on example from: https://www.nongnu.org/lwip/2_0_x/group__lwip__nosys.html
#define ETHERNET_MTU 1500

uint8_t mac[6] = {0xAA, 0x6F, 0x77, 0x47, 0x75, 0x8C};

#if 1
// 20230908 taylor
static void set_clock_khz(void);
#endif

static err_t netif_output(struct netif *netif, struct pbuf *p)
{
    LINK_STATS_INC(link.xmit);

    // lock_interrupts();
    // pbuf_copy_partial(p, mac_send_buffer, p->tot_len, 0);
    /* Start MAC transmit here */

    enc28j60PacketSend(p->len, (uint8_t *)p->payload);
    // pbuf_free(p);

    // error sending
    if (enc28j60Read(ESTAT) & ESTAT_TXABRT)
    {
        // a seven-byte transmit status vector will be
        // written to the location pointed to by ETXND + 1,
        printf("ERR - transmit aborted\n");
    }

    if (enc28j60Read(EIR) & EIR_TXERIF)
    {
        printf("ERR - transmit interrupt flag set\n");
    }

    // unlock_interrupts();
    return ERR_OK;
}

static void netif_status_callback(struct netif *netif)
{
    printf("netif status changed %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
}

static err_t netif_initialize(struct netif *netif)
{
    netif->linkoutput = netif_output;
    netif->output = etharp_output;
    // netif->output_ip6 = ethip6_output;
    netif->mtu = ETHERNET_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
    // MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100000000);
    SMEMCPY(netif->hwaddr, mac, sizeof(netif->hwaddr));
    netif->hwaddr_len = sizeof(netif->hwaddr);
    return ERR_OK;
}

void main(void)
{
    set_clock_khz();

    stdio_init_all();

    // data sheet up to 20 mhz
    uint32_t clk = spi_init(SPI_PORT, SPI_CLK_KHZ);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // END PICO INIT

    sleep_ms(1000 * 3);

    printf("Compiled @ %s %s\r\n", __DATE__, __TIME__);

    #if 1
    // 20230914 taylor
    printf("Set ENC28J60\r\n");
    printf("10Mbps\r\n");
    #ifdef USE_SPI_DMA
    printf("DMA\r\n");
    #else
    printf("nDMA\r\n");
    #endif
    printf("\r\n");
    #endif

    printf("SPI clk = %d\r\n", clk);

    ip_addr_t addr, mask, static_ip;
    IP4_ADDR(&static_ip, ENC28J60_IP0, ENC28J60_IP1, ENC28J60_IP2, ENC28J60_IP3);
    IP4_ADDR(&mask, 255, 255, 255, 0);
    IP4_ADDR(&addr, ENC28J60_IP0, ENC28J60_IP1, ENC28J60_IP2, 1);

    struct netif netif;
    lwip_init();
    // IP4_ADDR_ANY if using DHCP client
    netif_add(&netif, &static_ip, &mask, &addr, NULL, netif_initialize, netif_input);
    netif.name[0] = 'e';
    netif.name[1] = '0';
    // netif_create_ip6_linklocal_address(&netif, 1);
    // netif.ip6_autoconfig_enabled = 1;
    netif_set_status_callback(&netif, netif_status_callback);
    netif_set_up(&netif);

    enc28j60Init(mac);
    uint8_t *eth_pkt = malloc(ETHERNET_MTU);
    struct pbuf *p = NULL;

    netif_set_link_up(&netif);

    #if 1
    // 20230908 taylor
    // Start lwiperf server
    lwiperf_report_fn fn;
    lwiperf_start_tcp_server_default(fn, NULL);
    #endif

    while (1)
    {
        uint16_t packet_len = enc28j60PacketReceive(ETHERNET_MTU, (uint8_t *)eth_pkt);
        #if 0
        // 20230911 taylor
        
        if (packet_len != 0)
        {
            printf("206 enc: Received packet of length = %d\n", packet_len);
        }

        if (packet_len > 1000)
        #else
        if (packet_len)
        #endif
        {
            p = pbuf_alloc(PBUF_RAW, packet_len, PBUF_POOL);
            pbuf_take(p, eth_pkt, packet_len);
            free(eth_pkt);
            eth_pkt = malloc(ETHERNET_MTU);
        }
        else
        {
            // printf("enc: no packet received\n");
        }

        if (packet_len && p != NULL)
        {
            LINK_STATS_INC(link.recv);

            if (netif.input(p, &netif) != ERR_OK)
            {
                pbuf_free(p);
            }
        }

        /* Cyclic lwIP timers check */
        sys_check_timeouts();

        /* your application goes here */
        #if 1
        // 20230908 taylor
        #if 0
        sleep_ms(100);
        #endif
        #else
        sleep_ms(100);
        #endif
    }
}

/* Clock */
static void set_clock_khz(void)
{
    // set a system clock frequency in khz
    set_sys_clock_khz(PLL_SYS_KHZ, true);

    // configure the specified clock
    clock_configure(
        clk_peri,
        0,                                                // No glitchless mux
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
        PLL_SYS_KHZ * 1000,                               // Input frequency
        PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
    );
}
