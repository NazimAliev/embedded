/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

#ifndef LW_AF_PRIV
#define LW_AF_PRIV

#include <ti/xdais/dm/iuniversal.h>
#include "afmath.h"

typedef struct LW_AF_Obj 
{
    IALG_Obj    alg;            /* MUST be first field of all XDAS algs */
    XDAS_Int16* twiddleBuf;
    XDAS_Int16* sigBuf;
    XDAS_Int16* refBuf;
    XDAS_UInt8* cfarBuf;
    XDAS_UInt16* afBuf;
    XDAS_UInt8* tmpBuf;
    XDAS_UInt8* tmp2Buf;
    XDAS_UInt32* matrixBuf;
    XDAS_UInt32 cfarLevel;
    XDAS_UInt16 logMode;
    XDAS_UInt16 alarms;
} LW_AF_Obj;

typedef struct LW_AF_Obj* LW_AF_Handle;

extern Int LW_AF_numAlloc(void);

extern Int LW_AF_alloc(const IALG_Params *algParams,
        IALG_Fxns **pf, IALG_MemRec memTab[]);

extern Int LW_AF_free(IALG_Handle handle, IALG_MemRec memTab[]);

extern Int LW_AF_initObj(IALG_Handle handle,
        const IALG_MemRec memTab[], IALG_Handle parent,
        const IALG_Params *algParams);

extern XDAS_Int32 LW_AF_process(IUNIVERSAL_Handle h,
        XDM1_BufDesc *inBufs, XDM1_BufDesc *outBufs, XDM1_BufDesc *inOutBufs,
        IUNIVERSAL_InArgs *inArgs, IUNIVERSAL_OutArgs *outArgs);

extern XDAS_Int32 LW_AF_control(IUNIVERSAL_Handle handle,
        IUNIVERSAL_Cmd id, IUNIVERSAL_DynamicParams *params,
        IUNIVERSAL_Status *status);

#endif

