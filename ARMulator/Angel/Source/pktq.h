/* -*-C-*-
 *
 * $Revision: 1.1.2.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:12 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited
 * All Rights Reserved.
 */

#ifndef _angel_pktq_h
#define _angel_pktq_h

typedef struct _paddr
{
    void *callback;
    void *cb_data;
    int length;
    DevChanID chan;
    p_Buffer buffer;
    struct _paddr *next;
} PacketInfo;

typedef struct
{
    PacketInfo *first;
    PacketInfo *last;
    PacketInfo *free;
    short count, nels;
} PacketQueue;

int Pq_AddToQueue(PacketQueue *queue, void *callback, void *cb_data,
                  int len, DevChanID chan, p_Buffer buf);

void Pq_InitQueue(PacketQueue *queue, PacketInfo *array, int len);

int Pq_RemoveFromQueue(PacketQueue *queue, void **callback, void **cb_data,
                       int *len, DevChanID *chan, p_Buffer *buf);

int Pq_QueueCount(PacketQueue *queue);

#endif
