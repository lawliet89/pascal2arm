/* -*-C-*-
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited
 * All Rights Reserved
 *
 * $Revision: 1.8.6.3 $
 *   $Author: rivimey $
 *     $Date: 1997/12/22 14:15:32 $
 *
 * ethernet.c:  Angel drivers for Ethernet using Fusion UCP/IP stack
 */

/* It is important to include config.h here before anyone else does since
 * it must be included with NEED_CONFIG_INFO defined. In addition we must
 * ensure that devconf.h gets included as this will do a check that Angel
 * and Fusion both agree on how much space is allocated for the Fusion
 * stack
 */
#define NEED_CONFIG_INFO
#include "config.h"
#undef  NEED_CONFIG_INFO
#include "devconf.h"

#include "angel.h"
#include "channels.h"           /* for getting buffers */
#include "endian.h"             /* PUT16LE */
#include "logging.h"            /* LogWarning() */
#include "support.h"            /* __rt_[en/dis]ableIRQ() */
#include "serlock.h"            /* serialisation, etc. */
#include "params.h"             /* parameter structures and utilities */

#include "pid.h"
#include "ethernet.h"

/*
 * Fusion includes
 */
#include "std.h"
#include "enet.h"
#include "flags.h"
#include "netdev.h"
#include "netioc.h"
#include "m.h"
#include "socket.h"
#include "ip.h"
#include "in.h"
#include "boot.h"
/*
 * forward declarations of necesary routines
 */
static DevError ethernet_Write(DeviceID devid, p_Buffer buffer,
                               unsigned int length, DevChanID devchan);

static DevError ethernet_RegisterRead(DeviceID devid,
                                      DevChanID devchan);

static DevError ethernet_Control(DeviceID devid, DeviceControl op, void *arg);

const struct angel_DeviceEntry angel_EthernetDevice =
{
    DT_ANGEL,
    {
        ethernet_Write, ethernet_RegisterRead
    },
    ethernet_Control,
    0,
    {
        0, NULL
    },
    {
        0, NULL
    }
};

/*
 * the three sockets that we talk down
 */
static int ctrlsock = -1, dbugsock = -1, applsock = -1;

/*
 * holder for errors from Fusion
 */
static int errno;

/*
 * The host we are currently talking to.  Assumed to be the last host
 * that we received anything from
 */
static struct in_sockaddr host;
static int hostlen;

static int rx_flow = 1; /* see do_poll()/ethernet_init() */

/* Our IP address, set elsewhere */
extern char fusion_ipaddr[];

/**********************************************************************/

/*
 *  Function: open_socket
 *   Purpose: Open a non-blocking UDP socket, and bind it to a port number
 *
 *    Params:
 *       Input: port    The port to bind to
 *
 *   Returns:
 *          OK: Socket number
 *       Error: -1
 */
static int open_socket(const unsigned int port)
{
    int sock;
    struct in_sockaddr to;

    /*
     * open the socket, and try to bind it to the given port
     */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0, &errno)) < 0)
    {
        LogWarning(LOG_ETHER, ("socket: error %d\n", errno));
        return -1;
    }

    /*
     * try to bind to the port
     */
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    if ( port > 0 )
        to.sin_addr.s_addr = INADDR_ANY;
    else
    {
        /* Fusion explicitly fails to allocate an automatic port
         * if s_addr == INADDR_ANY, instead using 0!!
         * So we force it to behave by specifying our real address
         */
        __rt_memcpy( &to.sin_addr.s_addr, fusion_ipaddr,
                     sizeof(to.sin_addr.s_addr) );
    }
    if ((errno = bind(sock, (struct sockaddr*)&to, sizeof(to))) != 0)
    {
        LogWarning(LOG_ETHER, ("bind(%d, %d): error %d\n", sock, port, errno));
        (void)close(sock);
        return -1;
    }

    /*
     * mark this socket as non-blocking
     */
    if ((errno = nonblocking(sock)) != 0)
    {
        LogWarning(LOG_ETHER, ("nonblocking(%d): error %d\n", sock, errno));
        (void)close(sock);
        return -1;
    }

    /*
     * socket is ready for action
     */
    return sock;
}

