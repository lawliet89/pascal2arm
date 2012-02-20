/*
 * tool version numbers file - in one place for easier updating.
 * Copyright (C) 1996 Advanced RISC Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.19.2.13 $
 * Checkin $Date: 1998/03/11 17:23:09 $
 * Revising $Author: ijohnson $
 */

#ifndef toolver_h
#define toolver_h

#ifdef TOOLVER_RELEASE

/* for use when all tools are labelled with the release version number */
/* may need to think about patching - what if not all tools are updated? */
#define TOOLVER_TCC       TOOLVER_RELEASE
#define TOOLVER_ARMCC     TOOLVER_RELEASE
#define TOOLVER_ARMCPP    TOOLVER_RELEASE
#define TOOLVER_ARMSD     TOOLVER_RELEASE
#define TOOLVER_ARMASM    TOOLVER_RELEASE
#define TOOLVER_ARMLIB    TOOLVER_RELEASE
#define TOOLVER_ARMLINK   TOOLVER_RELEASE
#define TOOLVER_ARMUL     TOOLVER_RELEASE
#define TOOLVER_SARMUL    TOOLVER_RELEASE
#define TOOLVER_DECAOF    TOOLVER_RELEASE
#define TOOLVER_DECAXF    TOOLVER_RELEASE
#define TOOLVER_RECONFIG  TOOLVER_RELEASE
#define TOOLVER_TOPCC     TOOLVER_RELEASE
#define TOOLVER_ARMPROF   TOOLVER_RELEASE
#define TOOLVER_AIF2HEX   TOOLVER_RELEASE
#define TOOLVER_ARMMAKE   TOOLVER_RELEASE
#define TOOLVER_ANGEL     TOOLVER_RELEASE
#define TOOLVER_ICEMAN2   TOOLVER_RELEASE

#else /* TOOLVER_RELEASE */

/* #ifndef ARM_RELEASE
# define ARM_RELEASE "unreleased"
#endif

... removed for SDT 2.11 release

*/

#define ARM_RELEASE "SDT 2.11a"

#ifdef SUPPLIER_RELEASE
# define SUPPLIER_INFO "(Advanced RISC Machines " ARM_RELEASE "/" SUPPLIER_RELEASE ")"
#else
# define SUPPLIER_INFO "(Advanced RISC Machines " ARM_RELEASE ")"
#endif

/*
 * This was changed so we can redefine version numbers for specific versions of specifc
 * tools easily. -- RIC 11/1997.
 */
#define TOOLNUM_AIF2HEX   "1.08"
#define TOOLNUM_ANGEL     "1.04"  
#define TOOLNUM_ARMASM    "2.40"
#define TOOLNUM_ARMCC     "4.76"
#define TOOLNUM_ARMCPP    "0.44/C" TOOLNUM_ARMCC
#define TOOLNUM_ARMLIB    "4.41"
#define TOOLNUM_ARMLINK   "5.11"
#define TOOLNUM_ARMMAKE   "4.08"
#define TOOLNUM_ARMPROF   "1.08"
#define TOOLNUM_ARMSD     "4.58"
#define TOOLNUM_ARMUL     "2.07"
#define TOOLNUM_DECAOF    "4.08"
#define TOOLNUM_DECAXF    "1.05"
#define TOOLNUM_ICEMAN2   "2.07"  
#define TOOLNUM_RECONFIG  "2.10"
#define TOOLNUM_SARMUL    "2.07"
#define TOOLNUM_TCC       "1.10"
#define TOOLNUM_TOPCC     "3.34"

/*
 * Insert any changes for special versions here, with a description of the product in
 * which they appear, if not readily apparent:
 */

#ifdef HACK_FOR_SLOW_TARGETS
/*
 * Special IceAgent which copes with slow (kilohertz-speed) target processors
 */
# undef TOOLNUM_ICEMAN2 
# define TOOLNUM_ICEMAN2  "2.07a"
#endif


#define TOOLVER_AIF2HEX   TOOLNUM_AIF2HEX " " SUPPLIER_INFO
#define TOOLVER_ANGEL     TOOLNUM_ANGEL " " SUPPLIER_INFO
#define TOOLVER_ARMASM    TOOLNUM_ARMASM " " SUPPLIER_INFO /* also used for tasm */
#define TOOLVER_ARMCC     TOOLNUM_ARMCC " " SUPPLIER_INFO
#define TOOLVER_ARMCPP    TOOLNUM_ARMCPP " " SUPPLIER_INFO
#define TOOLVER_ARMLIB    TOOLNUM_ARMLIB " " SUPPLIER_INFO
#define TOOLVER_ARMLINK   TOOLNUM_ARMLINK " " SUPPLIER_INFO
#define TOOLVER_ARMMAKE   TOOLNUM_ARMMAKE " " SUPPLIER_INFO
#define TOOLVER_ARMPROF   TOOLNUM_ARMPROF " " SUPPLIER_INFO
#define TOOLVER_ARMSD     TOOLNUM_ARMSD " " SUPPLIER_INFO
#define TOOLVER_ARMUL     TOOLNUM_ARMUL                /* no ARM needed */
#define TOOLVER_DECAOF    TOOLNUM_DECAOF " " SUPPLIER_INFO
#define TOOLVER_DECAXF    TOOLNUM_DECAXF " " SUPPLIER_INFO
#define TOOLVER_ICEMAN2   TOOLNUM_ICEMAN2 " " SUPPLIER_INFO
#define TOOLVER_RECONFIG  TOOLNUM_RECONFIG " " SUPPLIER_INFO
#define TOOLVER_SARMUL    TOOLNUM_SARMUL                /* no ARM needed */
#define TOOLVER_TCC       TOOLNUM_TCC " " SUPPLIER_INFO
#define TOOLVER_TOPCC     TOOLNUM_TOPCC " " SUPPLIER_INFO

#endif /* TOOLVER_RELEASE */

#endif /* toolver_h */
