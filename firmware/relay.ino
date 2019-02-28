/*

RELAY MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if RELAY_PROVIDER != RELAY_PROVIDER_NONE

#include <Vector.h>
#include <EEPROM.h>

typedef struct {
    // Configuration variables
    byte pin;                   // GPIO pin for the relay
    byte type;                  // RELAY_TYPE_NORMAL, RELAY_TYPE_INVERSE, RELAY_TYPE_LATCHED or RELAY_TYPE_LATCHED_INVERSE
    byte reset_pin;             // GPIO to reset the relay if RELAY_TYPE_LATCHED

    uint8_t register_address;      // Address in communication register to store state

    unsigned long delay_on;     // Delay to turn relay ON
    unsigned long delay_off;    // Delay to turn relay OFF

    // Status variables
    bool current_status;        // Holds the current (physical) status of the relay
    bool target_status;         // Holds the target status
    unsigned long fw_start;     // Flood window start time
    byte fw_count;              // Number of changes within the current flood window
    unsigned long change_time;  // Scheduled time to change

} relay_t;

Vector<relay_t> _relays;

bool _relayRecursive = false;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

void _relayConfigure() {
    for (byte i = 0; i < relayCount(); i++) {
        if (_relays[i].pin == GPIO_NONE) {
            continue;
        }

        pinMode(_relays[i].pin, OUTPUT);

        if (_relays[i].reset_pin != GPIO_NONE) {
            pinMode(_relays[i].reset_pin, OUTPUT);
        }

        if (_relays[i].type == RELAY_TYPE_INVERSE) {
            // Set to high to block short opening of relay
            digitalWrite(_relays[i].pin, HIGH);
        }
    }
}

// -----------------------------------------------------------------------------

void _relayBoot() {
    _relayRecursive = true;

    // Walk the relays
    bool status;

    for (byte i = 0; i < relayCount(); i++) {
        byte boot_mode = (bool) EEPROM.read(_relays[i].register_address);

        #if DEBUG_SUPPORT
            DPRINT(F("[RELAY] Relay #"));
            DPRINT(i);
            DPRINT(F(" boot mode "));
            DPRINTLN(boot_mode);
        #endif

        status = false;

        switch (boot_mode) {
            case RELAY_BOOT_ON:
                status = true;
                break;

            case RELAY_BOOT_OFF:
            default:
                break;
        }

        _relays[i].current_status = !status;
        _relays[i].target_status = status;

        _relays[i].change_time = millis();

        // Store state into communication register
        communicationWriteDigitalOutput(_relays[i].register_address, status);
    }

    _relayRecursive = false;
}

// -----------------------------------------------------------------------------
// RELAY PROVIDERS
// -----------------------------------------------------------------------------

void _relayProviderStatus(
    byte id,
    bool status
) {
    // Check relay ID
    if (id >= relayCount()) {
        return;
    }

    // Store new current status
    _relays[id].current_status = status;

    #if RELAY_PROVIDER == RELAY_PROVIDER_RELAY
        // If this is a light, all dummy relays have already been processed above
        // we reach here if the user has toggled a physical relay
        if (_relays[id].type == RELAY_TYPE_NORMAL) {
            digitalWrite(_relays[id].pin, status);

        } else if (_relays[id].type == RELAY_TYPE_INVERSE) {
            digitalWrite(_relays[id].pin, !status);

        } else if (_relays[id].type == RELAY_TYPE_LATCHED || _relays[id].type == RELAY_TYPE_LATCHED_INVERSE) {
            bool pulse = RELAY_TYPE_LATCHED ? HIGH : LOW;

            digitalWrite(_relays[id].pin, !pulse);

            if (_relays[id].reset_pin != GPIO_NONE) {
                digitalWrite(_relays[id].reset_pin, !pulse);
            }

            if (status || (_relays[id].reset_pin == GPIO_NONE)) {
                digitalWrite(_relays[id].pin, pulse);

            } else {
                digitalWrite(_relays[id].reset_pin, pulse);
            }

            niceDelay(RELAY_LATCHING_PULSE);

            digitalWrite(_relays[id].pin, !pulse);

            if (_relays[id].reset_pin != GPIO_NONE) {
                digitalWrite(_relays[id].reset_pin, !pulse);
            }
        }
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Walks the relay vector processing only those relays
 * that have to change to the requested mode
 * @bool mode Requested mode
 */
