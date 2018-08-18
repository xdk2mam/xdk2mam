/*
* Licensee agrees that the example code provided to Licensee has been developed and released by Bosch solely as an example to be used as a potential reference for Licensee�s application development. 
* Fitness and suitability of the example code for any use within Licensee�s applications need to be verified by Licensee on its own authority by taking appropriate state of the art actions and measures (e.g. by means of quality assurance measures).
* Licensee shall be responsible for conducting the development of its applications as well as integration of parts of the example code into such applications, taking into account the state of the art of technology and any statutory regulations and provisions applicable for such applications. Compliance with the functional system requirements and testing there of (including validation of information/data security aspects and functional safety) and release shall be solely incumbent upon Licensee. 
* For the avoidance of doubt, Licensee shall be responsible and fully liable for the applications and any distribution of such applications into the market.
* 
* 
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions are 
* met:
* 
*     (1) Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer. 
* 
*     (2) Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the
*     distribution.  
*     
*     (3)The name of the author may not be used to
*     endorse or promote products derived from this software without
*     specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR 
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
*  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
*  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
*  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
*  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
*  POSSIBILITY OF SUCH DAMAGE.
*/
/*----------------------------------------------------------------------------*/

#include "XDKAppInfo.h"
#undef BCDS_MODULE_ID
#define BCDS_MODULE_ID  XDK_APP_MODULE_ID_MAIN
/* system header files */
#include <stdio.h>
#include "BCDS_Basics.h"

/* additional interface header files */
#include "XdkSystemStartup.h"
#include "BCDS_Assert.h"
#include "BCDS_CmdProcessor.h"
#include "FreeRTOS.h"
#include "task.h"

#include "xdk2mam-mqtt.h"

/* own header files */

/* global variables ********************************************************* */
static CmdProcessor_T MainCmdProcessor;

/* functions */

/**
 * @brief        System Error Handling routine
 *
 * @param [in]   Error: Error Information from Basic
 *
 * @param [in]   isfromIsr :currently unused, can be used for future implementation.
 */
void ErrorHandlingFunc(Retcode_T error, bool isfromIsr)
{
    if(false == isfromIsr)
    {
        Retcode_Severity_T SeverityCode = Retcode_GetSeverity(error);
        if(SeverityCode < RETCODE_SEVERITY_WARNING)
        {
            printf("System Error:, Severity:%u Error code: %u Package Id:%u and module ID is :%u \r\n",
                    (unsigned int) SeverityCode,
                    (unsigned int) Retcode_GetCode(error),
                    (unsigned int) Retcode_GetPackage(error),
                    (unsigned int) Retcode_GetModuleId(error));
            printf(" Reset the system\n\r");
            for(;;);
        }

    }
    else
    {
        BCDS_UNUSED(error);
    }
}

int main(void)
{
    /* Mapping Default Error Handling function */
    Retcode_T returnValue = Retcode_Initialize(ErrorHandlingFunc);
    if (RETCODE_OK == returnValue)
    {
        returnValue = systemStartup();
    }
    if (RETCODE_OK == returnValue)
    {
        returnValue = CmdProcessor_Initialize(&MainCmdProcessor, (char *) "MainCmdProcessor", TASK_PRIO_MAIN_CMD_PROCESSOR, TASK_STACK_SIZE_MAIN_CMD_PROCESSOR, TASK_Q_LEN_MAIN_CMD_PROCESSOR);
    }
    if (RETCODE_OK == returnValue)
    {
        /* Here we enqueue the application initialization into the command
         * processor, such that the initialization function will be invoked
         * once the RTOS scheduler is started below.
         */
        returnValue = CmdProcessor_Enqueue(&MainCmdProcessor, AppInitSystem, &MainCmdProcessor, UINT32_C(0));
    }
    if (RETCODE_OK != returnValue)
    {
        printf("System Startup failed");
        assert(false);
    }
    /* start scheduler */
    vTaskStartScheduler();
}
