/*

LED MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#include "config/all.h"

#include <Arduino.h>

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

bool _ledStatus(
    const uint8_t id
) {
    if (id >= LED_MAX_ITEMS) {
        return false;
    }

    bool status = digitalRead(led_module_items[id].pin);

    return led_module_items[id].reverse ? !status : status;
}

// -----------------------------------------------------------------------------

bool _ledStatus(
    const uint8_t id,
    const bool status
) {
    if (id >=LED_MAX_ITEMS) {
        return false;
    }

    digitalWrite(led_module_items[id].pin, led_module_items[id].reverse ? !status : status);

    return status;
}

// -----------------------------------------------------------------------------

bool _ledToggle(
    const uint8_t id
) {
    if (id >= LED_MAX_ITEMS) {
        return false;
    }

    return _ledStatus(id, !_ledStatus(id));
}

// -----------------------------------------------------------------------------

uint8_t _ledMode(
    const uint8_t id
) {
    if (id >= LED_MAX_ITEMS) {
        return false;
    }

    return led_module_items[id].mode;
}

// -----------------------------------------------------------------------------

void _ledMode(
    const uint8_t id,
    const uint8_t mode
) {
    if (id >= LED_MAX_ITEMS) {
        return;
    }

    led_module_items[id].mode = mode;
}

// -----------------------------------------------------------------------------

void _ledBlink(
    const uint8_t id,
    unsigned long delayOff,
    unsigned long delayOn
) {
    if (id >= LED_MAX_ITEMS) {
        return;
    }

    if (led_module_items[id].next < millis()) {
        led_module_items[id].next += (_ledToggle(id) ? delayOn : delayOff);
    }
}

// -----------------------------------------------------------------------------

void _ledConfigure() {
    for (uint8_t i = 0; i < LED_MAX_ITEMS; i++) {
        _ledMode(i, _ledMode(i));
    }
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

void ledSetMode(
    const uint8_t id,
    const uint8_t mode
) {
    _ledMode(id, mode);
}

// -----------------------------------------------------------------------------

void ledSetup()
{
    for (uint8_t i = 0; i < LED_MAX_ITEMS; i++) {
        pinMode(led_module_items[i].pin, OUTPUT);

        _ledStatus(i, false);
    }

    _ledConfigure();

    #if DEBUG_SUPPORT
        DPRINT(F("[LED] Number of leds: "));
        DPRINTLN(LED_MAX_ITEMS);
    #endif
}

// -----------------------------------------------------------------------------

void ledLoop()
{
    for (uint8_t i = 0; i < LED_MAX_ITEMS; i++) {
        #if SYSTEM_DEVICE_STATE_LED_INDEX != INDEX_NONE
            if (i == SYSTEM_DEVICE_STATE_LED_INDEX) {
                if (firmwareIsRunning()) {
                    _ledStatus(i, true);

                } else if (firmwareIsError()) {
                    _ledBlink(i, 250, 250);

                } else {
                    _ledStatus(i, false);
                }

                continue;
            }
        #endif

        #if SYSTEM_DEVICE_COMMUNICATION_LED_INDEX != INDEX_NONE
            if (i == SYSTEM_DEVICE_COMMUNICATION_LED_INDEX) {
                if (firmwareIsRunning()) {
                    if (firmwareIsDiscoverable()) {
                        _ledBlink(i, 250, 500);

                    } else if (communicationHasAssignedAddress() == false) {
                        _ledBlink(i, 500, 500);

                    } else if (communicationIsMasterLost()) {
                        _ledBlink(i, 1500, 500);

                    } else {
                        _ledBlink(i, 4900, 100);
                    }

                } else {
                    _ledStatus(i, false);
                }

                continue;
            }
        #endif

        if (_ledMode(i) == LED_MODE_ON) {
            _ledStatus(i, true);

        } else if (_ledMode(i) == LED_MODE_OFF) {
            _ledStatus(i, false);
        }
    }
}