void _relayProcess(
    bool mode
) {
    unsigned long current_time = millis();

    for (byte id = 0; id < relayCount(); id++) {
        // Only process the relays we have to change
        if (_relays[id].target_status == _relays[id].current_status) {
            continue;
        }

        // Only process the relays we have to change to the requested mode
        if (_relays[id].target_status != mode) {
            continue;
        }

        // Only process if the change_time has arrived
        if (current_time < _relays[id].change_time) {
            continue;
        }

        #if DEBUG_SUPPORT
            DPRINT(F("[RELAY] #"));
            DPRINT(id);
            DPRINT(F(" set to "));
            DPRINTLN(_relays[id].target_status ? F("ON") : F("OFF"));
        #endif

        // Call the provider to perform the action
        _relayProviderStatus(id, _relays[id].target_status);

        // Store state into communication register
        communicationWriteDigitalOutput(_relays[id].register_address, _relays[id].target_status);
    }
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

byte relayCount() {
    return _relays.size();
}

// -----------------------------------------------------------------------------

bool relayStatus(
    byte id
) {
    // Check relay ID
    if (id >= relayCount()) {
        return false;
    }

    // Get status from storage
    return _relays[id].current_status;
}

// -----------------------------------------------------------------------------

bool relayStatus(
    byte id,
    bool status
) {
    if (id >= relayCount()) {
        return false;
    }

    bool changed = false;

    if (_relays[id].current_status == status) {
        if (_relays[id].target_status != status) {
            #if DEBUG_SUPPORT
                DPRINT(F("[RELAY] #"));
                DPRINT(id);
                DPRINT(F(" scheduled change cancelled\n"));
            #endif

            _relays[id].target_status = status;

            changed = true;
        }

    } else {
        unsigned long current_time = millis();
        unsigned long fw_end = _relays[id].fw_start + 1000 * RELAY_FLOOD_WINDOW;
        unsigned long delay = status ? _relays[id].delay_on : _relays[id].delay_off;

        _relays[id].fw_count++;
        _relays[id].change_time = current_time + delay;

        // If current_time is off-limits the floodWindow...
        if (current_time < _relays[id].fw_start || fw_end <= current_time) {
            // We reset the floodWindow
            _relays[id].fw_start = current_time;
            _relays[id].fw_count = 1;

        // If current_time is in the floodWindow and there have been too many requests...
        } else if (_relays[id].fw_count >= RELAY_FLOOD_CHANGES) {
            // We schedule the changes to the end of the floodWindow
            // unless it's already delayed beyond that point
            if (fw_end - delay > current_time) {
                _relays[id].change_time = fw_end;
            }
        }

        _relays[id].target_status = status;

        relaySync(id);

        #if DEBUG_SUPPORT
            DPRINT(F("[RELAY] #"));
            DPRINT(id);
            DPRINT(F(" scheduled "));
            DPRINT(status ? F("ON") : F("OFF"));
            DPRINT(F(" in "));
            DPRINT(_relays[id].change_time - current_time);
            DPRINT(F(" ms\n"));
        #endif

        changed = true;

        EEPROM.update(_relays[id].register_address, status);
    }

    return changed;
}

// -----------------------------------------------------------------------------

void relayToggle(
    byte id
) {
    if (id >= relayCount()) {
        return;
    }

    relayStatus(id, !relayStatus(id));
}

// -----------------------------------------------------------------------------

void relaySync(
    byte id
) {
    // No sync if none or only one relay
    if (relayCount() < 2) {
        return;
    }

    // Do not go on if we are comming from a previous sync
    if (_relayRecursive) {
        return;
    }

    // Flag sync mode
    _relayRecursive = true;

    byte relaySync = RELAY_SYNC;
    bool status = _relays[id].target_status;

    // If RELAY_SYNC_SAME all relays should have the same state
    if (relaySync == RELAY_SYNC_SAME) {
        for (byte i = 0; i < relayCount(); i++) {
            if (i != id) {
                relayStatus(i, status);
            }
        }

    // If NONE_OR_ONE or ONE and setting ON we should set OFF all the others
    } else if (status) {
        if (relaySync != RELAY_SYNC_ANY) {
            for (byte i = 0; i < relayCount(); i++) {
                if (i != id) {
                    relayStatus(i, false);
                }
            }
        }

    // If ONLY_ONE and setting OFF we should set ON the other one
    } else {
        if (relaySync == RELAY_SYNC_ONE) {
            byte i = (id + 1) % relayCount();
            relayStatus(i, true);
        }
    }

    // Unflag sync mode
    _relayRecursive = false;
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void relaySetup() {
    uint8_t register_address;

    // Ad-hoc relays
    #if RELAY1_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY1_PIN, RELAY1_TYPE, RELAY1_RESET_PIN, register_address, RELAY1_DELAY_ON, RELAY1_DELAY_OFF });
    #endif
    #if RELAY2_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY2_PIN, RELAY2_TYPE, RELAY2_RESET_PIN, register_address, RELAY2_DELAY_ON, RELAY2_DELAY_OFF });
    #endif
    #if RELAY3_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY3_PIN, RELAY3_TYPE, RELAY3_RESET_PIN, register_address, RELAY3_DELAY_ON, RELAY3_DELAY_OFF });
    #endif
    #if RELAY4_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY4_PIN, RELAY4_TYPE, RELAY4_RESET_PIN, register_address, RELAY4_DELAY_ON, RELAY4_DELAY_OFF });
    #endif
    #if RELAY5_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY5_PIN, RELAY5_TYPE, RELAY5_RESET_PIN, register_address, RELAY5_DELAY_ON, RELAY5_DELAY_OFF });
    #endif
    #if RELAY6_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY6_PIN, RELAY6_TYPE, RELAY6_RESET_PIN, register_address, RELAY6_DELAY_ON, RELAY6_DELAY_OFF });
    #endif
    #if RELAY7_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY7_PIN, RELAY7_TYPE, RELAY7_RESET_PIN, register_address, RELAY7_DELAY_ON, RELAY7_DELAY_OFF });
    #endif
    #if RELAY8_PIN != GPIO_NONE
        register_address = communicationRegisterDigitalOutput();

        _relays.push_back((relay_t) { RELAY8_PIN, RELAY8_TYPE, RELAY8_RESET_PIN, register_address, RELAY8_DELAY_ON, RELAY8_DELAY_OFF });
    #endif

    _relayConfigure();
    _relayBoot();

    relayLoop();

    #if DEBUG_SUPPORT
        DPRINT(F("[RELAY] Number of relays: "));
        DPRINTLN(relayCount());
    #endif
}

//------------------------------------------------------------------------------

void relayLoop() {
    for (byte i = 0; i < relayCount(); i++) {
        if (communicationReadDigitalOutput(_relays[i].register_address) != relayStatus(i)) {
            relayStatus(i, communicationReadDigitalOutput(_relays[i].register_address));
        }
    }

    _relayProcess(false);
    _relayProcess(true);
}

#endif // RELAY_PROVIDER != RELAY_PROVIDER_NONE