/* -*-C-*-
 *
 * $Revision: 1.5.6.2 $
 *   $Author: rivimey $
 *     $Date: 1997/12/19 15:54:52 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: High to low level interface via ring buffers
 * for serial (and similar) devices.
 */

#ifndef angel_serring_h
#define angel_serring_h

#include "ringbuff.h"
#include "rxtx.h"

/*
 * Flags stored in angel_DeviceStatus
 */
#define SER_RX_IRQ_EN         (1 << 8)  /* reflects state of IMR */
#define SER_RX_DISABLED       (1 << 9)  /* rx disabled and XOFF sent */

#define SER_TX_DATA           (1 << 10) /* true if tx'ing packet */
#define SER_TX_FLOW_CONTROL   (1 << 11) /* true if need to tx flow char */
#define SER_TX_CONTROLLED_OFF (1 << 12) /* true if disabled by XOFF */
#define SER_TX_IRQ_EN         (1 << 13) /* reflects state of IMR */
#define SER_TX_EOD            (1 << 14) /* end of data pkt is in ring buffer */

/* mask needed by st16c552_ControlTx() */
#define SER_TX_MASK (SER_TX_DATA | SER_TX_FLOW_CONTROL | SER_TX_CONTROLLED_OFF)

#define SER_RX_QUEUED         (1 << 15) /* deferred rx processing is queued */
#define SER_TX_QUEUED         (1 << 16) /* deferred tx processing is queued */
#define SER_TX_KICKSTART      (1 << 17) /* need to kickstart packet Tx */
#define SER_RX_OVERFLOW       (1 << 18) /* rx detected character overflow */

/* collected details required for Rx/Tx-engine based packet devices */
typedef struct RxTxState {
    const struct re_config *const config;
    struct re_state        *const re_state;
    struct te_state        *const te_state;
    struct data_packet     *const rx_packet;
    struct data_packet     *const tx_packet;
    int rx_pkt_flow;       /* see DC_RX_PACKET_FLOW, serpkt.c */
} RxTxState;

/* collected details required for raw devices */
typedef struct RawState {
    unsigned int      rx_to_go;
    unsigned int      rx_copied;
    p_Buffer          rx_data;
    unsigned int      tx_n_done;
    unsigned int      tx_length;
    p_Buffer          tx_data;
} RawState;

/* function prototypes */
typedef void (*ControlTx_Fn)(DeviceID devid);
typedef void (*ControlRx_Fn)(DeviceID devid);
typedef DevError (*Control_Fn)(DeviceID devid, DeviceControl op, void *arg);
typedef void (*KickStart_Fn)(DeviceID devid);
typedef void (*Processor_Fn)(void *args);

/* collected glue for interface between high- and low-level device drivers */
typedef struct SerialControl {
    RxTxState      *const rx_tx_state; /* can be NULL */
    RawState       *const raw_state;   /* can be NULL --> look for ETX */
    Processor_Fn    tx_processing;     /* how to fill/process tx ring */
    Processor_Fn    rx_processing;     /* how to read/process rx ring */
    unsigned int    port;              /* device-specific ID */
    RingBuffer     *const tx_ring;     /* tx ring buffer */
    RingBuffer     *const rx_ring;     /* rx ring buffer */
    ControlTx_Fn    control_tx;        /* how to control tx IRQs, etc. */
    ControlRx_Fn    control_rx;        /* how to control rx IRQs, etc. */
    Control_Fn      control;           /* device control function */
    KickStart_Fn    kick_start;        /* kick-start function for tx */
} SerialControl;

/* handy macros for accessing above structures */
#define SerCtrl(d)      ((SerialControl *)(angel_Device[(d)]->data))
#define SerEngine(d)    (SerCtrl(d)->rx_tx_state)

#if !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0

/* prototypes for shared packet-based high-level device driver */
void serpkt_int_rx_processing(void *args);
void serpkt_int_tx_processing(void *args);

DevError serpkt_AsyncWrite(DeviceID devid, p_Buffer buffer,
                           unsigned int length, DevChanID devchan);

DevError serpkt_RegisterRead(DeviceID devid, DevChanID devchan);

DevError serpkt_Control(DeviceID devid, DeviceControl op, void *arg);

void serpkt_flow_control(char fc_char, void *cb_data);

#endif /* ndef MINIMAL_ANGEL */

/* prototypes for shared raw high-level device driver */
void serraw_int_rx_processing(void *args);
void serraw_int_tx_processing(void *args);

DevError serraw_Write(DeviceID devid, p_Buffer buffer, unsigned int length);
DevError serraw_Read(DeviceID devid, p_Buffer buffer, unsigned int length);
DevError serraw_Control(DeviceID devid, DeviceControl op, void *arg);

#endif /* ndef angel_serring_h */

/* EOF serring.h */
