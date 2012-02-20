/* -*-C-*-
 *
 * $Revision: 1.2 $
 *   $Author: amerritt $
 *     $Date: 1996/11/04 18:39:34 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Profiling support
 */

#ifndef angel_prof_h
#define angel_prof_h

/* Angel_ProfileTimerInitialise is called (if it exists) during Angel
 * startup.  It typically puts the profile timer into a safe state (off).
 */
void Angel_ProfileTimerInitialise(void);

void Angel_ProfileTimerStart(int interval);

void Angel_ProfileTimerStop(void);

void Angel_TimerIntHandler(unsigned ident, unsigned data, unsigned empty_stack);

typedef struct {
  unsigned *map;
  unsigned *counts;
  int numentries;
  int enabled;
} AngelProfileState;

extern AngelProfileState angel_profilestate;

#endif
