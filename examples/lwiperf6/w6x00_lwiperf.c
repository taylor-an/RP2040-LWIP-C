/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "socket.h"
#include "w6x00_spi.h"
#include "w6x00_lwip.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#include "lwip/apps/lwiperf.h"
#include "lwip/etharp.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Socket */
#define SOCKET_MACRAW 0

/* Port */
#define PORT_LWIPERF 5001

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
extern uint8_t mac[6];
static ip_addr_t g_ip;
static ip_addr_t g_mask;
static ip_addr_t g_gateway;

/* LWIP */
struct netif g_netif;
lwiperf_report_fn fn;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int8_t retval = 0;
    uint8_t *pack = malloc(ETHERNET_MTU);
    uint16_t pack_len = 0;
    struct pbuf *p = NULL;

    // Initialize network configuration
    IP4_ADDR(&g_ip, W6100_IP0, W6100_IP1, W6100_IP2, W6100_IP3);
    IP4_ADDR(&g_mask, 255, 255, 255, 0);
    IP4_ADDR(&g_gateway, W6100_IP0, W6100_IP1, W6100_IP2, 1);

    set_clock_khz();

    // Initialize stdio after the clock change
    stdio_init_all();

    sleep_ms(1000 * 3); // wait for 3 seconds

    printf("Compiled @ %s %s\r\n", __DATE__, __TIME__);

    printf("Set W6100\r\n");
    #ifdef W6100_PHY_AUTO
    printf("Auto Negotiation\r\n");
    #else
    printf("Manual\r\n");
    #endif
    #ifdef W6100_PHY_10MBPS
    printf("10Mbps\r\n");
    #else
    printf("100Mbps\r\n");
    #endif
    #ifdef USE_SPI_DMA
    printf("DMA\r\n");
    #else
    printf("nDMA\r\n");
    #endif
    #ifdef W6100_BUFF_16
    printf("Socket buffer size 16/16\r\n");
    #else
    printf("Socket buffer size 2/2\r\n");
    #endif
    printf("\r\n");

    wizchip_spi_initialize();
    wizchip_cris_initialize();

    wizchip_reset();
    wizchip_initialize();
    wizchip_check();

    // Set ethernet chip MAC address
    setSHAR(mac);
    ctlwizchip(CW_RESET_PHY, 0);

    // Initialize LWIP in NO_SYS mode
    lwip_init();

    netif_add(&g_netif, &g_ip, &g_mask, &g_gateway, NULL, netif_initialize, netif_input);
    g_netif.name[0] = 'e';
    g_netif.name[1] = '0';

    // Assign callbacks for link and status
    netif_set_link_callback(&g_netif, netif_link_callback);
    netif_set_status_callback(&g_netif, netif_status_callback);

    // MACRAW socket open
    retval = socket(SOCKET_MACRAW, Sn_MR_MACRAW, PORT_LWIPERF, 0x00);

    if (retval < 0)
    {
        printf(" MACRAW socket open failed\n");
    }

    // Set the default interface and bring it up
    netif_set_link_up(&g_netif);
    netif_set_up(&g_netif);

    // Start lwiperf server
    lwiperf_start_tcp_server_default(fn, NULL);

    /* Infinite loop */
    while (1)
    {
        getsockopt(SOCKET_MACRAW, SO_RECVBUF, &pack_len);

        if (pack_len > 0)
        {
            pack_len = recv_lwip(SOCKET_MACRAW, (uint8_t *)pack, pack_len);

            if (pack_len)
            {
                p = pbuf_alloc(PBUF_RAW, pack_len, PBUF_POOL);
                pbuf_take(p, pack, pack_len);
                free(pack);

                pack = malloc(ETHERNET_MTU);
            }
            else
            {
                printf(" No packet received\n");
            }

            if (pack_len && p != NULL)
            {
                LINK_STATS_INC(link.recv);

                if (g_netif.input(p, &g_netif) != ERR_OK)
                {
                    pbuf_free(p);
                }
            }
        }

        /* Cyclic lwIP timers check */
        sys_check_timeouts();
    }
}

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
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