/*
 *  Function: findport
 *   Purpose: Ascertain the port number bound to a socket
 *
 *    Params:
 *       Input: sock    socket descriptor
 *
 *   Returns:
 *          OK: port (in network byte order)
 *       Error: 0
 */
static unsigned short int findport(int sock)
{
    struct in_sockaddr name;
    int namelen;

    namelen = sizeof(name);
    if (getsockname(sock, (struct sockaddr *)&name, &namelen) != 0)
    {
        LogWarning(LOG_ETHER, ("findport failed for sock %d\n", sock));
        return 0;
    }
    else
        return name.sin_port;
}

/*
 *  Function: do_ctrlpoll
 *   Purpose: Poll the control socket for received packets
 *
 *    Params: None
 *
 *   Returns: Nothing
 */
static void do_ctrlpoll(void)
{
    static const char ctrlmagic[] = CTRL_MAGIC;
    CtrlResponse dialogue;
    struct in_sockaddr remote;
    int remotelen = sizeof(remote);

    /*
     * see whether anything is available on the control socket
     */
    if (recvfrom(ctrlsock, (char *)dialogue, sizeof(dialogue), 0,
                 (struct sockaddr *)&remote, &remotelen, &errno) < 0)
    {
        if (errno != EWOULDBLOCK)
            LogWarning(LOG_ETHER, ("recvfrom: error %d\n", errno));
    }
    else if (__rt_strcmp(ctrlmagic, (char *)dialogue) == 0)
    {
        /*
         * this is a valid control packet - send the port numbers
         * back to the requester
         */
        unsigned short *sptr = (unsigned short *)(dialogue + RESP_DBUG);
        *sptr   = findport(dbugsock);
        *++sptr = findport(applsock);
        if (sendto(ctrlsock, (char *)&dialogue, sizeof(dialogue), 0,
                   (struct sockaddr *)&remote, remotelen, &errno) < 0)
            LogWarning(LOG_ETHER, ("sendto: error %d\n", errno));
        else
        {
            /*
             * make a copy of the remote address of the client
             */
            os_move(&remote, &host, remotelen);
            hostlen = remotelen;
        }
    }
    else
        LogWarning(LOG_ETHER, ("bad control packet\n"));
}

/*
 *  Function: do_poll
 *   Purpose: Poll an open socket for received packets
 *
 *    Params:
 *       Input: sock    The socket to poll
 *
 *              devchan The device channel associated with the socket
 *
 *   Returns: Nothing
 */
static void do_poll(const int sock, DevChanID devchan)
{
    int nbytes;
    p_Buffer buffer;
    static char frmbuf[UDP_RQ_MAX];

    /* if we've been disabled from returning packets, don't ask for them! */
    if (rx_flow == 0)
    {
        LogWarning(LOG_ETHER, ("do_poll: rx_flow == 0\n", errno));
        return;
    }
    
    /*
     * Unfortunately, we don't know how long the next packet is going
     * to be, so we have a private MTU buffer, and copy into an
     * allocated buffer when we know how much space to ask for.  If
     * this proves to be too inefficient, then we will have to look
     * at other ways of doing this.
     */
    if ((nbytes = recv(sock, frmbuf, sizeof(frmbuf), 0, &errno)) < 0)
    {
        if (errno != EWOULDBLOCK)
            LogWarning(LOG_ETHER, ("do_poll: recv: error %d\n", errno));
    }
    else if( nbytes != 0 )
    {
        if (rx_flow < 0)
            rx_flow ++;
        
        /*
         * pass this along to higher levels
         */
        if ((buffer = angel_DD_GetBuffer(DI_ETHERNET, devchan, nbytes)) != NULL)
        {
            __rt_memcpy(buffer, frmbuf, nbytes);
            angel_DD_GotPacket(DI_ETHERNET, buffer, nbytes, DS_DONE, devchan);
        }
        else
            LogWarning(LOG_ETHER, ("do_poll: couldn't DD_GetBuffer"));
    }
}

