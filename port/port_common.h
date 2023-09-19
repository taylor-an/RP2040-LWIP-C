/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PORT_COMMON_H_
#define _PORT_COMMON_H_

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
/* Common */
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/critical_section.h"
#include "hardware/spi.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

// Set W6100 Configuation

#if 0
#define W6100_PHY_AUTO
#else
#define W6100_PHY_MANUAL
#if 1
#define W6100_PHY_10MBPS
#else
#define W6100_PHY_100MBPS
#endif
#endif

#if 1
#define W6100_PHY_FULL_DUP
#else
#define W6100_PHY_HALF_DUP
#endif

#if 1
#define W6100_BUFF_16
#else
#define W6100_BUFF_2
#endif

#define W6100_IP0 192
#define W6100_IP1 168
#define W6100_IP2 100
#define W6100_IP3 10

// Set ENC28J60 Configuation

#define ENC28J60_IP0 192
#define ENC28J60_IP1 168
#define ENC28J60_IP2 100
#define ENC28J60_IP3 20

// Set W6100 and ENC28J60 Configuration

#if 1
#define USE_SPI_DMA
#else
#define USE_SPI_NDMA
#endif

#define PLL_SYS_KHZ (133 * 1000)
#define SPI_CLK_KHZ (20 * 1000 * 1000)

// 133M 50M @ Get SPI CLK 33.250000 MHz
// 133M 30M @ Get SPI CLK 22.166666 MHz
// 133M 25M @ Get SPI CLK 22.166666 MHz
// 133M 20M @ Get SPI CLK 16.625000 MHz
// 133M 23M @ Get SPI CLK 22.166666 MHz

#endif /* _PORT_COMMON_H_ */
