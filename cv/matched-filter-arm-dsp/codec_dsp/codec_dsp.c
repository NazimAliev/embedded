/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

// codec_dsp.c

#include "../mf.h"
#include "mfilter.h"
#include <xdc/std.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Diags.h>
#include <ti/dsplib/dsplib.h>
#include <string.h>

#include <ti/xdais/dm/iuniversal.h>

#ifdef __TI_COMPILER_VERSION__
/* XDAIS Rule 13 - this #pragma should only apply to TI codegen */
#pragma CODE_SECTION(LW_AF_control, ".text:algControl")
#pragma CODE_SECTION(LW_AF_process, ".text:algProcess")
#pragma CODE_SECTION(LW_AF_initObj, ".text:algInit")
#pragma CODE_SECTION(LW_AF_free,    ".text:algFree")
#pragma CODE_SECTION(LW_AF_alloc,   ".text:algAlloc")
#pragma CODE_SECTION(LW_AF_numAlloc,   ".text:algNumAlloc")
#endif

#include "codec_dsp.h"
#include "codec_dsp_priv.h"

/* TODO, need to autogenerate this */
#define VERSIONSTRING "1.00.00.00"

/* Helper definitions */
#define BITSPERBYTE     8       /* number of bits in a byte */

#define IALGFXNS  \
    &LW_AF_IALG,  /* module ID */                         \
    NULL,                       /* activate */                          \
    LW_AF_alloc,  /* alloc */                             \
    NULL,                       /* control (NULL => no control ops) */  \
    NULL,                       /* deactivate */                        \
    LW_AF_free,   /* free */                              \
    LW_AF_initObj,/* init */                              \
    NULL,                       /* moved */                             \
    LW_AF_numAlloc              /* number of memTabs required (NULL => IALG_MAXMEMRECS) */

#define NUMBUFS 7

IUNIVERSAL_Fxns LW_AF_I = 
{
    {IALGFXNS},
    LW_AF_process,
    LW_AF_control,
};

#ifdef __TI_COMPILER_VERSION__
/* satisfy XDAIS symbol requirement without any overhead */
asm("LW_AF_IALG .set LW_AF_I");

#else

/*
 *  We duplicate the structure here to allow this code to be compiled and
 *  run using non-TI toolchains at the expense of unnecessary data space
 *  consumed by the definition below.
 */
IALG_Fxns LW_AF_IALG = 
{
    /* module_vendor_interface */
    IALGFXNS
};

#endif

IUNIVERSAL_Params LW_AF_PARAMS = 
{
    sizeof(IUNIVERSAL_Params),       /* size */
};

Int LW_AF_numAlloc(void)
{
	Log_print2(Diags_USER1, "[%d] %s", 0, (IArg)"numAlloc");
    return(NUMBUFS);
}
/*
 * Request memory only for DSP working buffers!
 * Memory for input/output data ARM <-> DSP already allocated in app.c
 */
Int LW_AF_alloc(const IALG_Params *algParams,
    IALG_Fxns **pf, IALG_MemRec memTab[])
{
	Log_print2(Diags_USER1, "[%d] %s", 1, (IArg)"alloc");
    /* Request memory for my object */
    memTab[0].size = sizeof(LW_AF_Obj);
    memTab[0].alignment = 0;
    memTab[0].space = IALG_EXTERNAL;
    memTab[0].attrs = IALG_PERSIST;

    /* Request memory for twiddle buf */
    memTab[1].size = BUFSIZE_1D;
    memTab[1].alignment = 8;
    memTab[1].space = IALG_EXTERNAL;
    memTab[1].attrs = IALG_PERSIST;

    /* Максимальное использование временных буферов - NSAMPLES комплексных int */
    /* Request memory for tmp buf */
    memTab[2].size = BUFSIZE_1D;
    memTab[2].alignment = 8;
    memTab[2].space = IALG_EXTERNAL;
    memTab[2].attrs = IALG_PERSIST;

    /* Request memory for tmp2 buf */
    memTab[3].size = BUFSIZE_1D;
    memTab[3].alignment = 8;
    memTab[3].space = IALG_EXTERNAL;
    memTab[3].attrs = IALG_PERSIST;

    /* Request memory img tmp1 buf */
    memTab[4].size = BUFSIZE_2D;
    memTab[4].alignment = 8;
    memTab[4].space = IALG_EXTERNAL;
    memTab[4].attrs = IALG_PERSIST;

    /* Request memory for img tmp2 buf */
    memTab[5].size = BUFSIZE_2D;
    memTab[5].alignment = 8;
    memTab[5].space = IALG_EXTERNAL;
    memTab[5].attrs = IALG_PERSIST;

    /* Request memory for img tmp3 buf */
    memTab[6].size = BUFSIZE_2D;
    memTab[6].alignment = 8;
    memTab[6].space = IALG_EXTERNAL;
    memTab[6].attrs = IALG_PERSIST;

    return(NUMBUFS);
}

