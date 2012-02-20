/* -*-C-*-
 *
 * $Revision: 1.3.6.3 $
 *   $Author: rivimey $
 *     $Date: 1998/01/12 21:21:21 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Ring buffers for use in device drivers
 */

#ifndef angel_ringbuff_h
#define angel_ringbuff_h

/*
 * a ring buffer structure for interposing between 
 * the interrupt handler and deferred processor 
 */
typedef struct RingBuffer
{
    volatile unsigned int  count;
    volatile unsigned int  tail;
    volatile unsigned char data[RING_SIZE];
    volatile unsigned int  ovr[(RING_SIZE+31)/32]; /* +31 to round up not down */
} RingBuffer;

/*
 * ring buffer operations as macros. When empty, reset 'tail' to zero
 * to bring it back within the buffer...
 */
#define ringBufNotEmpty(r)   ((r)->count > 0)
#define ringBufEmpty(r)      (((r)->count == 0) ? ((r)->tail = 0, 1): 0)
#define ringBufFull(r)       ((r)->count >= RING_SIZE)
#define ringBufCount(r)      ((r)->count)

#define ringBufGetChar(r)    ((r)->count--, (r)->data[((r)->tail++) % RING_SIZE])
#define ringBufPutChar(r, c) ((r)->data[((r)->tail + ((r)->count++)) % RING_SIZE] = (c))

/*
 * Set/Get overrun flag. Uses bitmap 'ovr' above
 * note: SetOvr and IsOvr both apply to the next character Put or Got.
 */
#define ringTail(r)          (((r)->tail) % RING_SIZE)
#define ringBufIsOvr(r)      ((r)->ovr[ringTail(r) >> 5] & (1 << (ringTail(r) & 0x1f)))
#define ringBufSetOvr(r)     ((r)->ovr[ringTail(r) >> 5] |=  (1 << (ringTail(r) & 0x1f)))
#define ringBufResetOvr(r)   ((r)->ovr[ringTail(r) >> 5] &= ~(1 << (ringTail(r) & 0x1f)))

#endif /* ndef angel_ringbuff_h */

/* EOF ringbuff.h */
