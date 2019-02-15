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

#define DPRINTLN(x) Serial.println(x)
#define DPRINT(x) Serial.print(x)

// =============================================================================
// NODE GLOBAL DESCRIPTION
// =============================================================================

#define NODE_SERIAL_NO     { aa, 0f, 3a, 4c, 00 }   // Serial number generated during compilation process
#define NODE_VERSION       "0.0.1"                  // Hardware version (PCB revision)
