/*

BUTTON MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

#include "config/all.h"

#include <Arduino.h>

#if !defined(ARDUINO_ARCH_SAM) && !defined(ARDUINO_ARCH_SAMD) && !defined(ARDUINO_ARCH_STM32F2)
    #include <EEPROM.h>
#else
    #include <../lib/ArmEeprom/Samd21Eeprom.h>
#endif

uint8_t _buttonMapEvent(
    const uint8_t event,
    const uint8_t count,
    const uint16_t length
) {
    if (event == EVENT_PRESSED) {
        return BUTTON_EVENT_PRESSED;

    } else if (event == EVENT_CHANGED) {
        return BUTTON_EVENT_CLICK;

    } else if (event == EVENT_RELEASED) {
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

void _buttonEvent(
    const uint8_t id,
    const uint8_t event
) {
    if (id >= BUTTON_MAX_ITEMS) {
        return;
    }

    if (button_module_items[id].current_status == event) {
        return;
    }

    if (button_module_items[id].current_status == BUTTON_EVENT_PRESSED && event == BUTTON_EVENT_NONE) {
        return;
    }

    button_module_items[id].current_status = event;

    #if DEBUG_SUPPORT
        DPRINT(F("[BUTTON] Button #"));
        DPRINT(id);
        DPRINT(F(" event "));
        DPRINTLN(event);
    #endif

    #if REGISTER_MAX_INPUT_REGISTERS_SIZE
        if (button_module_items[id].register_address != INDEX_NONE) {
            registerWriteRegister(REGISTER_TYPE_INPUT, button_module_items[id].register_address, event);
        }
    #endif

    #if SYSTEM_CONFIGURE_DEVICE_BUTTON_INDEX != INDEX_NONE
        if (id == SYSTEM_CONFIGURE_DEVICE_BUTTON_INDEX) {
            switch (event)
            {

                case BUTTON_EVENT_CLICK:
                    if (firmwareIsDiscoverable()) {
                        firmwareSetDiscoverable(false);

                    } else if (firmwareIsRunning()) {
                        firmwareSetDeviceState(DEVICE_STATE_STOPPED_BY_OPERATOR);

                    } else {
                        firmwareSetDeviceState(DEVICE_STATE_RUNNING);
                    }
                    break;

                case BUTTON_EVENT_LNGCLICK:
                    #if DEBUG_SUPPORT
                        DPRINTLN(F("[BUTTON] Activating pairing mode"));
                    #endif

                    firmwareSetDiscoverable(true);
                    break;

                case BUTTON_EVENT_LNGLNGCLICK:
                    #if DEBUG_SUPPORT
                        DPRINTLN(F("[BUTTON] Clearing EEPROM"));
                    #endif

                    // Clear stored values to factory settings
                    for (uint8_t i = 0 ; i < EEPROM.length() ; i++) {
                        EEPROM.write(i, 0);
                    }
                    break;

            }
        }
    #endif
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void buttonSetup()
{
    #if DEBUG_SUPPORT
        DPRINT(F("[BUTTON] Number of buttons: "));
        DPRINTLN(BUTTON_MAX_ITEMS);
    #endif
}

// -----------------------------------------------------------------------------

void buttonLoop()
{
    for (uint8_t i = 0; i < BUTTON_MAX_ITEMS; i++) {
        uint8_t event = button_module_items[i].button->loop();
        uint8_t count = button_module_items[i].button->getEventCount();
        unsigned long length = button_module_items[i].button->getEventLength();

        uint8_t mapped = _buttonMapEvent(event, count, length);

        _buttonEvent(i, mapped);
    }
}
