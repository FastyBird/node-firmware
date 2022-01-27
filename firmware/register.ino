/*

REGISTER MODULE

Copyright (C) 2022 FastyBird s.r.o. <code@fastybird.com>

*/

#include "config/all.h"

#include <Arduino.h>

#if !defined(ARDUINO_ARCH_SAM) && !defined(ARDUINO_ARCH_SAMD) && !defined(ARDUINO_ARCH_STM32F2)
    #include <EEPROM.h>
#else
    #include <../lib/ArmEeprom/Samd21Eeprom.h>
#endif

// -----------------------------------------------------------------------------
// REGISTERS HELPERS
// -----------------------------------------------------------------------------

void _registerWriteToEeprom(
    const uint8_t type,
    const uint8_t address,
    const uint8_t flashAddress,
    const uint8_t * value
) {
    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {

        case REGISTER_DATA_TYPE_UINT8:
        case REGISTER_DATA_TYPE_INT8:
        {
            EEPROM.update(flashAddress, value[0]);
            break;
        }

        case REGISTER_DATA_TYPE_UINT16:
        case REGISTER_DATA_TYPE_INT16:
        {
            EEPROM.update(flashAddress, value[0]);
            EEPROM.update((flashAddress + 1), value[1]);
            break;
        }

        case REGISTER_DATA_TYPE_UINT32:
        case REGISTER_DATA_TYPE_INT32:
        case REGISTER_DATA_TYPE_FLOAT32:
        {
            EEPROM.update(flashAddress, value[0]);
            EEPROM.update((flashAddress + 1), value[1]);
            EEPROM.update((flashAddress + 2), value[2]);
            EEPROM.update((flashAddress + 3), value[3]);
            break;
        }

        case REGISTER_DATA_TYPE_BOOLEAN:
        {
            BOOLEAN_UNION_t bool_write_value;

            bool_write_value.bytes[0] = value[0];
            bool_write_value.bytes[1] = value[1];
            bool_write_value.bytes[2] = value[2];
            bool_write_value.bytes[3] = value[3];

            EEPROM.update(flashAddress, (bool_write_value.number == REGISTER_BOOLEAN_VALUE_TRUE));
            break;
        }

    }
}

