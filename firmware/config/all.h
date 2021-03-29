/*

CONFIGURATION IMPORT CONTAINER

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#include "version.h"
#include "types.h"
#include "prototypes.h"
#include "hardware.h"
#include "general.h"
#include "progmem.h"
#include "dependencies.h"

#if DEBUG_SUPPORT
    #define DPRINTLN(x) Serial.println(x)
    #define DPRINT(x) Serial.print(x)
#endif

// =============================================================================
// NODE GLOBAL DESCRIPTION
// =============================================================================

#define DEVICE_SERIAL_NO     "0124004A"   // Serial number generated during compilation process
#define DEVICE_VERSION       "0.0.1"      // Hardware version (PCB revision)
