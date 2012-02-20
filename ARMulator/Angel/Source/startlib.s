        TTL     Generic ARM start-of-day (initialisation) code  > startlib.s
        ; ---------------------------------------------------------------------

        ; This file provides the Angel initialisation and startup code
        ; for library based systems. It is also the entry point for
        ; the C-Library. After the initialisation, the start-up
        ; routine "_main" is called.
        ;
        ; The source file "startrom.s" provides the Angel startup code
        ; for ROM based versions of Angel, which eventually calls this
        ; code to start Angel and the application.
        ;
        ; $Revision: 1.9.6.1 $
        ;   $Author: rivimey $
        ;     $Date: 1997/12/10 18:50:22 $
        ;
        ; Copyright Advanced RISC Machines Limited, 1995.
        ; All Rights Reserved
        ; ---------------------------------------------------------------------

        GET     listopts.s           ; standard listing control
        GET     lolevel.s            ; automatically built manifest definitions
        GET     macros.s             ; standard assembler support
        GET     target.s             ; target specific manifests

        ; ---------------------------------------------------------------------

        EXPORT  |__main|             ; Angel startup entry point (non-APCS)

        IMPORT  |__rt_asm_fatalerror| ; error reporting via suppasm.s
        IMPORT  |angel_ExceptionInit|,WEAK  ; exception handler initialisation
        IMPORT  |angel_InitialiseDevices|,WEAK  ; device driver initialisation
        IMPORT  |angel_InitialiseChannels|,WEAK ; channel initialisation
        IMPORT  |Angel_ProfileTimerInitialise|,WEAK ; profile & yield timer
        IMPORT  |angel_BootInit|,WEAK       ; bootstrap agent initialisation
        IMPORT  |angel_DebugInit|,WEAK      ; debugging support initialisation
        IMPORT  |angelOS_Initialise|,WEAK   ; OS debug support initialisation
        IMPORT  |angel_SysLibraryInit|,WEAK ; CLib support initialisation
  IF :DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0
        IMPORT  |Angel_RawDeviceControl|,WEAK ; ---"---
        IMPORT  |__entry|                     ; Application entry point
  ELSE
        IMPORT  |angel_DeviceControl|,WEAK  ;  Used to set the LED on the card
        IMPORT  |__entry|,WEAK                ; Application entry point
  ENDIF
        IMPORT  |Angel_Yield|
        IMPORT  |Angel_EnterSVC|
        IMPORT  |Angel_ExitToUSR|
        IMPORT  |Angel_InitialiseTaskFinished|
        IMPORT  |Angel_BlockApplication|
        IMPORT  |Angel_StackBase|
        
        ; ---------------------------------------------------------------------

        AREA    |C$$Code$$Startup|,CODE,READONLY
|__main|
        ; in:   SVC mode; IRQs and FIQs disabled.
        ;       r13 = FD stack
        ; out:  Exits via the |_main| routine.
        ;
        ; Setup an APCS register state before calling any of the initialisation
        ; routines.  We also want to be in USR mode, with the Angel stack
        ; in use.  However we want to keep interrupts disabled too, which is
        ; unusual (normally USR mode => IRQ and FIQ enabled).
        ;
              
        LDR     a2, =Angel_StackBase
        LDR     a2, [a2]
        ADD     sp, a2, #Angel_SVCStackOffset ;  Reset the SVC stack
        
        MOV     a1, #USRmode :OR: IRQDisable :OR: FIQDisable
        MSR     CPSR, a1
        
        ADD     sp, a2, #Angel_AngelStackOffset
        ADD     sl, a2, #Angel_AngelStackLimitOffset
        MOV     fp, #0x00000000

        IF :DEF: JTAG_ADP_SUPPORTED
          IF JTAG_ADP_SUPPORTED <> 0
            B       __entry
            
            EXPORT |main|
