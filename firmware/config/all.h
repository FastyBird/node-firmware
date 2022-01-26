/*

CONFIGURATION IMPORT CONTAINER

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#pragma once

#ifndef FW_CONFIG
    #define FW_CONFIG

    #include "firmware.h"
    #include "types.h"
    #include "prototypes.h"
    #include "hardware.h"
    #include "general.h"
    #include "dependencies.h"

    #if DEBUG_SUPPORT
        #define DPRINTLN(x) Serial.println(x)
        #define DPRINT(x) Serial.print(x)
    #endif

    // =============================================================================
    // DEVICE GLOBAL DESCRIPTION
    // =============================================================================

    #define DEVICE_SERIAL_NO     "0124004A"   // Serial number generated during compilation process
    #define DEVICE_VERSION       "0.0.1"      // Hardware revision

#endif
