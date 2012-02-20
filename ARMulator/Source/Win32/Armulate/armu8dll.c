/* RCS $Revision: 1.13 $
 * Checkin $Date: 1997/02/26 12:51:20 $
 * Revising $Author: hbullman $
 */

#include <windows.h>
#include <stdio.h>
#include "armu8dll.h"
#include "armu8dll.h"
#include "resource.h"

#include "winrdi.h"
#include "windebug.h"
#include "multirdi.h"

#ifdef PICCOLO
toolconf armul_config;
#else
static Dbg_ConfigBlock *armul_config;
#endif

typedef struct { char name[16]; unsigned val; } Processor;
void AssignConfig();

/*
 * (obsolete line, I think)
 * extern Processor const *const ARMul_Processors[];
 */
extern struct RDIProcVec armul_rdi;
#ifdef PICCOLO
extern struct RDIProcVec picul_rdi;
#endif

LRESULT CALLBACK ConfigDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

/*
 * IDJ - Added function to register callback function to be called back
 *      in VieldControl()
 *
 */

typedef void (*PFV)(void *);

static PFV pfnYield = NULL;
static void *hYield = NULL;

void WINAPI WinRDI_Register_Yield_Callback (PFV yieldcallback, void *hyieldcallback)
{
        pfnYield = yieldcallback;
        hYield   = hyieldcallback;
}


/*
 * SJ - In order for a DLL to be recognised as an RDI connection by ADW the
 *      following function is needed.
 */
BOOL WINAPI WinRDI_Valid_RDI_DLL(void)
{
    return TRUE;
}

/*
 * SJ - The following function returns a description of what this DLL does, to ADW.
 *      This will be displayed on the 'Debugger Configuration' Target property page.
 */
char* WINAPI WinRDI_Get_DLL_Description(void)
{
    /* Limited to around 200 characters */
    static char *msg = "Use the ARM Debugger with the 'ARMulator' Instruction "
                       "Set Simulator. This allows you to execute ARM programs "
                       "without physical ARM hardware, by simulating the ARM "
                       "instructions in software.";
    return msg ;
}

// JRP - added WinRDI stuff so DLL works with new Debugger
//
#ifdef PICCOLO
int WINAPI WinRDI_Config(toolconf config, HWND hParent)
#else
int WINAPI WinRDI_Config(Dbg_ConfigBlock *config, HWND hParent) 
#endif
{
    int ret;
    armul_config = config;

    ret = DialogBox (ghArmulateMod, MAKEINTRESOURCE(IDD_CONFIG), hParent, ConfigDlgProc);
    return ret;
}

RDIProcVec* WINAPI WinRDI_GetRDIProcVec(void)
{
   return (RDIProcVec *)&armul_rdi;
}
#ifdef PICCOLO
RDIProcVec* WINAPI WinRDI_GetRDIProcVec2(void)
{
   return (RDIProcVec *)&picul_rdi;
}
#endif
int WINAPI WinRDI_GetVersion(void)
{
   return WinRDI_Version;
}

/******************************************************************************\
*
*  FUNCTION:    DllMain
*
*  INPUTS:      hDLL       - handle of DLL
*               dwReason   - indicates why DLL called
*               lpReserved - reserved
*
*  RETURNS:     TRUE (always, in this example.)
*
*               Note that the retuRn value is used only when
*               dwReason = DLL_PROCESS_ATTACH.
*
*               Normally the function would return TRUE if DLL initial-
*               ization succeeded, or FALSE it it failed.
*
*  GLOBAL VARS: ghArmulateMod - handle of DLL (initialized when PROCESS_ATTACHes)
*
*  COMMENTS:    The function will display a dialog box informing user of
*               each notification message & the name of the attaching/
*               detaching process/thread. For more information see
*               "DllMain" in the Win32 API reference.
*
\******************************************************************************/

BOOL WINAPI DllMain (HANDLE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            //
            // DLL is attaching to the address space of the current process.
            //
            ghArmulateMod = hDLL;

#ifdef _DEBUG
            {
                char buf[BUFSIZE+1];
                GetModuleFileName (NULL, (LPTSTR) buf, BUFSIZE);
                ADBG("ARMul810", "ARMulator DLL: Process attaching - %s\n", buf);
            }
#endif
            break;
        }

        case DLL_THREAD_ATTACH:
        {
            //
            // A new thread is being created in the current process.
            //
            ADBG("ARMul810", "ARMulator DLL: Thread attaching\n");
            break;
        }
        case DLL_THREAD_DETACH:
        {
            //
            // A thread is exiting cleanly.
            //
            ADBG("ARMul810", "ARMulator DLL: Thread detaching\n");
            break;
        }
        case DLL_PROCESS_DETACH:
        {
            //
            // The calling process is detaching the DLL from its address space.
            //
            ADBG("ARMul810", "ARMulator DLL: Process detaching\n");
            break;
        }
    }
    return TRUE;
}

