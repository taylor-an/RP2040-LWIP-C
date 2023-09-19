/**
 * Copyright (c) 2021 WIZnet Co.,Ltd
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
#include "w6x00_spi.h"

#include "tcp_server.h"

#include "timer.h"

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
/* Buffer */
#ifdef W6100_BUFF_16
#define ETHERNET_BUF_MAX_SIZE (1024 * 16)
#else
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)
#endif

/* Socket */
#define SOCKET_TCP_SERVER 0

/* Port */
#define PORT_TCP_SERVER 5001

#define RETRY_CNT   10000

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
/* Network */
static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {W6100_IP0, W6100_IP1, W6100_IP2, W6100_IP3},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {W6100_IP0, W6100_IP1, W6100_IP2, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .lla = {0xfe, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x02, 0x08, 0xdc, 0xff,
                0xfe, 0x57, 0x57, 0x25},             // Link Local Address
        .gua = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Global Unicast Address
        .sn6 = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // IPv6 Prefix
        .gw6 = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Gateway IPv6 Address
        .dns6 = {0x20, 0x01, 0x48, 0x60,
                0x48, 0x60, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x88, 0x88},             // DNS6 server
        .ipmode = NETINFO_STATIC_ALL
};

uint8_t tcp_client_destip[] = {
    192, 168, 50, 103
};

uint8_t tcp_client_destip6[] = {
    0x20, 0x01, 0x02, 0xb8,
    0x00, 0x10, 0xff, 0xff,
    0x71, 0x48, 0xcb, 0x27,
    0x36, 0xb9, 0x99, 0x2e
};

/* Loopback */
static uint8_t g_tcp_server_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/* Timer */
static volatile uint16_t g_msec_cnt = 0;

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
/* Clock */
static void set_clock_khz(void);

/* Timer */
static void repeating_timer_callback(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
int main()
{
    /* Initialize */
    int retval = 0;
    uint8_t dhcp_retry = 0;
    uint8_t dns_retry = 0;

    set_clock_khz();

    stdio_init_all();

    sleep_ms(1000 * 3);

    printf("Compiled @ %s, %s\n", __DATE__, __TIME__);

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

    wizchip_1ms_timer_initialize(repeating_timer_callback);

    network_initialize(g_net_info);

    /* Get network information */
    print_network_information(g_net_info);

    /* Infinite loop */
    while (1)
    {
        /* TCP server iperf test */
        if ((retval = tcp_server(SOCKET_TCP_SERVER, g_tcp_server_buf, PORT_TCP_SERVER, AS_IPV4)) < 0)
        {
            printf(" tcp_server error : %d\n", retval);

            while (1)
                ;
        }
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

/* Timer */
static void repeating_timer_callback(void)
{
    g_msec_cnt++;

    if (g_msec_cnt >= 1000 - 1)
    {
        g_msec_cnt = 0;
    }
}