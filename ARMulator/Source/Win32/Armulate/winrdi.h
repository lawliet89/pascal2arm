
/* 
 * winrdi.h:

 * Copyright (C) 1997 Advanced Risc Machines Ltd. All rights reserved.

 * Windows-specific RDI interfaces */

/*
 * RCS $Revision: 1.11.4.1 $
 * Checkin $Date: 1998/03/12 17:27:18 $
 * Revising $Author: rphelan $
 */

#ifndef __winrdi_h
#define __winrdi_h

#include "toolconf.h"

enum
{
    WinRDI_Version = 1   // DLL interface version number
};

// WinRDI_Download_... download bitflags 
enum
{
    WinRDI_DownloadStartup  = 1,
    WinRDI_DownloadImage    = 2
};

typedef struct WinRDI_ProgressInfo
{
    void *handle;
    int nRead;     // number of bytes read from channel
    int nWritten;  // number of bytes written
} WinRDI_ProgressInfo;

// The callback definitions...
#ifdef PICCOLO
typedef int         (WINAPI *WinRDI_funcInitialise)          (HWND hParent, toolconf);
typedef BOOL        (WINAPI *WinRDI_funcConfig)              (toolconf config, HWND hParent);
#else
typedef int         (WINAPI *WinRDI_funcInitialise)          (HWND hParent, Dbg_ConfigBlock*);//toolconf);
typedef BOOL        (WINAPI *WinRDI_funcConfig)              (Dbg_ConfigBlock *dbgcfg, HWND hParent);//, toolconf config);
#endif /* else PICCOLO */

typedef int         (WINAPI *WinRDI_funcConsoleInitialise)   (char *drivername, char *args, int heartbeat);
typedef int         (WINAPI *WinRDI_funcGetVersion)          (void);
typedef RDIProcVec *(WINAPI *WinRDI_funcGetRDIProcVec)       (void);

#ifdef PICCOLO
typedef RDIProcVec *(WINAPI *WinRDI_funcGetRDIProcVec2)       (void);
#endif /* piccolo */

typedef int         (WINAPI *WinRDI_funcDownload)            (int *options, const char *filename);
typedef void        (WINAPI *WinRDI_funcProgressFunc)        (WinRDI_ProgressInfo *info);
typedef void        (WINAPI *WinRDI_funcSetProgressFunc)     (WinRDI_funcProgressFunc func, void *handle);
typedef void        (WINAPI *WinRDI_funcZeroProgressValues)  (void);
typedef BOOL        (WINAPI *WinRDI_funcIsIdleProcessing)    (void);
typedef int         (WINAPI *WinRDI_funcIdle)                (void);
typedef BOOL        (WINAPI *WinRDI_funcIsProcessingSWI)     (void);
typedef void        (WINAPI *WinRDI_funcSetStopping)         (BOOL stopping);
typedef char*       (WINAPI *WinRDI_funcFlashDLAvailable)    (void);

typedef BOOL        (WINAPI *WinRDI_funcValid_RDI_DLL)       (void);
typedef char*       (WINAPI *WinRDI_funcGet_DLL_Description) (void);
typedef void        (*PFV)                                   (void *handle);
typedef void        (WINAPI *WinRDI_funcRegister_Yield_Callback) (PFV yieldcallback, void *hyieldcallback);



/*
 * The WinRDI interface supported by any DLL providing an RDI interface
 */
#ifdef PICCOLO
int WINAPI WinRDI_Initialise(HWND hParent, toolconf conf);
RDIProcVec *WINAPI WinRDI_GetRDIProcVec2(void);
#else
int WINAPI WinRDI_Initialise(HWND hParent, Dbg_ConfigBlock *config);
#endif /* else PICCOLO */

int WINAPI WinRDI_ConsoleInitialise(char *drivername, char *args, int heartbeat);
RDIProcVec *WINAPI WinRDI_GetRDIProcVec(void);
int WINAPI WinRDI_GetVersion(void);

/* RDI configuration function - takes a ToolConf handle as the parameter and
 *                              parent window handle.
 * - Removed toolconf for the moment!
 */
BOOL WINAPI WinRDI_Config(Dbg_ConfigBlock *config, HWND hParent);

/* called by WinDbg to request download of startup code or image or both
 * For options see WinRDI_Download_... above
 * 'filename' is the image file to download (ignored if ..._image not set)
 */
int WINAPI WinRDI_Download(int *options, const char *filename);

void WINAPI WinRDI_ProgressFunc(WinRDI_ProgressInfo *info);
void WINAPI WinRDI_SetProgressFunc(WinRDI_funcProgressFunc func, void *handle);
void WINAPI WinRDI_ZeroProgressValues(void);
BOOL WINAPI WinRDI_IsIdleProcessing(void);
int  WINAPI WinRDI_Idle(void);
BOOL WINAPI WinRDI_IsProcessingSWI(void);
void WINAPI WinRDI_SetStopping(BOOL stopping);

// find out whether we can use Flash Download...
char* WINAPI WinRDI_FlashDLAvailable(void);

char* WINAPI WinRDI_Get_DLL_Description(void);
BOOL  WINAPI WinRDI_Valid_RDI_DLL(void);
void  WINAPI WinRDI_Register_Yield_Callback(PFV yieldcallback, void *hyieldcallback);


// exports from windows-based RDI DLLs
#define WinRDI_strInitialise          "WinRDI_Initialise"
#define WinRDI_strGetVersion          "WinRDI_GetVersion"        
#define WinRDI_strGetRDIProcVec       "WinRDI_GetRDIProcVec"

#ifdef PICCOLO
#define WinRDI_strGetRDIProcVec2       "WinRDI_GetRDIProcVec2"
#endif

#define WinRDI_strConfig              "WinRDI_Config"
#define WinRDI_strDownload            "WinRDI_Download"
#define WinRDI_strSetProgressFunc     "WinRDI_SetProgressFunc"
#define WinRDI_strZeroProgressValues  "WinRDI_ZeroProgressValues"
#define WinRDI_strIsIdleProcessing    "WinRDI_IsIdleProcessing" 
#define WinRDI_strIdle                "WinRDI_Idle"              
#define WinRDI_strIsProcessingSWI     "WinRDI_IsProcessingSWI"   
#define WinRDI_strSetStopping         "WinRDI_SetStopping"
#define WinRDI_strFlashDLAvailable    "WinRDI_FlashDLAvailable"
#define WinRDI_strConsoleInitialise   "WinRDI_ConsoleInitialise"

#define WinRDI_strGet_DLL_Description "WinRDI_Get_DLL_Description"
#define WinRDI_strValid_RDI_DLL       "WinRDI_Valid_RDI_DLL"
#define WinRDI_strRegister_Yield_Callback "WinRDI_Register_Yield_Callback"

// macros to return the addresses of standard functions using GetProcAddress
#define WinRDI_GetProcAddress(handle,name) (WinRDI_func##name)GetProcAddress (handle, WinRDI_str##name)

#endif
