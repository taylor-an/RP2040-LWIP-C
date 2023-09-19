#include <stdio.h>
#include "tcp_server.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "stdlib.h"

uint8_t* msg_v4 = "IPv4 mode";
uint8_t* msg_v6 = "IPv6 mode";
uint8_t* msg_dual = "Dual IP mode";

int32_t tcp_server(uint8_t sn, uint8_t* buf, uint16_t port, uint8_t ip_type)
{
    int32_t ret;
    datasize_t sentsize=0;
    int8_t status,inter;
    uint8_t tmp = 0;
    datasize_t received_size;
    uint8_t arg_tmp8;
    uint8_t* mode_msg;

    if(ip_type == AS_IPV4)
    {
       mode_msg = msg_v4;
    }else if(ip_type == AS_IPV6)
    {
       mode_msg = msg_v6;
    }else
    {
       mode_msg = msg_dual;
    }
    #ifdef _TCP_SERVER_DEBUG_
        uint8_t dst_ip[16], ext_status;
        uint16_t dst_port;
    #endif
        getsockopt(sn, SO_STATUS, &status);
        switch(status)
        {
        case SOCK_ESTABLISHED :
            ctlsocket(sn,CS_GET_INTERRUPT,&inter);
            if(inter & Sn_IR_CON)
            {
            #ifdef _TCP_SERVER_DEBUG_
                getsockopt(sn,SO_DESTIP,dst_ip);
                getsockopt(sn,SO_EXTSTATUS, &ext_status);
                if(ext_status & TCPSOCK_MODE){
                    //IPv6
                    printf("%d:Peer IP : %04X:%04X", sn, ((uint16_t)dst_ip[0] << 8) | ((uint16_t)dst_ip[1]),
                            ((uint16_t)dst_ip[2] << 8) | ((uint16_t)dst_ip[3]));
                    printf(":%04X:%04X", ((uint16_t)dst_ip[4] << 8) | ((uint16_t)dst_ip[5]),
                            ((uint16_t)dst_ip[6] << 8) | ((uint16_t)dst_ip[7]));
                    printf(":%04X:%04X", ((uint16_t)dst_ip[8] << 8) | ((uint16_t)dst_ip[9]),
                            ((uint16_t)dst_ip[10] << 8) | ((uint16_t)dst_ip[11]));
                    printf(":%04X:%04X, ", ((uint16_t)dst_ip[12] << 8) | ((uint16_t)dst_ip[13]),
                            ((uint16_t)dst_ip[14] << 8) | ((uint16_t)dst_ip[15]));
                }else
                {
                    //IPv4
                    //getSn_DIPR(sn,dst_ip);
                    printf("%d:Peer IP : %.3d.%.3d.%.3d.%.3d, ",
                            sn, dst_ip[0], dst_ip[1], dst_ip[2], dst_ip[3]);
                }
                getsockopt(sn,SO_DESTPORT,&dst_port);
                printf("Peer Port : %d\r\n", dst_port);
            #endif
                arg_tmp8 = Sn_IR_CON;
                ctlsocket(sn,CS_CLR_INTERRUPT,&arg_tmp8);
            }
            getsockopt(sn,SO_RECVBUF,&received_size);

            if(received_size > 0){
                if(received_size > DATA_BUF_SIZE) received_size = DATA_BUF_SIZE;
                ret = recv(sn, buf, received_size);

                if(ret <= 0) return ret;      // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
                received_size = (uint16_t) ret;
                #if 0
                sentsize = 0;

                while(received_size != sentsize)
                {
                    ret = send(sn, buf+sentsize, received_size-sentsize);
                    if(ret < 0)
                    {
                        close(sn);
                        return ret;
                    }
                    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                }
                #endif
            }
            break;
        case SOCK_CLOSE_WAIT :
            #ifdef _TCP_SERVER_DEBUG_
                printf("%d:CloseWait\r\n",sn);
            #endif
            getsockopt(sn, SO_RECVBUF, &received_size);
            if(received_size > 0) // Don't need to check SOCKERR_BUSY because it doesn't not occur.
            {
                if(received_size > DATA_BUF_SIZE) received_size = DATA_BUF_SIZE;
                ret = recv(sn, buf, received_size);

                if(ret <= 0) return ret;      // check SOCKERR_BUSY & SOCKERR_XXX. For showing the occurrence of SOCKERR_BUSY.
                received_size = (uint16_t) ret;
                #if 0
                sentsize = 0;

                while(received_size != sentsize)
                {
                    ret = send(sn, buf+sentsize, received_size-sentsize);
                    if(ret < 0)
                    {
                        close(sn);
                        return ret;
                    }
                    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
                }
                #endif
            }

            if((ret = disconnect(sn)) != SOCK_OK) return ret;
                #ifdef _TCP_SERVER_DEBUG_
                    printf("%d:Socket Closed\r\n", sn);
                #endif
            break;
        case SOCK_INIT :
            if( (ret = listen(sn)) != SOCK_OK) return ret;
            printf("%d:Listen, TCP server, port [%d] as %s\r\n", sn, port, mode_msg);
            break;
        case SOCK_CLOSED:
            #ifdef _TCP_SERVER_DEBUG_
                printf("%d:TCP server loopback start\r\n",sn);
            #endif
                switch(ip_type)
                {
                case AS_IPV4:
                    tmp = socket(sn, Sn_MR_TCP4, port, SOCK_IO_NONBLOCK|SF_TCP_NODELAY);
                    break;
                case AS_IPV6:
                    tmp = socket(sn, Sn_MR_TCP6, port, SOCK_IO_NONBLOCK|SF_TCP_NODELAY);
                    break;
                case AS_IPDUAL:
                    tmp = socket(sn, Sn_MR_TCPD, port, SOCK_IO_NONBLOCK|SF_TCP_NODELAY);
                    break;
                default:
                    break;
                }
                if(tmp != sn)    /* reinitialize the socket */
                {
                    #ifdef _TCP_SERVER_DEBUG_
                        printf("%d : Fail to create socket.\r\n",sn);
                    #endif
                    return SOCKERR_SOCKNUM;
                }
            #ifdef _TCP_SERVER_DEBUG_
                printf("%d:Socket opened[%d]\r\n",sn, getSn_SR(sn));
            #endif
            break;
        default:
            break;
        }
    return 1;
}