/*
 *  Function: ethernet_init
 *   Purpose: Initialise the UDP stack
 *
 *    Params: Nothing
 *
 *   Returns:
 *          OK: DE_OKAY
 *       Error: DE_NO_DEV
 */
static DevError ethernet_init(void)
{
    /* rx_flow == 1 enabled packet flow; == 0 disables it, < 0 specifies the
       number of packets allowed through. See DC_RX_PACKET_FLOW && serpkt.c */
    rx_flow = 1;
    
    /*
     * try to initialise the three sockets
     */
    if ((ctrlsock = open_socket(CTRL_PORT)) < 0)
    {
        LogWarning(LOG_ETHER, ( "Couldn't open control socket\n" ));
        return DE_NO_DEV;
    }

    if ((dbugsock = open_socket(0)) < 0)
    {
        (void)close(ctrlsock);
        ctrlsock = -1;

        LogWarning(LOG_ETHER, ( "Couldn't open DBUG socket\n" ));
        return DE_NO_DEV;
    }

    if ((applsock = open_socket(0)) < 0)
    {
        (void)close(ctrlsock);
        ctrlsock = -1;

        (void)close(dbugsock);
        dbugsock = -1;

        LogWarning(LOG_ETHER, ( "Couldn't open APPLSOCK socket\n" ));
        return DE_NO_DEV;
    }

    /*
     * it worked!
     */
    return DE_OKAY;
}

/**********************************************************************/

/*
 *  Function: ethernet_Write
 *   Purpose: Entry point for asynchronous writes to the ethernet device.
 *            See documentation for angel_DeviceWrite in devdriv.h
 */
static DevError ethernet_Write(DeviceID devid, p_Buffer buffer,
                               unsigned int length, DevChanID devchan)
{
    int sock, nbytes;
    DevStatus status = DS_DONE;

    IGNORE(devid);

    if (devchan == DC_DBUG)
        sock = dbugsock;
    else if (devchan == DC_APPL)
        sock = applsock;
    else
    {
        LogError(LOG_ETHER, ("ethernet_Write: bad devchan %d\n", devchan));
        return DE_BAD_CHAN;
    }

    /*
     * we need to broadcast the first message sent (the booted,
     * message), but talk point-to-point once an host has been
     * identified
     */
    if (hostlen == 0)
    {
#ifdef FUSION_IS_NOT_SO_STUPID_AFTER_ALL
        /*
         * calculate broadcast address; this is crude (should use
         * SIOGIFBRDADDR, but Fusion is broken yet again, because
         * this, or an equivalent, is not supported).
         */
        extern FNSDATA local_fns_data[];
        FNSDATA *fdp = local_fns_data;
        unsigned int bcastaddr, subnet;

        /*
         * XXX
         *
         * despite our initial settings, after booting, subnet_mask and
         * my_ip_address are stored in network byte order.
         */
        subnet = *((__packed unsigned int *)(fdp->subnet_mask));
        bcastaddr = *((__packed unsigned int *)(fdp->my_ip_address));
        bcastaddr |= (IP_BROADCAST & ~subnet);
#else
        /*
         * GACK!
         *
         * It's even worse than I thought - Fusion does not support
         * directed broadcast, in direct contradiction to the relevant
         * RFC (1122), so we are forced to use the limited broadcast
         * address =8(
         */
        unsigned int bcastaddr = htonl(IP_BROADCAST);
#endif

        host.sin_family = AF_INET;
        host.sin_addr.s_addr = bcastaddr;
        hostlen = sizeof(host);
    }

    /* From this point, we need to be protected in SVC mode */
    Angel_EnterSVC();

    if ((nbytes = sendto(sock, (char *)buffer, length, 0,
                         (struct sockaddr *)&host, hostlen, &errno)) != length)
    {
        LogWarning(LOG_ETHER, ("ethernet_Write: sendto returned %d: error %d\n",
                               nbytes, errno));
        status = DS_DEV_ERROR;
    }

    /*
     * schedule a callback to say the packet has gone
     */
    angel_DD_SentPacket(DI_ETHERNET, buffer, length, status, devchan);

    /* now it's okay to resume in user mode */
    Angel_ExitToUSR();

    return DE_OKAY;
}