Int LW_AF_free(IALG_Handle handle, IALG_MemRec memTab[])
{
    return (LW_AF_alloc(NULL, NULL, memTab));
}


// DSP memory has allocated, assign pointers
Int LW_AF_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
    IALG_Handle p, const IALG_Params *algParams)
{
	Log_print2(Diags_USER1, "[%d] %s", 2, (IArg)"initObj");
    LW_AF_Handle h = (LW_AF_Handle) handle;
    const IUNIVERSAL_Params *params = (IUNIVERSAL_Params *)algParams;
    if (params == NULL) 
    {
        params = &LW_AF_PARAMS;
    }

    h->bufTwiddle = (XDAS_Int16*)memTab[1].base;
	//Log_print2(Diags_USER1, "[%x] %s", (IArg)h->bufTwiddle, (IArg)"bufTwiddle ptr");
    h->bufTmp1 = (XDAS_UInt8*)memTab[2].base;
	//Log_print2(Diags_USER1, "[%x] %s", (IArg)h->bufTmp1, (IArg)"bufTmp1 ptr");
    h->bufTmp2 = (XDAS_UInt8*)memTab[3].base;
	//Log_print2(Diags_USER1, "[%x] %s", (IArg)h->bufTmp2, (IArg)"bufTmp2 ptr");
    h->bufImgRef = (XDAS_UInt8*)memTab[4].base;
	//Log_print2(Diags_USER1, "[%x] %s", (IArg)h->bufImgRef, (IArg)"bufImgRef ptr");
    h->bufImgTmp1 = (XDAS_UInt8*)memTab[5].base;
	//Log_print2(Diags_USER1, "[%x] %s", (IArg)h->bufImgTmp1, (IArg)"bufImgTmp1 ptr");
    h->bufImgTmp2 = (XDAS_UInt8*)memTab[6].base;
	//Log_print2(Diags_USER1, "[%x] %s", (IArg)h->bufImgTmp2, (IArg)"bufImgTmp2 ptr");
    return (IALG_EOK);
}

