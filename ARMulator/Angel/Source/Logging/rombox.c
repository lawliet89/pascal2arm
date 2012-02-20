/* -*-C-*-
 *
 * $Revision: 1.1 $
 *   $Author: mgray $
 *     $Date: 1996/08/22 15:42:42 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Debug interface for ROMbox ROMulator
 */

#include "devconf.h"
#include "rombox.h"

#define ROMBOX_WINDOW           (512)
#define ROMBOX_BASE             (ROMTop - ROMBOX_WINDOW)
#define ROMBOX_OFFSET_READ      (256)

#define ROMBOX_READ(addr)       \
        ((unsigned char)(*(volatile unsigned char *)(ROMBOX_BASE + (addr))))

#define THDAV                   (2)
#define ROMBOX_THDAV            (ROMBOX_READ(0) & THDAV)

static unsigned char forced_read;

static void WriteByte( char c )
{
    unsigned int        read_addr = ROMBOX_OFFSET_READ + c;
    unsigned char       dummy = ROMBOX_READ( read_addr );
    forced_read = dummy;
}

void RomboxPutByte( char c )
{
    /* Wait until ready */
    while ( ROMBOX_THDAV )
       /* idle */ ;

    WriteByte( c );
}

/* EOF rombox.c */
