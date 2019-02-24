/*

CORE MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include "config/all.h"

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <PJON.h>
#include <Vector.h>

#if BUTTON_SUPPORT
    #include <DebounceEvent.h>
#endif

// -----------------------------------------------------------------------------
// BOOTING
// -----------------------------------------------------------------------------

void setup() {
    communicationSetup();

    #if BUTTON_SUPPORT
        buttonSetup();
    #endif

    #if RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relaySetup();
    #endif

    ledSetup();
}

// -----------------------------------------------------------------------------

void loop() {
    #if BUTTON_SUPPORT
        buttonLoop();
    #endif

    #if RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relayLoop();
    #endif

    ledLoop();

    communicationLoop();
}
