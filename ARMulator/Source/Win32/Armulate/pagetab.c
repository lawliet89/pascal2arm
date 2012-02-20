/* module to provide page tables
 *
 * RCS $Revision: 1.4 $
 * Checkin $Date: 1997/03/25 13:07:52 $
 * Revising $Author: mwilliam $
 */

#include "armdefs.h"
#include "armcnf.h"

enum access {
  NO_ACCESS,
  NO_USR_W,
  SVC_RW,
  ALL_ACCESS
  };

enum entrytype {
  INVALID,
  PAGE,
  SECTION
  };

#define U_BIT 16
#define C_BIT 8
#define B_BIT 4

#define L1Entry(type,addr,dom,ucb,acc) \
  ((type==SECTION) ? (((addr)&0xfff00000)|((acc)<<10)|  \
                      ((dom)<<5)|(ucb)|(type)) :        \
   (type==PAGE) ? (((addr)&0xfffffc00)|((dom)<<5)|      \
                   ((ucb)&U_BIT)|(type)) :              \
   0)

static struct {
  tag_t option;
  ARMword bit;
  int def;
} ctl_flags[] = {
  ARMulCnf_MMU,         MMU_M, TRUE,
  ARMulCnf_AlignFaults, MMU_A, FALSE,
  ARMulCnf_Cache,       MMU_C, TRUE,
  ARMulCnf_WriteBuffer, MMU_W, TRUE,
  ARMulCnf_Prog32,      MMU_P, TRUE,
  ARMulCnf_Data32,      MMU_D, TRUE,
  ARMulCnf_LateAbort,   MMU_L, TRUE,
  ARMulCnf_BigEnd,      MMU_B, FALSE,
  ARMulCnf_SystemProt,  MMU_S, FALSE,
  ARMulCnf_ROMProt,     MMU_R, FALSE,
  ARMulCnf_BranchPredict, MMU_Z, TRUE,
  ARMulCnf_ICache,      MMU_I, TRUE
  };

typedef struct {
  toolconf clone;
  ARMul_State *state;
} pagetab;

static void InterruptHandler(void *handle,unsigned int which)
{
  pagetab *tab = (pagetab *)handle;
  toolconf config = tab->clone;
  ARMul_State *state = tab->state;
  const char *option;
  ARMword value;
  ARMword page;
  ARMword entry;
  unsigned int i;

  if (!(which & ARMul_InterruptUpcallReset))
    return;                     /* we only care about reset */

  value=ToolConf_DLookupUInt(config, ARMulCnf_DAC, 0x3);
  ARMul_CPWrite(state,15,3,&value);
  
  value=ToolConf_DLookupUInt(config, ARMulCnf_PageTableBase, 0x40000000);
  value&=0xffffc000;            /* align */
  ARMul_CPWrite(state,15,2,&value);
  
  /* start with all pages flat, uncacheable, read/write, unbufferable */
  entry=L1Entry(SECTION,0,0,U_BIT|C_BIT|B_BIT,ALL_ACCESS);
  for (i=0, page=value; i<4096; i++, page+=4) {
    ARMul_WriteWord(state, page, entry | (i<<20));
  }

  /* look to see if any regions are defined on top of this */
  for (i=1; i<100; i++) {
    char buffer[32];
    ARMword low,physical,mask;
    ARMword page;
    toolconf region;
    int j,n;
      
    sprintf(buffer,"REGION[%d]",i);
    region=ToolConf_Child(config,(tag_t)buffer);
    if (region==NULL) break;    /* stop */

    option=ToolConf_Lookup(region,ARMulCnf_VirtualBase);
    if (option) {
      low=option ? strtoul(option,NULL,0) : 0;
      physical=ToolConf_DLookupUInt(region, ARMulCnf_PhysicalBase, low);
    } else {
      low=physical=ToolConf_DLookupUInt(region, ARMulCnf_PhysicalBase,0);
    }

    n=ToolConf_DLookupUInt(region, ARMulCnf_Pages,4096);

    mask=0;
    option=ToolConf_Lookup(region,ARMulCnf_Cacheable);
    mask=ToolConf_AddFlag(option,mask,C_BIT,TRUE);
    option=ToolConf_Lookup(region,ARMulCnf_Bufferable);
    mask=ToolConf_AddFlag(option,mask,B_BIT,TRUE);
    option=ToolConf_Lookup(region,ARMulCnf_Updateable);
    mask=ToolConf_AddFlag(option,mask,U_BIT,TRUE);

    mask |= (ToolConf_DLookupUInt(region, ARMulCnf_Domain, 0) & 0xf) << 5;
    mask |= (ToolConf_DLookupUInt(region, ARMulCnf_AccessPermissions,
                                  ALL_ACCESS) & 3) << 10;

    /* set TRANSLATE=NO to generate translation faults */
    if (ToolConf_DLookupBool(region, ARMulCnf_Translate, TRUE))
      mask |= SECTION;
    
    low&=0xfff00000;
    mask=(physical & 0xfff00000) | (mask & 0xdfe);
    
    j=low>>20;          /* index of first section */
    n+=j; if (n>4096) n=4096;
    for (page=(value+j*4); j<n; j++, mask+=0x1000, page+=4) {
      ARMul_WriteWord(state,page,mask);
    }
  }

  /* now enable the caches, etc. */
  value=0;
  for (i=0;i<sizeof(ctl_flags)/sizeof(ctl_flags[0]);i++) {
    option=ToolConf_Lookup(config,ctl_flags[i].option);
    if (option)
      value=ToolConf_AddFlag(option,value,ctl_flags[i].bit,TRUE);
    else if (ctl_flags[i].def)
      value|=ctl_flags[i].bit;
  }

  if (value & (MMU_C | MMU_B)) value|=MMU_M; /* always enable the MMU too */

  ARMul_CPWrite(state,15,1,&value); /* enable the MMU etc. */
}

static void PagetabFree(void *handle)
{
  pagetab *tab = (pagetab *)handle;
  ToolConf_Reset(tab->clone);
  free(tab);
}

static ARMul_Error PagetabInit(ARMul_State *state, toolconf config)
{
  pagetab *tab;
  ARMul_PrettyPrint(state,", Pagetables");

  tab = (pagetab *)malloc(sizeof(*tab));
  if (tab) tab->clone = ToolConf_Clone(config);
  if (tab && tab->clone) {
    tab->state = state;
    ARMul_InstallInterruptHandler(state, InterruptHandler, tab);
    ARMul_InstallExitHandler(state, PagetabFree, tab);
    return ARMulErr_NoError;
  }
  return ARMul_RaiseError(state, ARMulErr_OutOfMemory);
}


const ARMul_ModelStub ARMul_Pagetable = {
  PagetabInit,
  (tag_t)"Pagetables"
  };