// Pointers for input/output data - from buf descs
XDAS_Int32 LW_AF_process(IUNIVERSAL_Handle handle,
        XDM1_BufDesc *inBufs, XDM1_BufDesc *outBufs, XDM1_BufDesc *inOutBufs,
        IUNIVERSAL_InArgs *inArgs, IUNIVERSAL_OutArgs *outArgs)
{
	Log_print2(Diags_USER1, "[%d] %s", 3, (IArg)"process");
    LW_AF_Handle h = (LW_AF_Handle) handle;
	LW_AF_InArgs* inArgsLw = (LW_AF_InArgs*)inArgs;
	LW_AF_OutArgs* outArgsLw = (LW_AF_OutArgs*)outArgs;

    /* validate arguments - this codec only supports "base" XDM. */
    if ((inArgsLw->universal.size != sizeof(*inArgsLw)) ||
            (outArgsLw->universal.size != sizeof(*outArgsLw))) 
    {
        outArgs->extendedError = XDM_UNSUPPORTEDPARAM;
        return (IUNIVERSAL_EUNSUPPORTED);
    }
    /* validate that there's at least a single inBuf and outBuf */
    if ((inBufs->numBufs < 1) || (outBufs->numBufs < 1)) 
    {
        outArgs->extendedError = XDM_UNSUPPORTEDPARAM;
        return (IUNIVERSAL_EFAIL);
    }

    /* everything looks good, do the 'transcode', set outArgs and return */
	h->inParmU32 = inArgsLw->inParmU32;
	h->inParmU16 = inArgsLw->inParmU16;

    h->inBufSig = (XDAS_Int16*)inBufs->descs[0].buf;
    h->inBufRef = (XDAS_Int16*)inBufs->descs[1].buf;
    h->inBufImgSig = (XDAS_Int16*)inBufs->descs[2].buf;
    h->inBufImgRef = (XDAS_Int16*)inBufs->descs[3].buf;
    h->outBufFft = (XDAS_Int16*)outBufs->descs[0].buf;
    h->outBufImg = (XDAS_Int16*)outBufs->descs[1].buf;

    // XXX
    // здесь обработка данных
    memset(h->bufTmp1, 0, BUFSIZE_1D);
    memset(h->bufTmp2, 0, BUFSIZE_1D);
    memset(h->bufImgRef, 0, BUFSIZE_2D);
    memset(h->bufImgTmp1, 0, BUFSIZE_2D);
    memset(h->bufImgTmp2, 0, BUFSIZE_2D);

	mfilter_init(h->bufTwiddle,
                    h->inBufImgRef,
                    (int16_t*)h->bufTmp1,
                    (int16_t*)h->bufImgTmp1,
                    (int16_t*)h->bufImgRef);
	Log_print2(Diags_USER1, "[%d] %s", 4, (IArg)"Before convolve");
	mfilter_convolve(h->bufTwiddle,
                        (int16_t*)h->inBufImgSig,
                        (int16_t*)h->bufImgRef,
                        (int16_t*)h-> bufTmp1,
                        (int16_t*)h->bufImgTmp1,
                        (int16_t*)h->bufImgTmp2,
                        (int16_t*)h->outBufImg);
	/**************************/

    /* report how we accessed the input buffer */
    inBufs->descs[0].accessMask = 0;
    XDM_SETACCESSMODE_READ(inBufs->descs[0].accessMask);
    inBufs->descs[1].accessMask = 0;
    XDM_SETACCESSMODE_READ(inBufs->descs[1].accessMask);

    /* report how we accessed the output buffer */
    outBufs->descs[0].accessMask = 0;
    XDM_SETACCESSMODE_WRITE(outBufs->descs[0].accessMask);
	outArgsLw->outParmU16 = h->outParmU16;
    return (IUNIVERSAL_EOK);
}

XDAS_Int32 LW_AF_control(IUNIVERSAL_Handle handle,
    IUNIVERSAL_Cmd id, IUNIVERSAL_DynamicParams *dynParams,
    IUNIVERSAL_Status *status)
{
    XDAS_Int32 retVal;

    /* validate arguments - this codec only supports "base" XDM. */
    if ((dynParams->size != sizeof(*dynParams)) ||
            (status->size != sizeof(*status))) 
    {
        return (IUNIVERSAL_EUNSUPPORTED);
    }

    /* initialize for the general case where we don't access the data buffer */
    XDM_CLEARACCESSMODE_READ(status->data.descs[0].accessMask);
    XDM_CLEARACCESSMODE_WRITE(status->data.descs[0].accessMask);

    switch (id) 
    {
        case XDM_GETVERSION:
            if ((status->data.descs[0].buf != NULL) &&
                (status->data.descs[0].bufSize >=
                    strlen(VERSIONSTRING))) 
	    {

                strncpy((char *)status->data.descs[0].buf, VERSIONSTRING,
                        strlen(VERSIONSTRING));

                /* null terminate the string */
                status->data.descs[0].buf[strlen(VERSIONSTRING)] = '\0';

                /* strncpy wrote to the data buffer */
                XDM_SETACCESSMODE_WRITE(status->data.descs[0].accessMask);

                retVal = IUNIVERSAL_EOK;
            }
            else 
	    {
                retVal = IUNIVERSAL_EFAIL;
            }

            break;

        default:
            /* unsupported cmd */
            retVal = IUNIVERSAL_EFAIL;
            break;
    }

    return (retVal);
}