main
          ENDIF
        ENDIF


        
        ; This is the main C-Library and Angel startup point. The
        ; Angel world and the library are initialised, before calling
        ; the application. NOTE: In a ROM only monitor world, the
        ; application is a simple control loop managing debug messages
        ; from the host. However for normal applications, or ROM based
        ; applications the code called will be the user code. This
        ; file does not expect any other interface, other than when
        ; the initialisation is complete the routine "_main" will be
        ; called to enter the application "main" routine.
        ;
        ; This code *IS* single threaded. When Angel is being
        ; constructed as part of a multi-threaded application, then
        ; this initialisation code should be atomic. At the moment
        ; this is achieved by ensuring that this code is entered in
        ; SVC mode with processor interrupts disabled. If part of a
        ; ROM image, then the ROM boot-strapping code will ensure that
        ; this routine is entered with the correct mode and
        ; status. Similarly the new Angel run-time specifies that
        ; applications are always entered through |__main| in SVC mode
        ; with processor interrupts disabled. The run-time library
        ; then deals with switching to USR mode after its
        ; initialisation, and before starting the application. This is
        ; different from the old Demon method of always starting new
        ; applications in USR mode, but does allow applications in the
        ; new Angel world to have suitable priviledged control. This
        ; avoids the Angel run-time from having to know anything about
        ; the environment that started the Angel application, which in
        ; itself need *NOT* be based on Angel.
        ;
        ; This routine contains the generic Angel initialisation. Any
        ; ROM specific boot-strapping is held in the "startrom.s"
        ; source file.
        ;
        LDR     a1,=angel_ExceptionInit ; get address of function
        TEQ     a1,#0x00000000          ; if zero, it does not exist
        BLNE    angel_ExceptionInit     ; initialise the exception system
        ;
        ;
        ; The following calls should probably be build-time
        ; conditional. The Angel library being constructed may not
        ; require the device driver support, or any debug channel
        ; support. At the moment we just include run-time checks for
        ; the relevant functions being available.
        ;
        LDR     a1,=angel_InitialiseDevices     ; get address of function
        TEQ     a1,#0x00000000                  ; if zero, it does not exist
        BEQ     NoDeviceSupport                 ; cannot have comms support
        ;
        BL      angel_InitialiseDevices         ; init device driver world

        ;
        ; Now set the LED which is attached to one of the real devices
        ;

  IF :DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0
        LDR     a1,=Angel_RawDeviceControl
  ELSE
        LDR     a1,=angel_DeviceControl
  ENDIF
        TEQ     a1, #0
        BEQ     NoDevicesSoNoLEDs
        ;
        MOV     a1, #DI_LED_DEVICE
        MOV     a2, #DC_SET_LED
        MOV     a3, #1
        ;
  IF :DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0
        BL      Angel_RawDeviceControl
  ELSE
        BL      angel_DeviceControl
  ENDIF

        ;
NoDevicesSoNoLEDs
        ;
        LDR     a1,=angel_InitialiseChannels    ; get address of function
        TEQ     a1,#0x00000000                  ; if zero, it does not exist
        BEQ     NoChannelSupport                ; cannot have comms support
        ;
        BL      angel_InitialiseChannels        ; initialise the channel world
        ;
        ; NOTE: There is no error status return from the device
        ; initialisation routine. This means that each device driver
        ; must keep its own active (and initialised OK) state, with
        ; the users of the device driver dealing with individual
        ; device driver error returns.
        ;
        LDR     a1,=angel_DebugInit             ; check for debug agent
        TEQ     a1,#0x00000000                  ; if zero, it does not exist
        BLNE    angel_DebugInit                 ; initialise debugging world

        LDR     a1,=angelOS_Initialise          ; check for OS presence.
        TEQ     a1,#0x00000000                  ; if zero, it does not exist
        BLNE    angelOS_Initialise              ; initialise the OS.

        ;
        LDR     a1,=angel_SysLibraryInit        ; Check for C Library Support
        TEQ     a1,#0x00000000                  ; If zero, does not exist.
        BLNE    angel_SysLibraryInit
        
        LDR     a1,=Angel_ProfileTimerInitialise  ; Check for Profile / Yield
        TEQ     a1,#0x00000000                  ; Timer Support
        BLNE    Angel_ProfileTimerInitialise
        ;
        ; The Angel defined startup channels should now be initialised.
        ;
