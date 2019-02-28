/*

CONFIGURATION IMPORT CONTAINER

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include "version.h"
#include "types.h"
#include "hardware.h"
#include "defaults.h"
#include "general.h"
#include "prototypes.h"
#include "progmem.h"

#if DEBUG_SUPPORT
    #define DPRINTLN(x) Serial.println(x)
    #define DPRINT(x) Serial.print(x)
#else
    #define DPRINTLN(x) void
    #define DPRINT(x) void
#endif

// =============================================================================
// NODE GLOBAL DESCRIPTION
// =============================================================================

#define NODE_SERIAL_NO     "0124004A"   // Serial number generated during compilation process
#define NODE_VERSION       "0.0.1"      // Hardware version (PCB revision)
