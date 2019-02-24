/*

LED MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include <Vector.h>

typedef struct {
    byte pin;
    bool reverse;
    byte mode;
} led_t;

Vector<led_t> _leds;

bool _led_update = false;            // For relay-based modes

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

bool _ledStatus(
    byte id
) {
    if (id >= ledCount()) {
        return false;
    }

    bool status = digitalRead(_leds[id].pin);

    return _leds[id].reverse ? !status : status;
}

// -----------------------------------------------------------------------------

bool _ledStatus(
    byte id,
    bool status
) {
    if (id >=ledCount()) {
        return false;
    }

    digitalWrite(_leds[id].pin, _leds[id].reverse ? !status : status);

    return status;
}

// -----------------------------------------------------------------------------

bool _ledToggle(
    byte id
) {
    if (id >= ledCount()) {
        return false;
    }

    return _ledStatus(id, !_ledStatus(id));
}

// -----------------------------------------------------------------------------

byte _ledMode(
    byte id
) {
    if (id >= ledCount()) {
        return false;
    }

    return _leds[id].mode;
}

// -----------------------------------------------------------------------------

void _ledMode(
    byte id,
    byte mode
) {
    if (id >= ledCount()) {
        return;
    }

    _leds[id].mode = mode;
}

// -----------------------------------------------------------------------------

void _ledBlink(
    byte id,
    unsigned long delayOff,
    unsigned long delayOn
) {
    if (id >= ledCount()) {
        return;
    }

    static unsigned long next = millis();

    if (next < millis()) {
        next += (_ledToggle(id) ? delayOn : delayOff);
    }
}

// -----------------------------------------------------------------------------

void _ledConfigure() {
    for (byte i = 0; i < ledCount(); i++) {
        _ledMode(i, _ledMode(i));
    }

    _led_update = true;
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

byte ledCount() {
    return _leds.size();
}

// -----------------------------------------------------------------------------

void ledUpdate(bool value) {
    _led_update = value;
}

// -----------------------------------------------------------------------------

void ledSetup() {
    #if LED1_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED1_PIN, LED1_PIN_INVERSE, LED1_MODE });
    #endif

    #if LED2_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED2_PIN, LED2_PIN_INVERSE, LED2_MODE });
    #endif

    #if LED3_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED3_PIN, LED3_PIN_INVERSE, LED3_MODE });
    #endif

    #if LED4_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED4_PIN, LED4_PIN_INVERSE, LED4_MODE });
    #endif

    #if LED5_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED5_PIN, LED5_PIN_INVERSE, LED5_MODE });
    #endif

    #if LED6_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED6_PIN, LED6_PIN_INVERSE, LED6_MODE });
    #endif

    #if LED7_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED7_PIN, LED7_PIN_INVERSE, LED7_MODE });
    #endif

    #if LED8_PIN != GPIO_NONE
        _leds.push_back((led_t) { LED8_PIN, LED8_PIN_INVERSE, LED8_MODE });
    #endif

    for (byte i = 0; i < ledCount(); i++) {
        pinMode(_leds[i].pin, OUTPUT);

        _ledStatus(i, false);
    }

    _ledConfigure();

    #if DEBUG_SUPPORT
        DPRINT(F("[LED] Number of leds: "));
        DPRINTLN(ledCount());
    #endif
}

// -----------------------------------------------------------------------------

void ledLoop() {
    for (byte i = 0; i < ledCount(); i++) {
        if (_ledMode(i) == LED_MODE_BUS) {
            if (communicationConnected()) {
                _ledBlink(i, 4900, 100);

            } else {
                _ledBlink(i, 500, 500);
            }

        } else if (_ledMode(i) == LED_MODE_ON) {
            _ledStatus(i, true);

        } else if (_ledMode(i) == LED_MODE_OFF) {
            _ledStatus(i, false);
        }
    }

    _led_update = false;
}