NoDeviceSupport
NoChannelSupport

        BL      Angel_EnterSVC
        BL      Angel_ExitToUSR ; This will reenable interrupts !

  IF :DEF: MINIMAL_ANGEL :LAND: MINIMAL_ANGEL<>0

        ; We just jump straight to the application's __entry point
        BL      __entry

AppFinished
        ADR     a1, appexitmsg
        B       __rt_asm_fatalerror
appexitmsg
        DCB     "Appl returned\n"
        ALIGN

  ELSE
        
        ; The boot agent should be the last part of the communications
        ; system to be initialised, because it sends the first message
        ; to the host. The host should not be generating *ANY*
        ; messages until it has received the "booted" status message
        ; from the target.

        LDR     a1,=angel_BootInit  ; check that boot agent exists
        TEQ     a1,#0x00000000      ; if zero, it does not exist
        BLNE    angel_BootInit      ; deal with starting bootstrap agent

        IF :DEF: LATE_STARTUP :LAND: LATE_STARTUP <> 0

          ; We're doing late startup, so as long as there is an entry point,
          ; simply unblock the application, and call Yield to let it run
        
          LDR     a1,=__entry
          TEQ     a1,#0x00000000
          BEQ     No__entry
          BL      Angel_InitialiseTaskFinished
          MOV     r0, #0        ; unblock
          BL      Angel_BlockApplication
          BL      Angel_Yield
AppDidNotRun
          B       AppDidNotRun  ; should never get here
        
No__entry
          ;; If it doesn't exist, we drop through
        
        ENDIF
        
        BL      Angel_InitialiseTaskFinished

        ;; Shouldn't get here more than once
        
BlockingLoop
        BL      Angel_Yield
        B       BlockingLoop

  ENDIF ; ELSE not minimal angel
                
        ; ---------------------------------------------------------------------
        ; -- Angel run-time workspace -----------------------------------------
        ; ---------------------------------------------------------------------
        ; Data areas should be declared as necessary. The old Demon source
        ; used assembler so no problem with C inter-working of the variables
        ; was required. However, the C-Demon work continued to use assembler
        ; "space" definitions. This meant a lot of "complicated" work when
        ; inter-working with the C portions of C-Demon. For Angel the simple
        ; approach of defining proper data AREAs in assembler, and ensuring
        ; that the C definitions are correct (to ensure data exists in the
        ; correct type of AREA) is used to ensure that clean assembler/C
        ; inter-working can occur.
        ;
        ; The following data area defines the layout of the Angel
        ; workspace. It does not specify where the work-space is
        ; placed in the final memory map. When Angel is being built as
        ; part of a ROM image, then the workspace lives at a fixed
        ; lo-memory address. When Angel is part of an application,
        ; then the workspace is placed in the standard application
        ; memory by the linker. This ensures that an Angel system
        ; loaded as part of an application is seperate to any version
        ; of Angel already present in a boot/debug ROM.

        AREA    |Angel$$DATA|,DATA,NOINIT
        EXPORT  angel_StartupStatus
        EXPORT  angel_MMUtype
angel_LibraryWorkspaceStart     %       0       ; start of this workspace section
        
        ; This is a status variable used to hold information on how
        ; Angel was started. For ROM systems this can include whether
        ; the system was started as a result of a PowerOnReset, or for
        ; library based systems it can be information on how the
        ; system was downloaded and initialised. Zero denotes a
        ; hard-reset, whilst a non-zero value denotes a soft-reset.
angel_StartupStatus             %       4       ; type of Angel reset

        
        ; This variable is used to denote whether the startup code
        ; discovered an ARM MMU unit. This is needed to ensure that
        ; certain routines can discover whether there is a cache
        ; present. i.e. memory sizing code needsd to ensure that the
        ; cache is disabled when checking for ghosting.
angel_MMUtype                   %       4       ; type of MMU present
                        
angel_LibraryWorkspaceEnd       %       0       ; allows size of area to be calculated

        ; ---------------------------------------------------------------------
        END     ; EOF startlib.s
