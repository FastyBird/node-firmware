/*

RELAY MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#if RELAY_PROVIDER != RELAY_PROVIDER_NONE

bool _relayRecursive = false;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

void _relayConfigure()
{
    for (uint8_t i = 0; i < RELAY_MAX_ITEMS; i++) {
        if (relay_module_items[i].pin == GPIO_NONE) {
            continue;
        }

        pinMode(relay_module_items[i].pin, OUTPUT);

        if (relay_module_items[i].reset_pin != GPIO_NONE) {
            pinMode(relay_module_items[i].reset_pin, OUTPUT);
        }

        if (relay_module_items[i].type == RELAY_TYPE_INVERSE) {
            // Set to high to block short opening of relay
            digitalWrite(relay_module_items[i].pin, HIGH);
        }
    }
}

// -----------------------------------------------------------------------------

void _relayBoot()
{
    _relayRecursive = true;

    // Walk the relays
    bool status;

    for (uint8_t i = 0; i < RELAY_MAX_ITEMS; i++) {
        uint8_t boot_mode = (bool) EEPROM.read(relay_module_items[i].memory_address);

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

        relay_module_items[i].current_status = !status;
        relay_module_items[i].target_status = status;

        relay_module_items[i].change_time = millis();

        // Store state into communication register
        communicationWriteAnalogOutput(relay_module_items[i].register_address, (status ? 0xFF00 : 0x0000));
    }

    _relayRecursive = false;
}

// -----------------------------------------------------------------------------
// RELAY PROVIDERS
// -----------------------------------------------------------------------------

void _relayProviderStatus(
    const uint8_t id,
    const bool status
) {
    // Check relay ID
    if (id >= RELAY_MAX_ITEMS) {
        return;
    }

    // Store new current status
    relay_module_items[id].current_status = status;

    #if RELAY_PROVIDER == RELAY_PROVIDER_RELAY
        // If this is a light, all dummy relays have already been processed above
        // we reach here if the user has toggled a physical relay
        if (relay_module_items[id].type == RELAY_TYPE_NORMAL) {
            digitalWrite(relay_module_items[id].pin, status);

        } else if (relay_module_items[id].type == RELAY_TYPE_INVERSE) {
            digitalWrite(relay_module_items[id].pin, !status);

        } else if (relay_module_items[id].type == RELAY_TYPE_LATCHED || relay_module_items[id].type == RELAY_TYPE_LATCHED_INVERSE) {
            bool pulse = RELAY_TYPE_LATCHED ? HIGH : LOW;

            digitalWrite(relay_module_items[id].pin, !pulse);

            if (relay_module_items[id].reset_pin != GPIO_NONE) {
                digitalWrite(relay_module_items[id].reset_pin, !pulse);
            }

            if (status || (relay_module_items[id].reset_pin == GPIO_NONE)) {
                digitalWrite(relay_module_items[id].pin, pulse);

            } else {
                digitalWrite(relay_module_items[id].reset_pin, pulse);
            }

            niceDelay(RELAY_LATCHING_PULSE);

            digitalWrite(relay_module_items[id].pin, !pulse);

            if (relay_module_items[id].reset_pin != GPIO_NONE) {
                digitalWrite(relay_module_items[id].reset_pin, !pulse);
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
    const bool mode
) {
    unsigned long current_time = millis();

    for (uint8_t id = 0; id < RELAY_MAX_ITEMS; id++) {
        // Only process the relays we have to change
        if (relay_module_items[id].target_status == relay_module_items[id].current_status) {
            continue;
        }

        // Only process the relays we have to change to the requested mode
        if (relay_module_items[id].target_status != mode) {
            continue;
        }

        // Only process if the change_time has arrived
        if (current_time < relay_module_items[id].change_time) {
            continue;
        }

        #if DEBUG_SUPPORT
            DPRINT(F("[RELAY] #"));
            DPRINT(id);
            DPRINT(F(" set to "));
            DPRINTLN(relay_module_items[id].target_status ? F("ON") : F("OFF"));
        #endif

        // Call the provider to perform the action
        _relayProviderStatus(id, relay_module_items[id].target_status);

        // Store state into communication register
        communicationWriteAnalogOutput(relay_module_items[id].register_address, (relay_module_items[id].target_status ? 0xFF00 : 0x0000));
    }
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

bool relayStatus(
    const uint8_t id
) {
    // Check relay ID
    if (id >= RELAY_MAX_ITEMS) {
        return false;
    }

    // Get status from storage
    return relay_module_items[id].current_status;
}

// -----------------------------------------------------------------------------

bool relayStatus(
    const uint8_t id,
    const bool status
) {
    if (id >= RELAY_MAX_ITEMS) {
        return false;
    }

    bool changed = false;

    if (relay_module_items[id].current_status == status) {
        if (relay_module_items[id].target_status != status) {
            #if DEBUG_SUPPORT
                DPRINT(F("[RELAY] #"));
                DPRINT(id);
                DPRINT(F(" scheduled change cancelled\n"));
            #endif

            relay_module_items[id].target_status = status;

            changed = true;
        }

    } else {
        unsigned long current_time = millis();
        unsigned long fw_end = relay_module_items[id].fw_start + 1000 * RELAY_FLOOD_WINDOW;
        unsigned long delay = status ? relay_module_items[id].delay_on : relay_module_items[id].delay_off;

        relay_module_items[id].fw_count++;
        relay_module_items[id].change_time = current_time + delay;

        // If current_time is off-limits the floodWindow...
        if (current_time < relay_module_items[id].fw_start || fw_end <= current_time) {
            // We reset the floodWindow
            relay_module_items[id].fw_start = current_time;
            relay_module_items[id].fw_count = 1;

        // If current_time is in the floodWindow and there have been too many requests...
        } else if (relay_module_items[id].fw_count >= RELAY_FLOOD_CHANGES) {
            // We schedule the changes to the end of the floodWindow
            // unless it's already delayed beyond that point
            if (fw_end - delay > current_time) {
                relay_module_items[id].change_time = fw_end;
            }
        }

        relay_module_items[id].target_status = status;

        relaySync(id);

        #if DEBUG_SUPPORT
            DPRINT(F("[RELAY] #"));
            DPRINT(id);
            DPRINT(F(" scheduled "));
            DPRINT(status ? F("ON") : F("OFF"));
            DPRINT(F(" in "));
            DPRINT(relay_module_items[id].change_time - current_time);
            DPRINT(F(" ms\n"));
        #endif

        changed = true;

        EEPROM.update(relay_module_items[id].memory_address, status);
    }

    return changed;
}

// -----------------------------------------------------------------------------

void relayToggle(
    const uint8_t id
) {
    if (id >= RELAY_MAX_ITEMS) {
        return;
    }

    relayStatus(id, !relayStatus(id));
}

// -----------------------------------------------------------------------------

void relaySync(
    const uint8_t id
) {
    // No sync if none or only one relay
    if (RELAY_MAX_ITEMS < 2) {
        return;
    }

    // Do not go on if we are comming from a previous sync
    if (_relayRecursive) {
        return;
    }

    // Flag sync mode
    _relayRecursive = true;

    uint8_t relaySync = RELAY_SYNC;
    bool status = relay_module_items[id].target_status;

    // If RELAY_SYNC_SAME all relays should have the same state
    if (relaySync == RELAY_SYNC_SAME) {
        for (uint8_t i = 0; i < RELAY_MAX_ITEMS; i++) {
            if (i != id) {
                relayStatus(i, status);
            }
        }

    // If NONE_OR_ONE or ONE and setting ON we should set OFF all the others
    } else if (status) {
        if (relaySync != RELAY_SYNC_ANY) {
            for (uint8_t i = 0; i < RELAY_MAX_ITEMS; i++) {
                if (i != id) {
                    relayStatus(i, false);
                }
            }
        }

    // If ONLY_ONE and setting OFF we should set ON the other one
    } else {
        if (relaySync == RELAY_SYNC_ONE) {
            uint8_t i = (id + 1) % RELAY_MAX_ITEMS;
            relayStatus(i, true);
        }
    }

    // Unflag sync mode
    _relayRecursive = false;
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void relaySetup()
{
    _relayConfigure();
    _relayBoot();

    relayLoop();

    #if DEBUG_SUPPORT
        DPRINT(F("[RELAY] Number of relays: "));
        DPRINTLN(RELAY_MAX_ITEMS);
    #endif
}

//------------------------------------------------------------------------------

void relayLoop()
{
    // Process request only if device is in running mode
    if (firmwareIsRunning()) {
        for (uint8_t i = 0; i < RELAY_MAX_ITEMS; i++) {
            uint16_t register_value;

            communicationReadAnalogOutput(relay_module_items[i].register_address, register_value);

            bool expected_value = register_value == 0xFF00;

            if (expected_value != relayStatus(i)) {
                relayStatus(i, expected_value);
            }
        }
    }

    _relayProcess(false);
    _relayProcess(true);
}

#endif // RELAY_PROVIDER != RELAY_PROVIDER_NONE