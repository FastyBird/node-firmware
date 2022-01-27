/*

CORE MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#include "config/all.h"

#include <Arduino.h>

void(* resetFunc) (void) = 0;

bool _firmwareIsBooting = false;

// -----------------------------------------------------------------------------
// FIRMWARE BASIC API
// -----------------------------------------------------------------------------

void firmwareSetDeviceState(
    uint8_t setStatus
) {
    // Check if valid value is provided to store
    if (
        setStatus != DEVICE_STATE_RUNNING
        && setStatus != DEVICE_STATE_STOPPED
        && setStatus != DEVICE_STATE_STOPPED_BY_OPERATOR
        && setStatus != DEVICE_STATE_PAIRING
        && setStatus != DEVICE_STATE_ERROR
    ) {
        return;
    }

    registerWriteRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS, setStatus);
}

// -----------------------------------------------------------------------------

uint8_t firmwareGetDeviceState()
{
    uint8_t device_state;

    registerReadRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS, device_state);

    // Check if valid value is stored in registry
    if (
        device_state != DEVICE_STATE_RUNNING
        && device_state != DEVICE_STATE_STOPPED
        && device_state != DEVICE_STATE_STOPPED_BY_OPERATOR
        && device_state != DEVICE_STATE_PAIRING
        && device_state != DEVICE_STATE_ERROR
    ) {
        return DEVICE_STATE_ERROR;
    }

    return device_state;
}

// -----------------------------------------------------------------------------

bool firmwareIsRunning()
{
    uint8_t register_value;

    registerReadRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS, register_value);
    
    return register_value == DEVICE_STATE_RUNNING;
}

// -----------------------------------------------------------------------------

bool firmwareIsPairing()
{
    uint8_t register_value;

    registerReadRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS, register_value);
    
    return register_value == DEVICE_STATE_PAIRING;
}

// -----------------------------------------------------------------------------

bool firmwareIsBooting()
{
    return _firmwareIsBooting;
}

// -----------------------------------------------------------------------------
// BOOTING
// -----------------------------------------------------------------------------

void setup()
{
    _firmwareIsBooting = true;

    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32F2)
        Serial1.begin(COMMUNICATION_SERIAL_BAUDRATE);
    #else
        Serial.begin(COMMUNICATION_SERIAL_BAUDRATE);
    #endif

    registerSetup();

    communicationSetup();

    buttonSetup();

    #if BUTTON_EXPANDER_SUPPORT
        expanderSetup();
    #endif

    #if RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relaySetup();
    #endif

    ledSetup();

    #if DEBUG_SUPPORT
        DPRINT(F("[FIRMWARE] Device is in "));
        DPRINT(firmwareGetDeviceState());
        DPRINTLN(F(" state"));
    #endif

    _firmwareIsBooting = false;
}

// -----------------------------------------------------------------------------

void loop()
{
    buttonLoop();

    #if BUTTON_EXPANDER_SUPPORT
        expanderLoop();
    #endif

    #if RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relayLoop();
    #endif

    ledLoop();

    communicationLoop();
}
