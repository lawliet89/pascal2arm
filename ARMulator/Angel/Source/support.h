/* -*-C-*-
 *
 * $Revision: 1.9.6.1 $
 *   $Author: rivimey $
 *     $Date: 1997/12/10 18:50:25 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 * support.h - General support routines in suppasm.s.
 */

#ifndef angel_support_h
#define angel_support_h

/*
 *  Function: delay
 *   Purpose: Pause for a small period of time
 *
 *    Params:
 *       Input: period  Number of microseconds to delay for
 *
 *   Returns: Nothing
 *
 *      NOTE: Actual delay period is dependent upon host machine's
 *            clock granularity.
 */
extern void delay(unsigned int period);

/*---------------------------------------------------------------------------*/

/*
 * __rt_memcpy
 * -----------
 * This is a local Angel version of the "memcpy" routine.
 */
extern void *__rt_memcpy(void *dst, const void *src, unsigned int amount);

/*
 * more local Angel versions of common routines
 */
extern void *__rt_memset(void *s, int c, unsigned int n);
extern int __rt_memcmp(const void *s1, const void *s2, unsigned int n);
extern int __rt_strcmp(const char *s1, const char *s2);
extern int __rt_strncmp(const char *s1, const char *s2, unsigned int n);
extern unsigned int __rt_strlen(const char *s);
extern char *__rt_strcpy(char *dst, const char *src);
extern char *__rt_strcat(char *s1, const char *s2);

/*---------------------------------------------------------------------------*/

/*
 * angel_Endianess is an array of 2 words which are set at build time
 * depending on the endianess of the ROM:
 *
 *                       Little-Endian ROM   Big-Endian ROM
 * angel_Endianness[0]      ADP_CPU_LE         ADP_CPU_BE
 * angel_Endianness[1]      0                  ADP_CPU_BigEndian
 */
extern unsigned int angel_Endianess[2];

/*
 * Angel_LateStartup() permits an application to notify Angel that
 * it wishes to connect to a debugger now.
 */
typedef enum angel_LateStartType {
    AL_CONTINUE,
    AL_BLOCK
} angel_LateStartType;

void Angel_LateStartup( angel_LateStartType type );


#endif /* ndef angel_support_h */

/* EOF support.h */
