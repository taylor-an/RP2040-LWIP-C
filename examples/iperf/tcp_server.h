#ifndef _TCP_SERVER_H_
#define _TCP_SERVER_H_

#include <stdint.h>
#include "port_common.h"

/* TCP Server test debug message printout enable */
//#define	_TCP_SERVER_DEBUG_

/* DATA_BUF_SIZE define for TCP Server example */
#ifndef DATA_BUF_SIZE
	#ifdef W6100_BUFF_16
	#define DATA_BUF_SIZE			1024*16
	#else
	#define DATA_BUF_SIZE			1024*2
	#endif
#endif

int32_t tcp_server(uint8_t sn, uint8_t* buf, uint16_t port, uint8_t loopback_mode);


#endif
