/*

CORE MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#include "config/all.h"

#include <Arduino.h>

#if !defined(ARDUINO_ARCH_SAM) && !defined(ARDUINO_ARCH_SAMD) && !defined(ARDUINO_ARCH_STM32F2)
    #include <EEPROM.h>
#else
    #include <../lib/ArmEeprom/Samd21Eeprom.h>
#endif

uint8_t _firmware_device_state = DEVICE_STATE_STOPPED;


void _firmwareUpdateLedState()
{
    #if SYSTEM_DEVICE_STATE_LED != INDEX_NONE
        if (_firmware_device_state == DEVICE_STATE_RUNNING) {
            ledSetMode(SYSTEM_DEVICE_STATE_LED, LED_MODE_ON);

        } else if (
            _firmware_device_state == DEVICE_STATE_STOPPED
            || _firmware_device_state == DEVICE_STATE_STOPPED_BY_OPERATOR
        ) {
            ledSetMode(SYSTEM_DEVICE_STATE_LED, LED_MODE_OFF);
        }
    #endif
}

// -----------------------------------------------------------------------------

void _firmwareLoadState()
{
    _firmware_device_state = DEVICE_STATE_RUNNING;
    return;

    uint8_t stored_state = (uint8_t) EEPROM.read(FLASH_ADDRESS_DEVICE_STATE);

    if (stored_state == DEVICE_STATE_RUNNING) {
        _firmware_device_state = DEVICE_STATE_RUNNING;

    } else if (stored_state == DEVICE_STATE_STOPPED) {
        _firmware_device_state = DEVICE_STATE_STOPPED;

    } else if (stored_state == DEVICE_STATE_STOPPED_BY_OPERATOR) {
        _firmware_device_state = DEVICE_STATE_STOPPED_BY_OPERATOR;
    }

    #if SYSTEM_DEVICE_STATE_LED != INDEX_NONE
        _firmwareUpdateLedState();
    #endif
}

// -----------------------------------------------------------------------------
// FIRMWARE BASIC API
// -----------------------------------------------------------------------------

void firmwareSetDeviceState(
    uint8_t setStatus
) {
    if (setStatus == DEVICE_STATE_RUNNING) {
        _firmware_device_state = DEVICE_STATE_RUNNING;

    } else if (setStatus == DEVICE_STATE_STOPPED) {
        _firmware_device_state = DEVICE_STATE_STOPPED;

    } else if (setStatus == DEVICE_STATE_ERROR) {
        _firmware_device_state = DEVICE_STATE_ERROR;

    } else if (setStatus == DEVICE_STATE_STOPPED_BY_OPERATOR) {
        _firmware_device_state = DEVICE_STATE_STOPPED_BY_OPERATOR;

    } else {
        return;
    }

    #if SYSTEM_DEVICE_STATE_LED != INDEX_NONE
        _firmwareUpdateLedState();
    #endif

    EEPROM.update(FLASH_ADDRESS_DEVICE_STATE, _firmware_device_state);

    communicationReportDeviceState();
}

// -----------------------------------------------------------------------------

uint8_t firmwareGetDeviceState() {
    if (communicationIsInPairingMode()) {
        return DEVICE_STATE_PAIRING;
    }

    return _firmware_device_state;
}

// -----------------------------------------------------------------------------

bool firmwareIsRunning() {
    return _firmware_device_state == DEVICE_STATE_RUNNING;
}

// -----------------------------------------------------------------------------
// BOOTING
// -----------------------------------------------------------------------------

void setup()
{
    #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32F2)
        Serial1.begin(SERIAL_BAUDRATE);
    #else
        Serial.begin(SERIAL_BAUDRATE);
    #endif

    _firmwareLoadState();

    communicationSetup();

    buttonSetup();

    #if BUTTON_EXPANDER_SUPPORT
        expanderSetup();
    #endif

    #if RELAY_PROVIDER != RELAY_PROVIDER_NONE
        relaySetup();
    #endif

    ledSetup();

    #if SYSTEM_DEVICE_STATE_LED != INDEX_NONE
        _firmwareUpdateLedState();
    #endif
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
