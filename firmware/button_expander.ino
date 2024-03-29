
/*

BUTTON EXPANDER MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#if BUTTON_EXPANDER_SUPPORT

#include "config/all.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>

Adafruit_MCP23017 mcp;

#define EXPANDER_ADDRESS        0x20
#define EXPANDER_INPUTS         16

bool _expander_status[EXPANDER_INPUTS];
bool _expander_ready[EXPANDER_INPUTS];
bool _expander_reset_count[EXPANDER_INPUTS];

uint32_t _expander_event_start[EXPANDER_INPUTS];
uint32_t _expander_event_length[EXPANDER_INPUTS];
uint8_t _expander_event_count[EXPANDER_INPUTS];

uint8_t _expander_communication_register_address[EXPANDER_INPUTS];

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

uint8_t _expanderButtonRead(
    uint8_t pin
) {
    uint8_t event = BUTTON_EVENT_NONE;

    //digitalWrite((13 + 0), _expanderReadPin(1));

    if (_expanderReadPin(pin) != _expander_status[pin]) {
        // Debounce
        uint32_t start = millis();

        while (millis() - start < BUTTON_DEBOUNCE_DELAY) {
            delay(1);
        }

        if (_expanderReadPin(pin) != _expander_status[pin]) {
            _expander_status[pin] = !_expander_status[pin];

            // Released
            if (_expander_status[pin] == false) {
                _expander_event_length[pin] = millis() - _expander_event_start[pin];
                _expander_ready[pin] = true;

            // Pressed
            } else {
                event = BUTTON_EVENT_PRESSED;

                _expander_event_start[pin] = millis();
                _expander_event_length[pin] = 0;

                if (_expander_reset_count[pin]) {
                    _expander_event_count[pin] = 1;
                    _expander_reset_count[pin] = false;

                } else {
                    ++_expander_event_count[pin];
                }

                _expander_ready[pin] = false;
            }
        }
    }

    if (_expander_ready[pin] && (millis() - _expander_event_start[pin] > BUTTON_DBLCLICK_DELAY)) {
        _expander_ready[pin] = false;
        _expander_reset_count[pin] = true;

        event = BUTTON_EVENT_RELEASED;
    }

    return event;
}

// -----------------------------------------------------------------------------

uint8_t _expanderButtonMapEvent(
    const uint8_t event,
    const uint8_t count,
    const uint16_t length
) {
    if (event == BUTTON_EVENT_PRESSED) {
        return BUTTON_EVENT_PRESSED;

    } else if (event == BUTTON_EVENT_RELEASED) {
        if (count == 1) {
            if (length >= BUTTON_LNGLNGCLICK_DELAY) {
                return BUTTON_EVENT_LNGLNGCLICK;

            } else if (length >= BUTTON_LNGCLICK_DELAY) {
                return BUTTON_EVENT_LNGCLICK;
            }

            return BUTTON_EVENT_CLICK;

        } else if (count == 2) {
            return BUTTON_EVENT_DBLCLICK;

        } else if (count == 3) {
            return BUTTON_EVENT_TRIPLECLICK;

        } else {
            return BUTTON_EVENT_RELEASED;
        }
    }

    return BUTTON_EVENT_NONE;
}

// -----------------------------------------------------------------------------

void _expanderButtonEvent(
    const uint8_t id,
    uint8_t event
) {
    uint8_t mapped_event = _expanderButtonMapEvent(event, _expander_event_count[id], _expander_event_length[id]);

    if (id >= EXPANDER_INPUTS || mapped_event == 0) {
        return;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[EXPANDER] Button #"));
        DPRINT(id);
        DPRINT(F(" event "));
        DPRINTLN(mapped_event);
    #endif

    uint8_t communication_mapped_event = event;

    // Store state into communication register
    registerWriteRegister(REGISTER_TYPE_INPUT, _expander_communication_register_address[id], communication_mapped_event);
}

// -----------------------------------------------------------------------------

bool _expanderReadPin(
    const uint8_t pin
) {
    return !mcp.digitalRead(pin);
}

// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------

void expanderSetup()
{
    mcp.begin();

    for (uint8_t i = 0; i < EXPANDER_INPUTS; i++) {
        mcp.pinMode(i, INPUT);
        mcp.pullUp(i, HIGH);  // Turn on a 100K pullup internally

        _expander_status[i] = false;
        _expander_event_count[i] = 0;
        _expander_ready[i] = false;
        _expander_reset_count[i] = true;

        // Reserve address in register
        _expander_communication_register_address[i] = 0;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[EXPANDER] Number of buttons: "));
        DPRINTLN(EXPANDER_INPUTS);
    #endif
}

// -----------------------------------------------------------------------------

void expanderLoop()
{
    for (uint8_t i = 0; i < EXPANDER_INPUTS; i++) {
        if (uint8_t event = _expanderButtonRead(i)) {
            _expanderButtonEvent(i, event);
        }
    }
}
#endif // BUTTON_EXPANDER_SUPPORT