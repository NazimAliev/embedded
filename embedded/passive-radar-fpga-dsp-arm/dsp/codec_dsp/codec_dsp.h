/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF_UC
#define LW_AF_UC 

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  ======== UNIVERSALCOPY_TI_IUNIVERSALCOPY ========
 *  Our implementation of the IUNIVERSAL interface
 */
extern IUNIVERSAL_Fxns LW_AF_I;
extern IALG_Fxns LW_AF_IALG;

typedef struct
{
    IUNIVERSAL_InArgs universal; ///< must be first field according to XDM
	XDAS_UInt32 cfarLevel;
	XDAS_UInt16 logMode;

} LW_AF_InArgs;

typedef struct
{
    IUNIVERSAL_OutArgs universal; ///< must be first field according to XDM
	XDAS_UInt16 alarms;

} LW_AF_OutArgs;


#ifdef __cplusplus
}
#endif

#endif