/**************************************************************************\
*
*  function:  MainDlgProc()
*
*  input parameters:  standard window procedure parameters.
*
\**************************************************************************/

LRESULT CALLBACK ConfigDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        /********************************************************************\
        * WM_INITDIALOG
        \********************************************************************/
        case WM_INITDIALOG:
        {
            int nProcessor, nRes, nProc = 0, nItem;
            extern int ARMul_DefaultProcessor; /* becomes valid after call to RDI_cpunames */
            char szClockSpeed[20];
            int fpe, bytesex, cpu_speed;
            RDI_NameList const* theList = ARMul_RDI_cpunames();

#ifdef PICCOLO
            char const *str_bytesex, *processor_name;
#endif

#ifdef PICCOLO
            fpe = ToolConf_DLookupBool(armul_config, Dbg_Cnf_FPE, TRUE);
            
            str_bytesex = ToolConf_Lookup(armul_config, Dbg_Cnf_ByteSex);
            if (str_bytesex != NULL && str_bytesex[0]=='L')
                bytesex = 0;
            else
                bytesex = 1;

            cpu_speed = ToolConf_DLookupInt(armul_config,Dbg_Cnf_CPUSpeed,0);

            processor_name = ToolConf_Lookup(armul_config,Dbg_Cnf_Processor);

            nItem = ARMul_DefaultProcessor;
#else
            fpe = armul_config->fpe;
            bytesex = armul_config->bytesex;
            cpu_speed = armul_config->cpu_speed;
            nItem = (armul_config->processor ?
                                         armul_config->processor :
                                         ARMul_DefaultProcessor);
#endif


            // Fill the drop-down list with the processor names


            for (nProcessor = 0; nProcessor < theList->itemmax; nProcessor++)
            {
                nRes = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_ADDSTRING, 0,
                                           (LPARAM) (LPCTSTR)theList->names[nProcessor]);
#ifdef PICCOLO
                if( (processor_name != NULL) && 
                    !strcmp(processor_name, theList->names[nProcessor]))
                    nItem = nProcessor;
#endif
            }

            nRes = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_SETCURSEL,
                                       (WPARAM)nItem, (LPARAM)0);
              

            // Set the Endian radio button
            if (bytesex)
                nRes = SendDlgItemMessage (hwnd, IDC_BIG_ENDIAN, BM_SETCHECK, (WPARAM)1, (LPARAM)0);
            else
                nRes = SendDlgItemMessage (hwnd, IDC_ENDIAN, BM_SETCHECK, (WPARAM)1, (LPARAM)0);

            // Set the FPE checkbox
            if (fpe)
                nRes = SendDlgItemMessage (hwnd, IDC_FPE, BM_SETCHECK, (WPARAM)1, (LPARAM)0);
            else
                nRes = SendDlgItemMessage (hwnd, IDC_FPE, BM_SETCHECK, (WPARAM)0, (LPARAM)0);

            // Set the clockspeed editbox
            sprintf(szClockSpeed, "%5.2f", (double)((double)cpu_speed / (double)1000000.00));
            nRes = SendDlgItemMessage (hwnd, IDC_CLOCK_SPEED, WM_SETTEXT, (WPARAM)0, (LPARAM)(LPCSTR)szClockSpeed);
            return TRUE;
        }

        /********************************************************************\
        * WM_SYSCOMMAND
        \********************************************************************/
        case WM_SYSCOMMAND:
        {
            if (wParam == SC_CLOSE)
            {
                EndDialog (hwnd, TRUE);
                return TRUE;
            }
            else
                return FALSE;
        }
        break;

        /********************************************************************\
        * WM_COMMAND
        *
        * When the different buttons are hit, clear the list box, disable
        *  updating to it, call the function which will fill it, reenable
        *  updating, and then force a repaint.
        *
        \********************************************************************/
        case WM_COMMAND:
        {
            /* if the list box sends back messages, return.  */
            if (LOWORD(wParam)==IDC_PROCESSOR)
                return TRUE;

            /* switch on the control ID of the button that is pressed. */
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    AssignConfig(hwnd);
                    EndDialog (hwnd, TRUE);
                    return TRUE;
                }
                case IDCANCEL:
                {
                    EndDialog (hwnd, FALSE);
                    return TRUE;
                }
            }
            return TRUE;
            break; /* end WM_COMMAND */
        }
        default:
            return FALSE;

    } /* end switch(message) */

    return FALSE;
}

#ifdef PICCOLO

