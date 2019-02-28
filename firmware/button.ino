/*

BUTTON MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if BUTTON_SUPPORT

#include <DebounceEvent.h>
#include <Vector.h>

typedef struct {
    DebounceEvent * button;
    byte            register_address;      // Address in communication register to store state
} button_t;

Vector<button_t> _buttons;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

byte _buttonMapEvent(
    byte event,
    byte count,
    uint16_t length
) {
    if (event == EVENT_PRESSED) {
        return BUTTON_EVENT_PRESSED;

    } else if (event == EVENT_CHANGED) {
        return BUTTON_EVENT_CLICK;

    } else if (event == EVENT_RELEASED) {
        if (count == 1) {
            if (length > BUTTON_LNGLNGCLICK_DELAY) {
                return BUTTON_EVENT_LNGLNGCLICK;

            } else if (length > BUTTON_LNGCLICK_DELAY) {
                return BUTTON_EVENT_LNGCLICK;
            }

            return BUTTON_EVENT_CLICK;

        } else if (count == 2) {
            return BUTTON_EVENT_DBLCLICK;

        } else if (count == 3) {
            return BUTTON_EVENT_TRIPLECLICK;
        }
    }

    return BUTTON_EVENT_NONE;
}

// -----------------------------------------------------------------------------

void _buttonEvent(
    byte id,
    byte event
) {
    if (id >= buttonCount() || event == 0) {
        return;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[BUTTON] Button #"));
        DPRINT(id);
        DPRINT(F(" event "));
        DPRINTLN(event);
    #endif

    // Store state into communication register
    communicationWriteAnalogInput(_buttons[id].register_address, (word) event);
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

int buttonCount() {
    return _buttons.size();
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void buttonSetup() {
    unsigned long _btn_delay = BUTTON_DBLCLICK_DELAY;
    uint8_t register_address;

    #if BUTTON1_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON1_PIN, BUTTON1_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON2_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON2_PIN, BUTTON2_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON3_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON3_PIN, BUTTON3_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON4_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON4_PIN, BUTTON4_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON5_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON5_PIN, BUTTON5_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON6_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON6_PIN, BUTTON6_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON7_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON7_PIN, BUTTON7_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif
    #if BUTTON8_PIN != GPIO_NONE
    {
        register_address = communicationRegisterAnalogInput(COMMUNICATION_DATA_TYPE_UINT8);

        _buttons.push_back((button_t) {new DebounceEvent(BUTTON8_PIN, BUTTON8_MODE, BUTTON_DEBOUNCE_DELAY, _btn_delay), register_address});
    }
    #endif

    #if DEBUG_SUPPORT
        DPRINT(F("[BUTTON] Number of buttons: "));
        DPRINTLN(buttonCount());
    #endif
}

// -----------------------------------------------------------------------------

void buttonLoop() {
    for (byte i = 0; i < buttonCount(); i++) {
        if (byte event = _buttons[i].button->loop()) {
            byte count = _buttons[i].button->getEventCount();
            unsigned long length = _buttons[i].button->getEventLength();

            byte mapped = _buttonMapEvent(event, count, length);

            _buttonEvent(i, mapped);
        }
    }
}

#endif // BUTTON_SUPPORT
