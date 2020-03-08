/*
 Copyright (c) by Nazim Aliev
 All rights reserved.

 nazim.ru@gmail.com

*/

/*
 *  ======== main.c ========
 */

#include <xdc/std.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Diags.h>

#include <ti/ipc/Ipc.h>

#include <ti/sdo/ce/CERuntime.h>
#include <ti/sdo/ce/Server.h>
#include <ti/sdo/ce/Engine.h>
#include <ti/sysbios/BIOS.h>

#include <ti/sdo/fc/global/FCSettings.h>
#include <ti/sdo/ce/global/CESettings.h>

/* Include these headers for VISA type defines */
#include <ti/sdo/ce/universal/universal.h>

/* Include codec header files */
#include "../codec_dsp/codec_dsp.h"

/*
 *  ======== main ========
 */
Void main(Int argc, Char *argv[])
{
    Server_AlgDesc  algDesc;
    Server_Status   ceStatus;
    Int             status;

    do {
        /* init IPC */
        status = Ipc_start();
    } while (status < 0);

    /* Enable this code to get trace for CERuntime_init() code. */
#if 0
    /*
     *  To got trace from module initialization functions, set FC and
     *  CE settings Diags masks before CERuntime_init() is called.
     */
    CESettings_init();
    FCSettings_init();

    Diags_setMask(FCSETTINGS_MODNAME"+EX1234567");
    Diags_setMask(CESETTINGS_MODNAME"+EX1234567");
#endif

    /* init Codec Engine */
    CERuntime_init();

    /*
     *  Enable this code to get Server and Engine trace.  This may be
     *  helpful for debugging, if Server_addAlg() fails.
     */
#if 0
    Diags_setMask(Server_MODNAME"+EX1234567");
    Diags_setMask(Engine_MODNAME"+EX1234567");
#endif

    /* Add AF Processing alg to the server */

    /* Set fields to defaults */
    Server_initAlgDesc(&algDesc);

    algDesc.name = "af_proc";
    algDesc.fxns = (IALG_Fxns *)&LW_AF_IALG;
    algDesc.idmaFxns = NULL;
    algDesc.iresFxns = NULL;
    algDesc.isLocal = TRUE;
    algDesc.groupId = 1;

    algDesc.types = UNIVERSAL_VISATYPE;
    algDesc.stubFxnsName = UNIVERSAL_STUBSNAME;
    algDesc.skelFxns = &UNIVERSAL_SKEL;

    algDesc.priority = 1;
    algDesc.stackSize = 0x1000;

    ceStatus = Server_addAlg(NULL, NULL, &algDesc);

    if (ceStatus != Server_EOK) {
        Log_print1(Diags_USER7, "App-> ERROR: can't add Alg (0x%x)\n",
                (IArg)ceStatus);
        /*
         *  We could halt here, but then we will not be able to get
         *  the trace log up to the host.
         */
        /* while (1) {} */
    }

    BIOS_start();
}