void AssignConfig(HWND hwnd)
{
        char szClockSpeed[20];
        int nProc, nByteSex;

        RDI_NameList const* theList = ARMul_RDI_cpunames();

        nProc = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_GETCURSEL,   (WPARAM)0, (LPARAM)0);
        if (nProc == CB_ERR)
                nProc = 0;
        ToolConf_UpdateTagged(armul_config, Dbg_Cnf_Processor, (char *) theList->names[nProc]);

        // Get the Endian radio button
        nByteSex = SendDlgItemMessage (hwnd, IDC_BIG_ENDIAN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);
        ToolConf_UpdateTagged(armul_config, Dbg_Cnf_ByteSex, (nByteSex) ? "B" : "L");

        // Get the FPE checkbox
        ToolConf_UpdateTagged(armul_config, Dbg_Cnf_FPE,
                   SendDlgItemMessage (hwnd, IDC_FPE, BM_GETCHECK, (WPARAM)0, (LPARAM)0)
                   ? "TRUE" : "FALSE");


        // Get the clockspeed editbox
        SendDlgItemMessage(hwnd, IDC_CLOCK_SPEED, WM_GETTEXT, (WPARAM)20, (LPARAM)szClockSpeed);
        ToolConf_UpdateTagged(armul_config, Dbg_Cnf_CPUSpeed, szClockSpeed);
}
#else
void AssignConfig(HWND hwnd)
{
        double dClock;
        char szClockSpeed[20];
        int nProc;

        nProc = SendDlgItemMessage (hwnd, IDC_PROCESSOR,  CB_GETCURSEL,   (WPARAM)0, (LPARAM)0);
        if (nProc == CB_ERR)
                nProc = 0;
        armul_config->processor = nProc;

        // Get the Endian radio button
        armul_config->bytesex = SendDlgItemMessage (hwnd, IDC_BIG_ENDIAN, BM_GETCHECK, (WPARAM)0, (LPARAM)0);

        // Get the FPE checkbox
        armul_config->fpe = SendDlgItemMessage (hwnd, IDC_FPE, BM_GETCHECK, (WPARAM)0, (LPARAM)0);

        // Get the clockspeed editbox
        SendDlgItemMessage(hwnd, IDC_CLOCK_SPEED, WM_GETTEXT, (WPARAM)20, (LPARAM)szClockSpeed);
        sscanf(szClockSpeed, "%lf", &dClock);
        armul_config->cpu_speed = (UINT)(dClock * 1000000);
}
#endif

/*
int FAR PASCAL RDI_Config(Dbg_ConfigBlock *config)
{
    int ret;
    armul_config = config;

    ret = DialogBox (ghArmulateMod, MAKEINTRESOURCE(IDD_CONFIG), NULL, (DLGPROC)ConfigDlgProc);
        return ret;
}
*/

static char *armul_rdi_names[] = {
    "ARMUL",
    "ARMul_RDI_open",
    "ARMul_RDI_close",
    "ARMul_RDI_read",
    "ARMul_RDI_write",
    "ARMul_RDI_CPUread",
    "ARMul_RDI_CPUwrite",
    "ARMul_RDI_CPread",
    "ARMul_RDI_CPwrite",
    "ARMul_RDI_setbreak",
    "ARMul_RDI_clearbreak",
    "ARMul_RDI_setwatch",
    "ARMul_RDI_clearwatch",
    "ARMul_RDI_execute",
    "ARMul_RDI_step",
    "ARMul_RDI_info",
    NOT_IMPLEMENTED, //"pointinq"
    NOT_IMPLEMENTED, //"addconfig"
    NOT_IMPLEMENTED, //"loadconfigdata"
    NOT_IMPLEMENTED, //"selectconfig"
    NOT_IMPLEMENTED, //"drivernames"
    "ARMul_RDI_cpunames",
    "ARMul_RDI_errmess",
        NOT_IMPLEMENTED, //"loadagent"
};
/*
int FAR PASCAL RDI_Get(char *szItem, int nItem)
{
        strncpy(szItem, armul_rdi_names[nItem], 254);
        return TRUE;
}
*/
/******************************************************************************\
*
*  FUNCTION: Utils
*
*  RETURNS:  ARMulator DLL Utility functions (not exported)
*
\******************************************************************************/

void YieldControl(int nLoops)
{
        MSG Message;
        int loop = 0;

          
        while (loop < nLoops)
        {
                                if (pfnYield != NULL)
                                pfnYield(hYield);
 
                if (PeekMessage(&Message, NULL, 0,0, PM_REMOVE))
                {
                        TranslateMessage(&Message);
                        DispatchMessage(&Message);
                }
                loop++;
        }
}

void armsd_hourglass(void)
{
        YieldControl(1); // This could be Selected By Options
}

