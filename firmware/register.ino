/*

REGISTER MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/


// -----------------------------------------------------------------------------
// REGISTERS HELPERS
// -----------------------------------------------------------------------------

uint8_t _registerGetRegisterDataTypeSize(
    const uint8_t type,
    const uint8_t address,
    const uint8_t dataType
) {
    switch (dataType)
    {
        case REGISTER_DATA_TYPE_UINT8:
        case REGISTER_DATA_TYPE_INT8:
            return 1;
    
        case REGISTER_DATA_TYPE_UINT16:
        case REGISTER_DATA_TYPE_INT16:
        case REGISTER_DATA_TYPE_BOOLEAN:
            return 2;
    
        case REGISTER_DATA_TYPE_UINT32:
        case REGISTER_DATA_TYPE_INT32:
        case REGISTER_DATA_TYPE_FLOAT32:
            return 4;

        default:
            return 0;
    }
}

// -----------------------------------------------------------------------------

uint8_t _registerGetRegisterDataTypeSize(
    const uint8_t type,
    const uint8_t address
) {
    if (type == REGISTER_TYPE_INPUT) {
        return _registerGetRegisterDataTypeSize(type, address, register_module_input_registers[address].data_type);

    } else if (type == REGISTER_TYPE_OUTPUT) {
        return _registerGetRegisterDataTypeSize(type, address, register_module_output_registers[address].data_type);

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        return _registerGetRegisterDataTypeSize(type, address, register_module_attribute_registers[address].data_type);
    }

    return 0;
}

// -----------------------------------------------------------------------------

bool _registerReadRegister(
    const uint8_t type,
    const uint8_t address,
    void * value
) {
    uint8_t size = _registerGetRegisterDataTypeSize(type, address);

    if (type == REGISTER_TYPE_INPUT) {
        if (address < REGISTER_MAX_INPUT_REGISTERS_SIZE) {
            memcpy(value, register_module_input_registers[address].value, size);

            return true;
        }

    } else if (type == REGISTER_TYPE_OUTPUT) {
        if (address < REGISTER_MAX_OUTPUT_REGISTERS_SIZE) {
            memcpy(value, register_module_output_registers[address].value, size);

            return true;
        }

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        if (address < REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE) {
            memcpy(value, register_module_attribute_registers[address].value, size);

            return true;
        }
    }

    char default_value[4] = { 0, 0, 0, 0 };

    memcpy(value, default_value, size);

    return false;
}

// -----------------------------------------------------------------------------

bool _registerWriteRegister(
    const uint8_t type,
    const uint8_t address,
    const void * value,
    const bool propagate
) {
    uint8_t registers_size;

    if (type == REGISTER_TYPE_OUTPUT) {
        registers_size = REGISTER_MAX_OUTPUT_REGISTERS_SIZE;

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        registers_size = REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE;

    } else {
        return false;
    }

    if (address >= registers_size) {
        return false;
    }

    #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
        if (type == REGISTER_TYPE_ATTRIBUTE && register_module_attribute_registers[address].settable == false) {
            return false;
        }
    #endif

    uint8_t size = _registerGetRegisterDataTypeSize(type, address);

    char stored_value[4] = { 0, 0, 0, 0 };

    _registerReadRegister(type, address, stored_value);

    if (type == REGISTER_TYPE_INPUT) {
        if (address > REGISTER_MAX_INPUT_REGISTERS_SIZE) {
            return false;
        }

        memcpy(register_module_input_registers[address].value, value, size);

    } else if (type == REGISTER_TYPE_OUTPUT) {
        if (address > REGISTER_MAX_OUTPUT_REGISTERS_SIZE) {
            return false;
        }

        memcpy(register_module_output_registers[address].value, value, size);

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        if (address > REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE) {
            return false;
        }

        memcpy(register_module_attribute_registers[address].value, value, size);
    }

    if (memcmp((const void *) stored_value, (const void *) value, sizeof(stored_value)) != 0) {
        if (propagate) {
            char read_value[4] = { 0, 0, 0, 0 };

            _registerReadRegister(type, address, read_value);

            communicationReportRegister(type, address, read_value);
        }

        // Store value in memory
        // EEPROM.update(FLASH_ADDRESS_DEVICE_STATE, _firmware_device_state);

        if (type == REGISTER_TYPE_ATTRIBUTE) {
            // Special handling for communication address stored in registry
            if (address == COMMUNICATION_ATTR_REGISTER_ADDR_ADDRESS) {
                // ...after address is stored, reload device
                // resetFunc();
            }
        }
    #if DEBUG_SUPPORT
        DPRINT(F("[REGISTER] Value was written into: "));
        DPRINT(type);
        DPRINTLN(F(" register"));
    } else {
        DPRINT(F("[REGISTER] Value to write into: "));
        DPRINT(type);
        DPRINTLN(F(" register is same as stored. Write skipped"));
    #endif
    }

    return true;
}

// Specialized convenience setters (these do not cost memory because of inlining)
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const uint8_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const uint16_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const uint32_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const int8_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const int16_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const int32_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const float value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool _registerWriteRegister(const uint8_t type, const uint8_t address, const bool value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }

// -----------------------------------------------------------------------------

void _registerReadFromEeprom(
    const uint8_t type,
    const uint8_t address,
    const uint8_t flashAddress
) {
    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {
        case REGISTER_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_stored_value;

            EEPROM.get(flashAddress, uint8_stored_value.number);

            _registerWriteRegister(type, address, uint8_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_stored_value;

            EEPROM.get(flashAddress, uint16_stored_value.number);

            _registerWriteRegister(type, address, uint16_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_stored_value;

            EEPROM.get(flashAddress, uint32_stored_value.number);

            _registerWriteRegister(type, address, uint32_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_INT8:
            INT8_UNION_t int8_stored_value;

            EEPROM.get(flashAddress, int8_stored_value.number);

            _registerWriteRegister(type, address, int8_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_INT16:
            INT16_UNION_t int16_stored_value;

            EEPROM.get(flashAddress, int16_stored_value.number);

            _registerWriteRegister(type, address, int16_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_INT32:
            INT32_UNION_t int32_stored_value;

            EEPROM.get(flashAddress, int32_stored_value.number);

            _registerWriteRegister(type, address, int32_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_stored_value;

            EEPROM.get(flashAddress, float_stored_value.number);

            _registerWriteRegister(type, address, float_stored_value.number, false);
            break;

        case REGISTER_DATA_TYPE_BOOLEAN:
            BOOLEAN_UNION_t bool_stored_value;

            EEPROM.get(flashAddress, bool_stored_value.number);

            _registerWriteRegister(type, address, bool_stored_value.number, false);
            break;
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
    if (type == REGISTER_TYPE_INPUT) {
        return register_module_input_registers[address].data_type;

    } else if (type == REGISTER_TYPE_OUTPUT) {
        return register_module_output_registers[address].data_type;

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        return register_module_attribute_registers[address].data_type;
    }

    return REGISTER_DATA_TYPE_UNKNOWN;
}

// -----------------------------------------------------------------------------

/**
 * Read value from register as array representation
 */
