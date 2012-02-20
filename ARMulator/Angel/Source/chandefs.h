/* -*-C-*-
 *
 * $Revision: 1.4.6.2 $
 *   $Author: rivimey $
 *     $Date: 1998/03/05 16:55:05 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Enumeration with all supported channels
 */

#ifndef angel_chandefs_h
#define angel_chandefs_h

enum channelIDs {
  CI_PRIVATE = 0,               /* channels protocol control messages */
  CI_HADP,                      /* ADP, host originated */
  CI_TADP,                      /* ADP, target originated */
  CI_HBOOT,                     /* Boot, host originated */
  CI_TBOOT,                     /* Boot, target originated */
  CI_CLIB,                      /* Semihosting C library support */
  CI_HUDBG,                     /* User debug support, host originated */
  CI_TUDBG,                     /* User debug support, target originated */
  CI_HTDCC,                     /* Thumb direct comms channel, host orig. */
  CI_TTDCC,                     /* Thumb direct comms channel, target orig. */
  CI_TLOG,                      /* Target debug/logging */
  CI_NUM_CHANNELS
};

/* type of frame channel id numbers */
typedef unsigned ChannelID;

/* type of frame sequence numbers */
typedef unsigned char SequenceNR;

/*
 * The number of sequence numbers used. For any protocol with n outstanding
 * packets allowed at once, at most 2^n-1 sequence numbers are needed, where
 * 'n' is the size of the protocol window.
 *
 * For the channels protocol, this number is  (2 ^ CI_NUM_CHANNELS) - 1
 *
 * Sadly, the original channels protocol simply uses all 255 numbers in a
 * byte sequence number.
 */

#define MAX_SEQ   ((SequenceNR)255)

/*
 * increment variable 'k' circularly, mod MAX_SEQ
 */
#define nextSEQ(k) (((k) < MAX_SEQ) ? (SequenceNR)((k) + 1) : (SequenceNR)0)
#define lastSEQ(k) (((k) > 0) ? (SequenceNR)((k) - 1) : MAX_SEQ)
#define INC(k)    (k) = nextSEQ(k)
#define DEC(k)    (k) = lastSEQ(k)

/*
 * Size in bytes of the channel header.
 * This is a duplicate of XXX in chanpriv.h, but we don't want everyone
 * to have access to all of chanpriv.h, so we'll double-check in chanpriv.h.
 */
#define CHAN_HEADER_SIZE (4)

#endif /* ndef angel_chandefs_h */

/* EOF chandefs.h */
