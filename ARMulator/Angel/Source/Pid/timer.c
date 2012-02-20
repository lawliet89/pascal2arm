/* -*-C-*-
 *
 * $Revision: 1.3 $
 *   $Author: amerritt $
 *     $Date: 1996/11/04 18:39:51 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Profile timer driver for PID card
 *
 * The timer used for profiling is also shared if ETHERNET_SUPPORTED
 * is set.  In this case the timer is also used to kick off Angel_Yield
 * roughly 5 times a second, since Ethernet is a fully polled device.
 * This is required to ensure that User interrupt requests (for example)
 * are spotted during execution of the user's application, when Angel_Yield
 * is not otherwise called.
 */


#include "pid.h"
#include "devconf.h"
#include "serlock.h"
#include "prof.h"

#if ETHERNET_SUPPORTED
#define CALL_YIELD_FROM_TIMER_IRQ 1
#else
#define CALL_YIELD_FROM_TIMER_IRQ 0
#endif

#if CALL_YIELD_FROM_TIMER_IRQ
static unsigned call_yield_every_n_irqs;
static unsigned call_yield_count;
#endif

static profiling_enabled = 0;

void Angel_ProfileTimerInitialise(void)
{
#if CALL_YIELD_FROM_TIMER_IRQ
  /* Set up timer to interrupt once every 0.2 seconds using the
   * factor of 256 prescaling available in the timer.
   * interval is the desired interrupt interval (in microseconds)
   */
  unsigned interval = 200000; /* micro seconds */
  /* This calculation is prone to errors due to rounding */
  CT1->Load = (unsigned)(((interval / 256) * SYSCLOCK) / 1000);
  CT1->Control = CTEnable | CTPeriodic | CTPrescale8;
  *IRQEnableSet = IRQ_TIMER1;
  call_yield_every_n_irqs = 1;
  call_yield_count = 0;
#else
  /* We don't have to worry about the timer until we switch on
   * profiling.
   */
  CT1->Control = CTDisable;
  *IRQEnableClear = IRQ_TIMER1;
#endif
  profiling_enabled = 0;
}


#if PROFILE_SUPPORTED

/* We have to set the timer to the requested interval, and work around
 * this for calling Yield (if required).
 */
void Angel_ProfileTimerStart(int interval)
{
  /* interval is the desired interrupt interval (in microseconds) */
#if ((SYSCLOCK / 1000) * 1000) != SYSCLOCK
# error Please recode formula - it relies on SYSCLOCK being a factor of 1000
#endif
#if CALL_YIELD_FROM_TIMER_IRQ
  /* Calculate the number of timer IRQ's in 0.2 seconds */
  call_yield_every_n_irqs = 200000U / interval;
#endif
  profiling_enabled = 1;
  CT1->Load = (unsigned)(interval * (SYSCLOCK / 1000));
  CT1->Control = CTEnable | CTPeriodic | CTPrescale0;
  *IRQEnableSet = IRQ_TIMER1;
}

void Angel_ProfileTimerStop(void)
{
  /* If we don't care about Angel_Yield then just disable the timer
   * otherwise we must leave it enabled, but can set it to interrupt
   * much less frequently to reduce the overhead.
   */
  Angel_ProfileTimerInitialise();
}

#pragma no_check_stack

void Angel_TimerIntHandler(unsigned ident, unsigned data, unsigned empty_stack)
{
  unsigned pc;
  
  IGNORE(ident);
  IGNORE(data);
  IGNORE(empty_stack);

  if (profiling_enabled) {
    pc = Angel_MutexSharedTempRegBlocks[0].r[15];
    if (angel_profilestate.enabled) {
      unsigned *map = angel_profilestate.map;
      int size = angel_profilestate.numentries;
      int low = 0,
        high = size-1;
      if (map[low] <= pc && pc < map[high]) {
        int i;
        for (;;) {
          i = (high + low) / 2;
          if (pc >= map[i]) {
            if (pc < map[i+1]) {
              i++; break;
            } else
              low = i;
          } else if (pc >= map[i-1])
            break;
          else
            high = i;
        }
        angel_profilestate.counts[i-1]++;
      }
    }
    CT1->Clear = 0;
  }

#if CALL_YIELD_FROM_TIMER_IRQ
  /* See if it is time to call Angel_Yield yet */
  if (++call_yield_count >= call_yield_every_n_irqs) {
    call_yield_count=0;
    CT1->Clear = 0;
    Angel_SerialiseTask(0, (angel_SerialisedFn) Angel_YieldCore, NULL,
                        empty_stack);
  }
#endif

}

#endif /* PROFILE_SUPPORTED */

#pragma check_stack

/* EOF pid/timer.c */
