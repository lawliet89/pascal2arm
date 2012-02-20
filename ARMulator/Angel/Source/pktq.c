/* -*-C-*-
 *
 * $Revision: 1.1.2.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:11 $
 *
 * Copyright (c) 1997 Advanced RISC Machines Limited
 * All Rights Reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#include "pktq.h"


void Pq_InitQueue(PacketQueue *queue, PacketInfo *array, int len)
{
    queue->first = NULL;
    queue->last = NULL;
    queue->count = 0;
    queue->free = array;
    queue->nels = len;
    for(i = 0; i < queue->nels; i++)
    {
        array[i].data = NULL;
        array[i].next = 0;
    }
}

int Pq_AddToQueue(PacketQueue *queue, void *callback, void *cb_data,
                  int len, DevChanID chan, p_Buffer buf)
{
    int i;
    PacketInfo *pa;

    for(i = 0; i < queue->nels; i++)
    {
        if (queue->free[i].data == NULL)
        {
            break;
        }
    }
    if (i == queue->nels)
        return -1;

    pa = queue->free[i];
    pa->data = data;
    pa->next = NULL;
    if (queue->count)
        queue->last->next = pa;
    else
        queue->last = queue->first = pa;
    queue->count++;
    
    return i;
}


int Pq_RemoveFromQueue(PacketQueue *queue, void **callback, void **cb_data,
                       int *len, DevChanID *chan, p_Buffer *buf)
{
    if (queue->count)
    {
        *callback = queue->first->callback;
        *cb_data = queue->first->cb_data;
        *len = queue->first->len;
        *buffer = queue->first->buf;
        *chan = queue->first->chan;
        queue->first->buffer = NULL;
        queue->first->len = 0;
        if (queue->first->next)
            queue->first = queue->first->next;
        else
            queue->first = queue->last = queue->first->next;
        queue->count--;
        return 1;
    }
    else
        return 0;
}    

int Pq_QueueCount(PacketQueue *queue)
{
    return queue->count;
}