void _registerInitializeFromEeprom(
    const uint8_t type,
    const uint8_t address,
    const uint8_t flashAddress
) {
    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {
        case REGISTER_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_stored_value;

            uint8_stored_value.bytes[0] = EEPROM.read(flashAddress);
            uint8_stored_value.bytes[1] = 0;
            uint8_stored_value.bytes[2] = 0;
            uint8_stored_value.bytes[3] = 0;

            _registerWriteRegister(type, address, uint8_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_stored_value;

            uint16_stored_value.bytes[0] = EEPROM.read(flashAddress);
            uint16_stored_value.bytes[1] = EEPROM.read(flashAddress + 1);
            uint16_stored_value.bytes[2] = 0;
            uint16_stored_value.bytes[3] = 0;

            _registerWriteRegister(type, address, uint16_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_stored_value;

            uint32_stored_value.bytes[0] = EEPROM.read(flashAddress);
            uint32_stored_value.bytes[1] = EEPROM.read(flashAddress + 1);
            uint32_stored_value.bytes[2] = EEPROM.read(flashAddress + 2);
            uint32_stored_value.bytes[3] = EEPROM.read(flashAddress + 3);

            _registerWriteRegister(type, address, uint32_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_INT8:
            INT8_UNION_t int8_stored_value;

            int8_stored_value.bytes[0] = EEPROM.read(flashAddress);
            int8_stored_value.bytes[1] = 0;
            int8_stored_value.bytes[2] = 0;
            int8_stored_value.bytes[3] = 0;

            _registerWriteRegister(type, address, int8_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_INT16:
            INT16_UNION_t int16_stored_value;

            int16_stored_value.bytes[0] = EEPROM.read(flashAddress);
            int16_stored_value.bytes[1] = EEPROM.read(flashAddress + 1);
            int16_stored_value.bytes[2] = 0;
            int16_stored_value.bytes[3] = 0;

            _registerWriteRegister(type, address, int16_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_INT32:
            INT32_UNION_t int32_stored_value;

            int32_stored_value.bytes[0] = EEPROM.read(flashAddress);
            int32_stored_value.bytes[1] = EEPROM.read(flashAddress + 1);
            int32_stored_value.bytes[2] = EEPROM.read(flashAddress + 2);
            int32_stored_value.bytes[3] = EEPROM.read(flashAddress + 3);

            _registerWriteRegister(type, address, int32_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_stored_value;

            float_stored_value.bytes[0] = EEPROM.read(flashAddress);
            float_stored_value.bytes[1] = EEPROM.read(flashAddress + 1);
            float_stored_value.bytes[2] = EEPROM.read(flashAddress + 2);
            float_stored_value.bytes[3] = EEPROM.read(flashAddress + 3);

            _registerWriteRegister(type, address, float_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_BOOLEAN:
            bool bool_memory_value = EEPROM.read(flashAddress);

            _registerWriteRegister(type, address, (bool_memory_value ? REGISTER_BOOLEAN_VALUE_TRUE : REGISTER_BOOLEAN_VALUE_FALSE), false);
            break;
    }
}

// -----------------------------------------------------------------------------

bool _registerReadRegister(
    const uint8_t type,
    const uint8_t address,
    const uint8_t dataTypeSize,
    void * value
) {
    // Reset to default
    memset(value, 0, dataTypeSize);

    if (type == REGISTER_TYPE_INPUT && address < REGISTER_MAX_INPUT_REGISTERS_SIZE) {
        memcpy(value, register_module_input_registers[address].value, dataTypeSize);

        return true;

    } else if (type == REGISTER_TYPE_OUTPUT && address < REGISTER_MAX_OUTPUT_REGISTERS_SIZE) {
        memcpy(value, register_module_output_registers[address].value, dataTypeSize);

        return true;

    } else if (type == REGISTER_TYPE_ATTRIBUTE && address < REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE) {
        memcpy(value, register_module_attribute_registers[address].value, dataTypeSize);

        return true;
    }

    return false;
}

// Specialized convenience setters (these do not cost memory because of inlining)
bool _registerReadRegister(const uint8_t type, const uint8_t address, uint8_t &value) { return _registerReadRegister(type, address, 1, &value); }
bool _registerReadRegister(const uint8_t type, const uint8_t address, uint16_t &value) { return _registerReadRegister(type, address, 2, &value); }
bool _registerReadRegister(const uint8_t type, const uint8_t address, uint32_t &value) { return _registerReadRegister(type, address, 4, &value); }
bool _registerReadRegister(const uint8_t type, const uint8_t address, int8_t &value) { return _registerReadRegister(type, address, 1, &value); }
bool _registerReadRegister(const uint8_t type, const uint8_t address, int16_t &value) { return _registerReadRegister(type, address, 2, &value); }
bool _registerReadRegister(const uint8_t type, const uint8_t address, int32_t &value) { return _registerReadRegister(type, address, 4, &value); }
bool _registerReadRegister(const uint8_t type, const uint8_t address, float &value) { return _registerReadRegister(type, address, 4, &value); }

// -----------------------------------------------------------------------------

bool _registerReadRegisterAsBytes(
    const uint8_t type,
    const uint8_t address,
    uint8_t * value
) {
    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {
        case REGISTER_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_read_value;

            _registerReadRegister(type, address, uint8_read_value.number);

            memcpy(value, uint8_read_value.bytes, 1);

            return true;

        case REGISTER_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_read_value;

            _registerReadRegister(type, address, uint16_read_value.number);

            memcpy(value, uint16_read_value.bytes, 2);

            return true;

        case REGISTER_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_read_value;

            _registerReadRegister(type, address, uint32_read_value.number);

            memcpy(value, uint32_read_value.bytes, 4);

            return true;

        case REGISTER_DATA_TYPE_INT8:
            INT8_UNION_t int8_read_value;

            _registerReadRegister(type, address, int8_read_value.number);

            memcpy(value, int8_read_value.bytes, 1);

            return true;

        case REGISTER_DATA_TYPE_INT16:
            INT16_UNION_t int16_read_value;

            _registerReadRegister(type, address, int16_read_value.number);

            memcpy(value, int16_read_value.bytes, 2);

            return true;

        case REGISTER_DATA_TYPE_INT32:
            INT32_UNION_t int32_read_value;

            _registerReadRegister(type, address, int32_read_value.number);

            memcpy(value, int32_read_value.bytes, 4);

            return true;

        case REGISTER_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_read_value;

            _registerReadRegister(type, address, float_read_value.number);

            memcpy(value, float_read_value.bytes, 4);

            return true;

        case REGISTER_DATA_TYPE_BOOLEAN:
            BOOLEAN_UNION_t bool_read_value;

            _registerReadRegister(type, address, bool_read_value.number);

            memcpy(value, bool_read_value.bytes, 4);

            return true;

        default:
            memset(value, 0, 4);

            return false;
    }
}

// -----------------------------------------------------------------------------

bool _registerWriteRegister(
    const uint8_t type,
    const uint8_t address,
    const uint8_t dataTypeSize,
    const void * value,
    const bool propagate
) {
    if (type == REGISTER_TYPE_INPUT && address >= REGISTER_MAX_INPUT_REGISTERS_SIZE) {
        return false;

    } else if (type == REGISTER_TYPE_OUTPUT && address >= REGISTER_MAX_OUTPUT_REGISTERS_SIZE) {
        return false;

    } else if (type == REGISTER_TYPE_ATTRIBUTE && address >= REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE) {
        return false;
    }

    uint8_t old_value[4] = { 0, 0, 0, 0 };

    if (_registerReadRegisterAsBytes(type, address, old_value) == false) {
        return false;
    }

    uint8_t flash_address = INDEX_NONE;

    if (type == REGISTER_TYPE_INPUT) {
        memcpy(register_module_input_registers[address].value, value, dataTypeSize);

        flash_address = register_module_input_registers[address].flash_address;

    } else if (type == REGISTER_TYPE_OUTPUT) {
        memcpy(register_module_output_registers[address].value, value, dataTypeSize);

        flash_address = register_module_output_registers[address].flash_address;

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        memcpy(register_module_attribute_registers[address].value, value, dataTypeSize);

        flash_address = register_module_attribute_registers[address].flash_address;

    } else {
        return false;
    }

    if (memcmp((const void *) old_value, (const void *) value, dataTypeSize) != 0) {
        #if DEBUG_SUPPORT
            DPRINT(F("[REGISTER] Value was written into: "));
            DPRINT(type);
            DPRINT(F(" register at address "));
            DPRINTLN(address);
        #endif

        if (flash_address != INDEX_NONE) {
            uint8_t stored_value[4] = { 0, 0, 0, 0 };

            if (_registerReadRegisterAsBytes(type, address, stored_value) == true) {
                if (
                    type == REGISTER_TYPE_ATTRIBUTE
                    && address == COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS
                ) {
                    uint8_t device_state;

                    registerReadRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS, device_state);

                    if (device_state == DEVICE_STATE_PAIRING) {
                        uint8_t store_value[4] = { DEVICE_STATE_STOPPED, 0, 0, 0 };

                        // Store value in memory
                        _registerWriteToEeprom(type, address, flash_address, store_value);

                    } else {
                        // Store value in memory
                        _registerWriteToEeprom(type, address, flash_address, stored_value);                        
                    }

                } else {
                    // Store value in memory
                    _registerWriteToEeprom(type, address, flash_address, stored_value);
                }
            }
        }

        if (propagate) {
            communicationReportRegister(type, address);
        }

        //if (type == REGISTER_TYPE_ATTRIBUTE) {
            // Special handling for communication address stored in registry
            //if (address == COMMUNICATION_ATTR_REGISTER_ADDR_ADDRESS && firmwareIsBooting() == false) {
                // Little delay before reboot
                //delay(500);

                // ...after address is stored, reload device
                //resetFunc();
            //}
        //}
    #if DEBUG_SUPPORT
    } else {
        DPRINT(F("[REGISTER] Value to write into: "));
        DPRINT(type);
        DPRINT(F(" register at address "));
        DPRINT(address);
        DPRINTLN(F(" is same as stored. Write skipped"));
    #endif
    }

    return true;
}

// Specialized convenience setters (these do not cost memory because of inlining)
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const uint8_t value, const bool propagate) { return _registerWriteRegister(type, address, 1, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const uint16_t value, const bool propagate) { return _registerWriteRegister(type, address, 2, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const uint32_t value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const int8_t value, const bool propagate) { return _registerWriteRegister(type, address, 1, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const int16_t value, const bool propagate) { return _registerWriteRegister(type, address, 2, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const int32_t value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const float value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const bool value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }

// -----------------------------------------------------------------------------

bool _registerWriteRegisterFromBytes(
    const uint8_t type,
    const uint8_t address,
    uint8_t * value,
    const bool propagate
) {
    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {
        case REGISTER_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_write_value;

            uint8_write_value.bytes[0] = value[0];
            uint8_write_value.bytes[1] = 0;
            uint8_write_value.bytes[2] = 0;
            uint8_write_value.bytes[3] = 0;

            _registerWriteRegister(type, address, uint8_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_write_value;

            uint16_write_value.bytes[0] = value[0];
            uint16_write_value.bytes[1] = value[1];
            uint16_write_value.bytes[2] = 0;
            uint16_write_value.bytes[3] = 0;

            _registerWriteRegister(type, address, uint16_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_write_value;

            uint32_write_value.bytes[0] = value[0];
            uint32_write_value.bytes[1] = value[1];
            uint32_write_value.bytes[2] = value[2];
            uint32_write_value.bytes[3] = value[3];

            _registerWriteRegister(type, address, uint32_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_INT8:
            INT8_UNION_t int8_write_value;

            int8_write_value.bytes[0] = value[0];
            int8_write_value.bytes[1] = 0;
            int8_write_value.bytes[2] = 0;
            int8_write_value.bytes[3] = 0;

            _registerWriteRegister(type, address, int8_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_INT16:
            INT16_UNION_t int16_write_value;

            int16_write_value.bytes[0] = value[0];
            int16_write_value.bytes[1] = value[1];
            int16_write_value.bytes[2] = 0;
            int16_write_value.bytes[3] = 0;

            _registerWriteRegister(type, address, int16_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_INT32:
            INT32_UNION_t int32_write_value;

            int32_write_value.bytes[0] = value[0];
            int32_write_value.bytes[1] = value[1];
            int32_write_value.bytes[2] = value[2];
            int32_write_value.bytes[3] = value[3];

            _registerWriteRegister(type, address, int32_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_write_value;

            float_write_value.bytes[0] = value[0];
            float_write_value.bytes[1] = value[1];
            float_write_value.bytes[2] = value[2];
            float_write_value.bytes[3] = value[3];

            _registerWriteRegister(type, address, float_write_value.number, propagate);

            return true;

        case REGISTER_DATA_TYPE_BOOLEAN:
            BOOLEAN_UNION_t bool_write_value;

            bool_write_value.bytes[0] = value[0];
            bool_write_value.bytes[1] = value[1];
            bool_write_value.bytes[2] = value[2];
            bool_write_value.bytes[3] = value[3];

            _registerWriteRegister(type, address, bool_write_value.number, propagate);

            return true;

        default:
            return false;
    }
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

/**
 * Get register configured data type
 */
uint8_t registerGetRegisterDataType(
    const uint8_t type,
    const uint8_t address
) {
    if (type == REGISTER_TYPE_INPUT && address < REGISTER_MAX_INPUT_REGISTERS_SIZE) {
        return register_module_input_registers[address].data_type;

    } else if (type == REGISTER_TYPE_OUTPUT && address < REGISTER_MAX_OUTPUT_REGISTERS_SIZE) {
        return register_module_output_registers[address].data_type;

    } else if (type == REGISTER_TYPE_ATTRIBUTE && address < REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE) {
        return register_module_attribute_registers[address].data_type;
    }

    return REGISTER_DATA_TYPE_UNKNOWN;
}

// -----------------------------------------------------------------------------


// Specialized convenience setters (these do not cost memory because of inlining)
bool registerReadRegister(const uint8_t type, const uint8_t address, uint8_t &value) { return _registerReadRegister(type, address, 1, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, uint16_t &value) { return _registerReadRegister(type, address, 2, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, uint32_t &value) { return _registerReadRegister(type, address, 4, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, int8_t &value) { return _registerReadRegister(type, address, 1, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, int16_t &value) { return _registerReadRegister(type, address, 2, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, int32_t &value) { return _registerReadRegister(type, address, 4, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, float &value) { return _registerReadRegister(type, address, 4, &value); }
// Specialized for transforming to boolean
bool registerReadRegister(const uint8_t type, const uint8_t address, bool &value) {
    uint32_t bool_read_value;

    if (_registerReadRegister(type, address, bool_read_value) == false) {
        return false;
    }

    value = bool_read_value == REGISTER_BOOLEAN_VALUE_TRUE;

    return true;
}
// Specialized for transforming to bytes
bool registerReadRegister(const uint8_t type, const uint8_t address, uint8_t * value) { return _registerReadRegisterAsBytes(type, address, value); }

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint8_t value, const bool propagate) { return _registerWriteRegister(type, address, 1, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint16_t value, const bool propagate) { return _registerWriteRegister(type, address, 2, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint32_t value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int8_t value, const bool propagate) { return _registerWriteRegister(type, address, 1, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int16_t value, const bool propagate) { return _registerWriteRegister(type, address, 2, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int32_t value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const float value, const bool propagate) { return _registerWriteRegister(type, address, 4, &value, propagate); }
// Specialized for transforming to boolean
bool registerWriteRegister(const uint8_t type, const uint8_t address, const bool value, const bool propagate) { return _registerWriteRegister(type, address, (value ? REGISTER_BOOLEAN_VALUE_TRUE : REGISTER_BOOLEAN_VALUE_FALSE), propagate); }
// Specialized for transforming from bytes
bool registerWriteRegister(const uint8_t type, const uint8_t address, uint8_t * value, const bool propagate) { return _registerWriteRegisterFromBytes(type, address, value, propagate); }

bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint8_t value) { return _registerWriteRegister(type, address, 1, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint16_t value) { return _registerWriteRegister(type, address, 2, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint32_t value) { return _registerWriteRegister(type, address, 4, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int8_t value) { return _registerWriteRegister(type, address, 1, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int16_t value) { return _registerWriteRegister(type, address, 2, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int32_t value) { return _registerWriteRegister(type, address, 4, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const float value) { return _registerWriteRegister(type, address, 4, &value, true); }
// Specialized for transforming to boolean
bool registerWriteRegister(const uint8_t type, const uint8_t address, const bool value) { return _registerWriteRegister(type, address, (value ? REGISTER_BOOLEAN_VALUE_TRUE : REGISTER_BOOLEAN_VALUE_FALSE), true); }
// Specialized for transforming from bytes
bool registerWriteRegister(const uint8_t type, const uint8_t address, uint8_t * value) { return _registerWriteRegisterFromBytes(type, address, value, true); }

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void registerSetup()
{
    #if REGISTER_MAX_INPUT_REGISTERS_SIZE
        for(int i = 0; i < REGISTER_MAX_INPUT_REGISTERS_SIZE; ++i) {
            if (register_module_input_registers[i].flash_address != INDEX_NONE) {
                _registerInitializeFromEeprom(REGISTER_TYPE_INPUT, i, register_module_input_registers[i].flash_address);
            }
        }
    #endif

    #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
        for(int i = 0; i < REGISTER_MAX_OUTPUT_REGISTERS_SIZE; ++i) {
            if (register_module_output_registers[i].flash_address != INDEX_NONE) {
                _registerInitializeFromEeprom(REGISTER_TYPE_OUTPUT, i, register_module_output_registers[i].flash_address);
            }
        }
    #endif

    #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
        for(int i = 0; i < REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE; ++i) {
            if (register_module_attribute_registers[i].flash_address != INDEX_NONE) {
                _registerInitializeFromEeprom(REGISTER_TYPE_ATTRIBUTE, i, register_module_attribute_registers[i].flash_address);
            }
        }
    #endif
}
