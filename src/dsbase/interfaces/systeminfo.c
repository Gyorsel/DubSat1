/*
 * systeminfo.c
 *
 *  Created on: Jun 29, 2017
 *      Author: jeffc
 */
#include <stdint.h>
#include "systeminfo.h"
#include "core/utils.h"
#include "core/debugtools.h"
#include "bsp/bsp.h"

FILE_STATIC uint8_t *SubsystemModulePaths[] =
                                           { "TestModule",
                                             "ADCS/BDot" };


uint8_t *getSubsystemModulePath()
{
    return SubsystemModulePaths[(uint8_t)bspGetModule()];
}

uint8_t infoReport(DebugMode mode)
{
    if (mode == InteractiveMode)
    {
        debugPrintF("**Subsystem Module: \t%s\r\n", getSubsystemModulePath());
        debugPrintF("Compiler Version:\t%d\r\n", __TI_COMPILER_VERSION__);
        debugPrintF("Standard C Version:\t%d\r\n", __STDC_VERSION__);
        debugPrintF("Date Compiled:\t\t%s\r\n", __DATE__);
        debugPrintF("Time Compiled:\t\t%s\r\n", __TIME__);

#if defined(_INLINE)
        debugPrintF("INLINE Enabled:\t\tTrue\r\n");
#else
        debugPrintF("INLINE Enabled:\t\tFalse\r\n");
#endif

    }
    else
    {
        debugPrintF("Stuff without as many words (e.g. just CSV)");
    }
    return 1;
}