bool registerReadRegister(
    const uint8_t type,
    const uint8_t address,
    char * value
) {
    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {
        case REGISTER_DATA_TYPE_UINT8:
        {
            UINT8_UNION_t uint8_read_value;

            const bool result = registerReadRegister(type, address, uint8_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, uint8_read_value.bytes, 1);
            return true;
        }

        case REGISTER_DATA_TYPE_UINT16:
        {
            UINT16_UNION_t uint16_read_value;

            const bool result = registerReadRegister(type, address, uint16_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, uint16_read_value.bytes, 2);
            return true;
        }

        case REGISTER_DATA_TYPE_UINT32:
        {
            UINT32_UNION_t uint32_read_value;

            const bool result = registerReadRegister(type, address, uint32_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, uint32_read_value.bytes, 4);
            return true;
        }

        case REGISTER_DATA_TYPE_INT8:
        {
            INT8_UNION_t int8_read_value;

            const bool result = registerReadRegister(type, address, int8_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, int8_read_value.bytes, 1);
            return true;
        }

        case REGISTER_DATA_TYPE_INT16:
        {
            INT16_UNION_t int16_read_value;

            const bool result = registerReadRegister(type, address, int16_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, int16_read_value.bytes, 2);
            return true;
        }

        case REGISTER_DATA_TYPE_INT32:
        {
            INT32_UNION_t int32_read_value;

            const bool result = registerReadRegister(type, address, int32_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, int32_read_value.bytes, 4);
            return true;
        }

        case REGISTER_DATA_TYPE_FLOAT32:
        {
            FLOAT32_UNION_t float_read_value;

            const bool result = registerReadRegister(type, address, float_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, float_read_value.bytes, 4);
            return true;
        }

        case REGISTER_DATA_TYPE_BOOLEAN:
        {
            BOOLEAN_UNION_t bool_read_value;

            const bool result = registerReadRegister(type, address, bool_read_value.number);

            if (result == false) {
                return false;
            }

            memcpy(value, bool_read_value.bytes, 2);
            return true;
        }
    
        default:
            char buffer[4] = { 0, 0, 0, 0 };

            memcpy(value, buffer, 4);
            return true;
    }

    return false;
}

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
bool registerReadRegister(const uint8_t type, const uint8_t address, uint8_t &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, uint16_t &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, uint32_t &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, int8_t &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, int16_t &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, int32_t &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, float &value) { return _registerReadRegister(type, address, &value); }
bool registerReadRegister(const uint8_t type, const uint8_t address, bool &value) { return _registerReadRegister(type, address, &value); }

