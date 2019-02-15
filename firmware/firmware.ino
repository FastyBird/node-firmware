/*

CORE MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include "config/all.h"

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <ArduinoJson.h>
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

    systemSetup();

    #if BUTTON_SUPPORT
        buttonSetup();
    #endif

    #if RELAY_SUPPORT && RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relaySetup();
    #endif

    ledSetup();

    nodeSetup();
}

// -----------------------------------------------------------------------------

void loop() {
    #if BUTTON_SUPPORT
        buttonLoop();
    #endif

    #if RELAY_SUPPORT && RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relayLoop();
    #endif

    ledLoop();

    nodeLoop();

    systemLoop();

    communicationLoop();
}
