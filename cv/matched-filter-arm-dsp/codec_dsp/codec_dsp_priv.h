/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

// codec_dsp_priv.h

#ifndef LW_AF_PRIV
#define LW_AF_PRIV

#include <ti/xdais/dm/iuniversal.h>
#include "mfilter.h"

// store all our pointers together to pass to data processing functions
// note: in/out data memory allocated by app.c
// DSP internal buffers allocated in init_Obj
typedef struct LW_AF_Obj 
{
    IALG_Obj    alg;            /* MUST be first field of all XDAS algs */
	// memory was allocated in app.c
    XDAS_Int16* inBufSig;	//(int16_t*) inBufs->descs[0].buf
    XDAS_Int16* inBufRef;	//(int16_t*) inBufs->descs[1].buf
    XDAS_Int16* inBufImgSig;	//(int8_t*) inBufs->descs[2].buf
    XDAS_Int16* inBufImgRef;	//(int8_t*) inBufs->descs[3].buf
    XDAS_Int16* outBufFft;	//(int16_t*) outBufs->descs[0].buf
    XDAS_Int16* outBufImg;	//(int16_t*) outBufs->descs[1].buf
	// memory was allocated in init_Obj
	// memTab[0] reserved for LW_AF_Obj
    XDAS_Int16* bufTwiddle;	//(XDAS_Int16*)memTab[1].base
    XDAS_UInt8* bufTmp1;		//(XDAS_Int16*)memTab[2].base
    XDAS_UInt8* bufTmp2;	//(XDAS_Int16*)memTab[3].base
    XDAS_UInt8* bufImgRef;	//(XDAS_Int16*)memTab[4].base
    XDAS_UInt8* bufImgTmp1;	//(XDAS_Int16*)memTab[5].base
    XDAS_UInt8* bufImgTmp2;	//(XDAS_Int16*)memTab[6].base
	// parms
    XDAS_UInt32 inParmU32;
    XDAS_UInt16 inParmU16;
    XDAS_UInt16 outParmU16;
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