// -----------------------------------------------------------------------------

/**
 * Write value to register from array representation
 */
bool registerWriteRegister(
    const uint8_t type,
    const uint8_t address,
    char * value,
    const bool propagate
) {
    uint8_t registers_size;

    if (type == REGISTER_TYPE_OUTPUT) {
        registers_size = REGISTER_MAX_OUTPUT_REGISTERS_SIZE;

    } else if (type == REGISTER_TYPE_ATTRIBUTE) {
        registers_size = REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE;

    } else {
        return false;
    }

    if (address >= registers_size) {
        return false;
    }

    uint8_t data_type = registerGetRegisterDataType(type, address);

    switch (data_type)
    {
        case REGISTER_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_write_value;

            uint8_write_value.bytes[0] = value[0];
            uint8_write_value.bytes[1] = 0;
            uint8_write_value.bytes[2] = 0;
            uint8_write_value.bytes[3] = 0;

            return registerWriteRegister(type, address, uint8_write_value.number, propagate);

        case REGISTER_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_write_value;

            uint16_write_value.bytes[0] = value[0];
            uint16_write_value.bytes[1] = value[1];
            uint16_write_value.bytes[2] = 0;
            uint16_write_value.bytes[3] = 0;

            return registerWriteRegister(type, address, uint16_write_value.number, propagate);

        case REGISTER_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_write_value;

            uint32_write_value.bytes[0] = value[0];
            uint32_write_value.bytes[1] = value[1];
            uint32_write_value.bytes[2] = value[2];
            uint32_write_value.bytes[3] = value[3];

            return registerWriteRegister(type, address, uint32_write_value.number, propagate);

        case REGISTER_DATA_TYPE_INT8:
            INT8_UNION_t int8_write_value;

            int8_write_value.bytes[0] = value[0];
            int8_write_value.bytes[1] = 0;
            int8_write_value.bytes[2] = 0;
            int8_write_value.bytes[3] = 0;

            return registerWriteRegister(type, address, int8_write_value.number, propagate);

        case REGISTER_DATA_TYPE_INT16:
            INT16_UNION_t int16_write_value;

            int16_write_value.bytes[0] = value[0];
            int16_write_value.bytes[1] = value[1];
            int16_write_value.bytes[2] = 0;
            int16_write_value.bytes[3] = 0;

            return registerWriteRegister(type, address, int16_write_value.number, propagate);

        case REGISTER_DATA_TYPE_INT32:
            INT32_UNION_t int32_write_value;

            int32_write_value.bytes[0] = value[0];
            int32_write_value.bytes[1] = value[1];
            int32_write_value.bytes[2] = value[2];
            int32_write_value.bytes[3] = value[3];

            return registerWriteRegister(type, address, int32_write_value.number, propagate);

        case REGISTER_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_write_value;

            float_write_value.bytes[0] = value[0];
            float_write_value.bytes[1] = value[1];
            float_write_value.bytes[2] = value[2];
            float_write_value.bytes[3] = value[3];

            return registerWriteRegister(type, address, float_write_value.number, propagate);

        case REGISTER_DATA_TYPE_BOOLEAN:
            BOOLEAN_UNION_t bool_write_value;

            bool_write_value.bytes[0] = value[0];
            bool_write_value.bytes[1] = value[1];
            bool_write_value.bytes[2] = 0;
            bool_write_value.bytes[3] = 0;

            return registerWriteRegister(type, address, bool_write_value.number, propagate);

        default:
            #if DEBUG_SUPPORT
                DPRINTLN(F("[REGISTER] Provided unknown data type for writing into register"));
            #endif
            break;
    }

    return false;
}

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint8_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint16_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint32_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int8_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int16_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int32_t value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const float value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const bool value, const bool propagate) { return _registerWriteRegister(type, address, &value, propagate); }

bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint8_t value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint16_t value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const uint32_t value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int8_t value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int16_t value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const int32_t value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const float value) { return _registerWriteRegister(type, address, &value, true); }
bool registerWriteRegister(const uint8_t type, const uint8_t address, const bool value) { return _registerWriteRegister(type, address, &value, true); }

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void registerSetup()
{
    // Write default value for packet max length
    _registerWriteRegister(
        REGISTER_TYPE_ATTRIBUTE,
        COMMUNICATION_ATTR_REGISTER_PML_ADDRESS,
        PJON_PACKET_MAX_LENGTH,
        false
    );

    #if REGISTER_MAX_INPUT_REGISTERS_SIZE
        for(int i = 0; i < REGISTER_MAX_INPUT_REGISTERS_SIZE; ++i) {
            if (register_module_input_registers[i].flash_address != INDEX_NONE) {
                _registerReadFromEeprom(REGISTER_TYPE_ATTRIBUTE, i, register_module_input_registers[i].flash_address);
                // _registerWriteRegister(REGISTER_TYPE_INPUT, i, EEPROM.get(register_module_input_registers[i].flash_address), false);
            }
        }
    #endif

    #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
        for(int i = 0; i < REGISTER_MAX_OUTPUT_REGISTERS_SIZE; ++i) {
            if (register_module_output_registers[i].flash_address != INDEX_NONE) {
                _registerReadFromEeprom(REGISTER_TYPE_ATTRIBUTE, i, register_module_output_registers[i].flash_address);
                // _registerWriteRegister(REGISTER_TYPE_OUTPUT, i, EEPROM.get(register_module_output_registers[i].flash_address), false);
            }
        }
    #endif

    #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
        for(int i = 0; i < REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE; ++i) {
            if (register_module_attribute_registers[i].flash_address != INDEX_NONE) {
                _registerReadFromEeprom(REGISTER_TYPE_ATTRIBUTE, i, register_module_attribute_registers[i].flash_address);
                // _registerWriteRegister(REGISTER_TYPE_ATTRIBUTE, i, EEPROM.get(register_module_attribute_registers[i].flash_address), false);
            }
        }
    #endif
}
