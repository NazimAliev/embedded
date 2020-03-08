/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 * 
 *  This file contains an implementation of the IUNIVERSAL interface
 *  defined by XDM.
 */

#include "../../af.h"
#include "afmath.h"
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

/* buffer definitions */
#define MININBUFS       1
#define MINOUTBUFS      1
#define MININBUFSIZE    1
#define MINOUTBUFSIZE   1

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

#define NUMBUFS 5

/*
 *  ======== LW_AF_IUNIVERSALCOPY ========
 *  This structure defines TI's implementation of the IUNIVERSAL interface
 *  for the LW_AF module.
 */
IUNIVERSAL_Fxns LW_AF_I = 
{
    {IALGFXNS},
    LW_AF_process,
    LW_AF_control,
};

/*
 *  ======== LW_AF_IALG ========
 *  This structure defines TI's implementation of the IALG interface
 *  for the LW_AF module.
 */
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
    return(NUMBUFS);
}
/*
 *  ======== LW_AF_alloc ========
 *  Return a table of memory descriptors that describe the memory needed
 *  to construct our object.
 */
/* ARGSUSED - this line tells the TI compiler not to warn about unused args. */
Int LW_AF_alloc(const IALG_Params *algParams,
    IALG_Fxns **pf, IALG_MemRec memTab[])
{
    /* Request memory for my object */
    memTab[0].size = sizeof(LW_AF_Obj);
    memTab[0].alignment = 0;
    memTab[0].space = IALG_EXTERNAL;
    memTab[0].attrs = IALG_PERSIST;

    /* Request memory for twiddle buf */
    memTab[1].size = BUFSIZE;
    memTab[1].alignment = 8;
    memTab[1].space = IALG_EXTERNAL;
    memTab[1].attrs = IALG_PERSIST;

    /* Максимальное использование временных буферов - NSAMPLES комплексных int */
    /* Request memory for tmp buf */
    memTab[2].size = 2*BUFSIZE;
    memTab[2].alignment = 8;
    memTab[2].space = IALG_EXTERNAL;
    memTab[2].attrs = IALG_PERSIST;

    /* Request memory for tmp2 buf */
    memTab[3].size = 2*BUFSIZE;
    memTab[3].alignment = 8;
    memTab[3].space = IALG_EXTERNAL;
    memTab[3].attrs = IALG_PERSIST;

    /* Request memory for matrix buf */
    // дополнительно с левого и правого края матрицы будет использовано WINSIZE столбцов
    memTab[4].size = BUFSIZE * (ROWS + WINSIZE) * INTSIZE;
    memTab[4].alignment = 8;
    memTab[4].space = IALG_EXTERNAL;
    memTab[4].attrs = IALG_PERSIST;

    return(NUMBUFS);
}


/*
 *  ======== LW_AF_free ========
 *  Return a table of memory pointers that should be freed.  Note
 *  that this should include *all* memory requested in the
 *  alloc operation above.
 */
/* ARGSUSED - this line tells the TI compiler not to warn about unused args. */
Int LW_AF_free(IALG_Handle handle, IALG_MemRec memTab[])
{
    /*
     * Because our internal object size doesn't depend on create
     * params, we can just leverage the algAlloc() call with default
     * (NULL) create params.
     */
    return (LW_AF_alloc(NULL, NULL, memTab));
}


/*
 *  ======== LW_AF_initObj ========
 *  Initialize the memory allocated on our behalf (including our object).
 */
/* ARGSUSED - this line tells the TI compiler not to warn about unused args. */
Int LW_AF_initObj(IALG_Handle handle, const IALG_MemRec memTab[],
    IALG_Handle p, const IALG_Params *algParams)
{
    LW_AF_Handle h = (LW_AF_Handle) handle;
    const IUNIVERSAL_Params *params = (IUNIVERSAL_Params *)algParams;

    /*
     * In lieu of XDM defined default params, use our codec-specific ones.
     * Note that these default values _should_ be documented in your algorithm
     * documentation so users know what to expect.
     */
    if (params == NULL) 
    {
        params = &LW_AF_PARAMS;
    }

    h->twiddleBuf = (XDAS_Int16*)memTab[1].base;
    h->tmpBuf = (XDAS_UInt8*)memTab[2].base;
    h->tmp2Buf = (XDAS_UInt8*)memTab[3].base;
    h->matrixBuf = (XDAS_UInt32*)memTab[4].base;
    afmath_init(h->twiddleBuf);
    return (IALG_EOK);
}


/*
 *  ======== LW_AF_process ========
 */
/* ARGSUSED - this line tells the TI compiler not to warn about unused args. */
XDAS_Int32 LW_AF_process(IUNIVERSAL_Handle handle,
        XDM1_BufDesc *inBufs, XDM1_BufDesc *outBufs, XDM1_BufDesc *inOutBufs,
        IUNIVERSAL_InArgs *inArgs, IUNIVERSAL_OutArgs *outArgs)
{
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
	h->cfarLevel = inArgsLw->cfarLevel;
	h->logMode = inArgsLw->logMode;

    // сигнальный буфер размером BUFSIZE+ROWS*2 байт и NSAMPLES комплексных слов,
    h->sigBuf = (short*)inBufs->descs[0].buf;
    // опорный буфер размером BUFSIZE байт и NSAMPLES комплексных слов,
    h->refBuf = (short*)(inBufs->descs[1].buf);
    h->cfarBuf = (UChar*)outBufs->descs[0].buf;
    h->afBuf = (UShort*)outBufs->descs[1].buf;

    // XXX
    // здесь обработка данных
    memset(h->tmpBuf, 0, 2*BUFSIZE);
    memset(h->tmp2Buf, 0, 2*BUFSIZE);
    memset(h->matrixBuf, 0, BUFSIZE*(ROWS+WINSIZE)*INTSIZE);
    memset(h->cfarBuf, 0, ABUFSIZE/BYTESIZE);
    memset(h->afBuf, 0, ABUFSIZE*2);

    // не вызывать в рабочем режиме, портит sigBuf !
    //afmath_fft(h->sigBuf, h->tmpBuf, h->tmp2Buf, h->twiddleBuf);

    afmath_af(h->sigBuf, h->refBuf, h->tmpBuf, h->tmp2Buf, h->matrixBuf, h->afBuf, h->twiddleBuf, h->logMode);
    //afmath_cfar(h->matrixBuf, h->cfarBuf);
    h->alarms = afmath_cfar2(h->matrixBuf, h->cfarBuf, h->cfarLevel);

    /* report how we accessed the input buffer */
    inBufs->descs[0].accessMask = 0;
    XDM_SETACCESSMODE_READ(inBufs->descs[0].accessMask);
    inBufs->descs[1].accessMask = 0;
    XDM_SETACCESSMODE_READ(inBufs->descs[1].accessMask);

    /* report how we accessed the output buffer */
    outBufs->descs[0].accessMask = 0;
    XDM_SETACCESSMODE_WRITE(outBufs->descs[0].accessMask);
    outBufs->descs[1].accessMask = 0;
    XDM_SETACCESSMODE_WRITE(outBufs->descs[1].accessMask);
	outArgsLw->alarms = h->alarms;
    return (IUNIVERSAL_EOK);
}


/*
 *  ======== LW_AF_control ========
 */
/* ARGSUSED - this line tells the TI compiler not to warn about unused args. */
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