/*
 *  Function: ethernet_RegisterRead
 *   Purpose: Entry point for asynchronous reads from the ethernet device.
 *            See documentation for angel_RegisterRead in devdriv.h
 */
static DevError ethernet_RegisterRead(DeviceID devid, DevChanID devchan )
{
    IGNORE(devid);
    IGNORE(devchan);

    /* nothing to do */
    return DE_OKAY;
}

/*
 *  Function: ethernet_Control
 *   Purpose: Entry point for ethernet device control functions.
 *            See documentation for angel_DeviceControl in devdriv.h
 */
static DevError ethernet_Control(DeviceID devid, DeviceControl op, void *arg)
{
    DevError ret_code;

    IGNORE(devid);
    IGNORE(arg);

    Angel_EnterSVC();

    LogInfo(LOG_ETHER, ( "ethernet_Control: op %d arg %x\n", op, arg));

    /* we only implement the standard ones here */
    switch (op)
    {
        case DC_RX_PACKET_FLOW:
            LogInfo(LOG_ETHER, ( "ethernet_Control: set rx_flow to %d (was %d)\n",
                                 (int) arg, rx_flow));
            rx_flow = (int)arg;
            ret_code = DE_OKAY;
            break;
            
        case DC_INIT:          
            ret_code = ethernet_init();
            break;

#if 0
        case DC_RESET:         
            ret_code = ethernet_reset(devid, port);
            break;

        case DC_RECEIVE_MODE:  
            ret_code = ethernet_recv_mode(devid, (DevRecvMode)((int)arg));
            break;

        case DC_SET_PARAMS:     
            ret_code = ethernet_set_params((const ParameterConfig *const)arg);
            break;
#endif

        default:
            ret_code = DE_BAD_OP;
            break;
    }

    Angel_ExitToUSR();

    return ret_code;
}

/**********************************************************************/

/*
 *  Function: angel_EthernetPoll
 *   Purpose: Poll for newly arrived packets.  See function prototype in
 *              ethernet.h for a full description.
 */
void angel_EthernetPoll(unsigned int data)
{
    int s = os_critical();

    IGNORE(data);

    /*
     * poll control device...
     */
    do_ctrlpoll();

    /*
     * ... then poll both channel devices
     */
    do_poll(dbugsock, DC_DBUG);
    do_poll(applsock, DC_APPL);

    os_normal(s);
}


void angel_EthernetNOP(unsigned int data)
{
    IGNORE(data);
}

/*
 *  Function: angel_FindEthernetConfigBlock
 *   Purpose: Search the Flash for an ethernet config block and return
 *            it if found.
 *
 *   Note that MIN_FLASH_SIZE etc are defined in pid.h
 */
angel_EthernetConfigBlock *angel_FindEthernetConfigBlock(void)
{
    unsigned sector_size, flash_size;
    angel_EthernetConfigBlock *block;

    /* Look at the bottom of the top sector or the Flash */

    for (flash_size=MIN_FLASH_SIZE; flash_size<MAX_FLASH_SIZE;
         flash_size *= 2) {
        for (sector_size=MIN_SECTOR_SIZE; sector_size<MAX_SECTOR_SIZE;
             sector_size *=2) {
            block = (angel_EthernetConfigBlock *) (FlashBase + flash_size - sector_size);
            if (block->block_identifier[0] == 0x7F &&
                block->block_identifier[1] == 0xDE &&
                block->block_identifier[2] == 0xA6 &&
                block->block_identifier[3] == 0x8B) {
                return block;
            }
        }
    }
    return NULL;
}

/* EOF ethernet.c */
