/*

COMMUNICATION MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#include "config/all.h"

#if COMMUNICATION_BUS_HARDWARE_SERIAL == 0
    #include <NeoSWSerial.h>
#endif

#include <Arduino.h>
#include <PJON.h>

#if COMMUNICATION_USE_CRC
    #include <uCRC16Lib.h>
#endif

PJON<ThroughSerialAsync> _communication_bus(PJON_NOT_ASSIGNED);

#if COMMUNICATION_BUS_HARDWARE_SERIAL == 0
    NeoSWSerial _communication_serial_bus(COMMUNICATION_BUS_TX_PIN, COMMUNICATION_BUS_RX_PIN);
#endif

uint32_t _communication_master_last_request = 0;

bool _communication_master_lost = false;
bool _communication_initial_state_to_master = false;

char _communication_output_buffer[PJON_PACKET_MAX_LENGTH];

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

void _communicationReplyWithException(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Packet identifier when exception was rised
    // 2 => Exception code
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_EXCEPTION;
    _communication_output_buffer[1] = (char) payload[0];
    _communication_output_buffer[2] = 0; // TODO: implement code

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 3) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive exception"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with exception"));
        }
    #else
        _communicationReplyToPacket(_communication_output_buffer, 3);
    #endif
}

#if REGISTER_MAX_OUTPUT_REGISTERS_SIZE || REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE

// -----------------------------------------------------------------------------
// WRITING MULTIPLE REGISTERS
// -----------------------------------------------------------------------------

void _communicationWriteMultipleRegisters(
    const word registerStartAddress,
    const word writeLength,
    uint8_t * payload,
    const uint8_t registerType
) {
/*
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word write_length = (word) payload[3] << 8 | (word) payload[4];
    uint8_t bytes_count = (uint8_t) payload[5];

    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested write to AO register at address: "));
        DPRINT(register_address);
        DPRINT(F(" and length: "));
        DPRINTLN(write_length);
    #endif

    if (
        // Write start address mus be between <0, buffer.size()>
        register_address < REGISTER_MAX_OUTPUT_REGISTERS_SIZE
        // Write end address have to be same or smaller as register size
        && (register_address + write_length) <= REGISTER_MAX_OUTPUT_REGISTERS_SIZE
    ) {
        uint8_t write_byte = 1;
        uint8_t data_byte;

        uint8_t write_address = register_address;

        while (
            write_address < (register_address + write_length)
            && write_address < REGISTER_MAX_OUTPUT_REGISTERS_SIZE
            && write_byte <= bytes_count
        ) {
            // TODO: Finish implementation
        }

        // 0 => Packet identifier
        // 1 => High byte of register address
        // 2 => Low byte of register address
        // 3 => High byte of write byte length
        // 4 => Low byte of write byte length
        _communication_output_buffer[0] = (char) packetId;
        _communication_output_buffer[1] = (char) (register_address >> 8);
        _communication_output_buffer[2] = (char) (register_address & 0xFF);
        _communication_output_buffer[3] = (char) (write_byte >> 8);
        _communication_output_buffer[4] = (char) (write_byte & 0xFF);

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, 5) == false) {
                // Device was not able to notify master about its address
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive multiple AO register write result"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with multiple AO register write result"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, 5);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined AO registers range"));
        #endif

        // TODO: Send exception
    }
*/
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting writing multiple registers
 *
 * 0    => Received packet identifier       => COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS_VALUES
 * 1    => Register type
 * 2    => High byte of register address
 * 3    => Low byte of register address
 * 4    => High byte of registers length
 * 5    => Low byte of registers length
 * 6-n  => Data to write into registers
 */
void _communicationWriteMultipleRegistersValuesHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register write address
    word register_start_address = (word) payload[2] << 8 | (word) payload[3];

    // Number of registers to write
    word write_length = (word) payload[4] << 8 | (word) payload[5];

    switch (register_type)
    {

        #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
            case REGISTER_TYPE_OUTPUT:
                _communicationWriteMultipleRegisters(register_start_address, write_length, payload, REGISTER_TYPE_OUTPUT);
                break;
        #endif

        #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case REGISTER_TYPE_ATTRIBUTE:
                _communicationWriteMultipleRegisters(register_start_address, write_length, payload, REGISTER_TYPE_ATTRIBUTE);
                break;
        #endif

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined registers type"));
            #endif

            _communicationReplyWithException(payload);

    }
}

// -----------------------------------------------------------------------------
// WRITING SINGLE REGISTER
// -----------------------------------------------------------------------------

void _communicationWriteSingleRegisterValue(
    uint8_t * payload,
    uint8_t * writeValue,
    const word registerAddress,
    const uint8_t registerType
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested writing value to single"));
        if (registerType == REGISTER_TYPE_OUTPUT) {
            DPRINT(F(" output "));
        } else if (registerType == REGISTER_TYPE_ATTRIBUTE) {
            DPRINT(F(" attribute "));
        }
        DPRINT(F("register at address: "));
        DPRINTLN(registerAddress);
    #endif

    #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
        // Check if attribute register is writtable
        if (
            registerType == REGISTER_TYPE_ATTRIBUTE
            && (
                registerAddress >= REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
                || register_module_attribute_registers[registerAddress].settable == false
            )
        ) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Attribute register is not writtable or out of range"));
            #endif

            _communicationReplyWithException(payload);

            return;
        }
    #endif

    if (registerWriteRegister(registerType, registerAddress, writeValue, false) == false) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Value could not be written into register"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    uint8_t stored_value[4] = { 0, 0, 0, 0 };

    if (registerReadRegister(registerType, registerAddress, stored_value) == false) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Written value could not be fetched from register"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Register type
    // 2    => High byte of register address
    // 3    => Low byte of register address
    // 4-7  => Written value
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER_VALUE;
    _communication_output_buffer[1] = (char) registerType;
    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = (char) stored_value[0];
    _communication_output_buffer[5] = (char) stored_value[1];
    _communication_output_buffer[6] = (char) stored_value[2];
    _communication_output_buffer[7] = (char) stored_value[3];

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 8) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive output register write result"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with output register write result"));
        }
    #else
        _communicationReplyToPacket(_communication_output_buffer, 8);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting writing single register
 *
 * 0        => Received packet identifier       => COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER_VALUE
 * 1        => Register type
 * 2        => High byte of register address
 * 3        => Low byte of register address
 * 4-5(7)   => Data to write into register
 */
void _communicationWriteSingleRegisterValueHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register write address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    switch (register_type)
    {

        #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
            case REGISTER_TYPE_OUTPUT:
            {
                uint8_t write_value[4] = { 0, 0, 0, 0 };

                write_value[0] = payload[4];
                write_value[1] = payload[5];
                write_value[2] = payload[6];
                write_value[3] = payload[7];

                _communicationWriteSingleRegisterValue(payload, write_value, register_address, REGISTER_TYPE_OUTPUT);
                break;
            }
        #endif

        #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case REGISTER_TYPE_ATTRIBUTE:
            {
                uint8_t write_value[4] = { 0, 0, 0, 0 };

                write_value[0] = payload[4];
                write_value[1] = payload[5];
                write_value[2] = payload[6];
                write_value[3] = payload[7];

                _communicationWriteSingleRegisterValue(payload, write_value, register_address, REGISTER_TYPE_ATTRIBUTE);
                break;
            }
        #endif

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined registers type"));
            #endif

            _communicationReplyWithException(payload);

    }
}
#endif

#if REGISTER_MAX_INPUT_REGISTERS_SIZE || REGISTER_MAX_OUTPUT_REGISTERS_SIZE || REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE

// -----------------------------------------------------------------------------
// READING MULTIPLE REGISTERS
// -----------------------------------------------------------------------------

void _communicationReadMultipleRegistersValues(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t readLength,
    const uint8_t registerType
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading values from multiple"));
        if (registerType == REGISTER_TYPE_INPUT) {
            DPRINT(F(" inputs "));
        } else if (registerType == REGISTER_TYPE_OUTPUT) {
            DPRINT(F(" outputs "));
        } else if (registerType == REGISTER_TYPE_ATTRIBUTE) {
            DPRINT(F(" attributes "));
        }
        DPRINT(F("registers from address: "));
        DPRINT(registerAddress);
        DPRINT(F(" and length: "));
        DPRINTLN(readLength);
    #endif

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => High byte of register address
    // 2    => Low byte of register address
    // 3    => Count of registers
    // 4-n  => Packet data
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS_VALUES;
    _communication_output_buffer[1] = (char) registerType;
    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = (char) 0; // Temporary value, will be updated after collecting all

    uint8_t byte_counter = 5;
    uint8_t byte_pointer = 5;
    uint8_t registers_counter = 0;

    uint8_t read_value[4] = { 0, 0, 0, 0 };

    for (uint8_t i = registerAddress; i < (registerAddress + readLength); i++) {
        #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            // Check if attribute register is queryable
            if (
                registerType == REGISTER_TYPE_ATTRIBUTE
                && (
                    i >= REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
                    || register_module_attribute_registers[i].queryable == false
                )
            ) {
                #if DEBUG_COMMUNICATION_SUPPORT
                    DPRINTLN(F("[COMMUNICATION][ERR] Attribute register is not readable or out of range"));
                #endif

                _communicationReplyWithException(payload);

                return;
            }
        #endif

        if (registerReadRegister(registerType, i, read_value) == false) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Value could not be fetched from register"));
            #endif

            return;
        }

        _communication_output_buffer[byte_pointer] = (char) read_value[0];
        byte_pointer++;
        _communication_output_buffer[byte_pointer] = (char) read_value[1];
        byte_pointer++;
        _communication_output_buffer[byte_pointer] = (char) read_value[2];
        byte_pointer++;
        _communication_output_buffer[byte_pointer] = (char) read_value[3];
        byte_pointer++;

        byte_counter = byte_counter + 4;

        registers_counter++;
    }

    // Update registers length
    _communication_output_buffer[4] = (char) registers_counter;

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, byte_counter) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive multiple registers reading"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with multiple registers content"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, byte_counter);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting reading multiple registers
 *
 * 0 => Received packet identifier      => COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS_VALUES
 * 1 => Register type
 * 2 => High byte of register address
 * 3 => Low byte of register address
 * 4 => High byte of registers length
 * 5 => Low byte of registers length
 */
void _communicationReadMultipleRegistersValuesHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register read start address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    // Number of registers to read
    word read_length = (word) payload[4] << 8 | (word) payload[5];

    switch (register_type)
    {

        #if REGISTER_MAX_INPUT_REGISTERS_SIZE
            case REGISTER_TYPE_INPUT:
                _communicationReadMultipleRegistersValues(payload, register_address, read_length, REGISTER_TYPE_INPUT);
                break;
        #endif

        #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
            case REGISTER_TYPE_OUTPUT:
                _communicationReadMultipleRegistersValues(payload, register_address, read_length, REGISTER_TYPE_OUTPUT);
                break;
        #endif

        #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case REGISTER_TYPE_ATTRIBUTE:
                _communicationReadMultipleRegistersValues(payload, register_address, read_length, REGISTER_TYPE_ATTRIBUTE);
                break;
        #endif

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined registers type"));
            #endif

            _communicationReplyWithException(payload);

    }
}

// -----------------------------------------------------------------------------
// READING SINGLE REGISTER
// -----------------------------------------------------------------------------

void _communicationReadSingleRegisterValue(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t registerType
) {    
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading value from single"));
        if (registerType == REGISTER_TYPE_INPUT) {
            DPRINT(F(" input "));
        } else if (registerType == REGISTER_TYPE_OUTPUT) {
            DPRINT(F(" output "));
        } else if (registerType == REGISTER_TYPE_ATTRIBUTE) {
            DPRINT(F(" attribute "));
        }
        DPRINT(F("register at address: "));
        DPRINTLN(registerAddress);
    #endif

    #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
        // Check if attribute register is queryable
        if (
            registerType == REGISTER_TYPE_ATTRIBUTE
            && (
                registerAddress >= REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
                || register_module_attribute_registers[registerAddress].queryable == false
            )
        ) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Attribute register is not readable or out of range"));
            #endif

            _communicationReplyWithException(payload);

            return;
        }
    #endif

    uint8_t read_value[4] = { 0, 0, 0, 0 };

    if (registerReadRegister(registerType, registerAddress, read_value) == false) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Value could not be fetched from register"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Register type
    // 2    => High byte of register address
    // 3    => Low byte of register address
    // 4-7  => Register value
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_SINGLE_REGISTER_VALUES;
    _communication_output_buffer[1] = (char) registerType;
    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = (char) read_value[0];
    _communication_output_buffer[5] = (char) read_value[1];
    _communication_output_buffer[6] = (char) read_value[2];
    _communication_output_buffer[7] = (char) read_value[3];

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 8) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive register reading"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with one register content"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 8);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting reading single register
 *
 * 0 => Received packet identifier      => COMMUNICATION_PACKET_READ_SINGLE_REGISTER_VALUES
 * 1 => Register type
 * 2 => High byte of register address
 * 3 => Low byte of register address
 */
void _communicationReadSingleRegisterValueHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register read address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    switch (register_type)
    {

        #if REGISTER_MAX_INPUT_REGISTERS_SIZE
            case REGISTER_TYPE_INPUT:
                _communicationReadSingleRegisterValue(payload, register_address, REGISTER_TYPE_INPUT);
                break;
        #endif

        #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
            case REGISTER_TYPE_OUTPUT:
                _communicationReadSingleRegisterValue(payload, register_address, REGISTER_TYPE_OUTPUT);
                break;
        #endif

        #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case REGISTER_TYPE_ATTRIBUTE:
                _communicationReadSingleRegisterValue(payload, register_address, REGISTER_TYPE_ATTRIBUTE);
                break;
        #endif

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined register type"));
            #endif

            _communicationReplyWithException(payload);

    }
}

#endif

// -----------------------------------------------------------------------------

#if REGISTER_MAX_INPUT_REGISTERS_SIZE || REGISTER_MAX_OUTPUT_REGISTERS_SIZE || REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE

void _communicationReadSingleRegisterStructure(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t registerType
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading structure from single"));
        if (registerType == REGISTER_TYPE_INPUT) {
            DPRINT(F(" input "));
        } else if (registerType == REGISTER_TYPE_OUTPUT) {
            DPRINT(F(" output "));
        } else if (registerType == REGISTER_TYPE_ATTRIBUTE) {
            DPRINT(F(" attribute "));
        }
        DPRINT(F("register at address: "));
        DPRINTLN(registerAddress);
    #endif

    uint8_t registers_size = 0;

    #if REGISTER_MAX_INPUT_REGISTERS_SIZE
        // Get total registers count
        if (registerType == REGISTER_TYPE_INPUT) {
            registers_size = REGISTER_MAX_INPUT_REGISTERS_SIZE;
        }
    #endif

    #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
        // Get total registers count
        if (registerType == REGISTER_TYPE_OUTPUT) {
            registers_size = REGISTER_MAX_OUTPUT_REGISTERS_SIZE;
        }
    #endif

    #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
        // Get total registers count
        if (registerType == REGISTER_TYPE_ATTRIBUTE) {
            registers_size = REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE;
        }
    #endif

    if (
        // Read start address mus be between <0, registers_size>
        registerAddress < registers_size
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0 => Packet identifier
        // 1 => Registers type
        // 2 => High byte of register address
        // 3 => Low byte of register address
        // 4 => Register data type
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_SINGLE_REGISTER_STRUCTURE;
        _communication_output_buffer[1] = (char) registerType;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
        _communication_output_buffer[4] = (char) registerGetRegisterDataType(registerType, registerAddress);

        uint8_t byte_pointer = 5;
        uint8_t byte_counter = 5;

        if (registerType == REGISTER_TYPE_ATTRIBUTE) {
            // 5    => High byte of register settable flag
            // 6    => Low byte of register settable flag
            // 7    => High byte of register queryable flag
            // 8    => Low byte of register queryable flag
            // 9    => Register name length
            // 10-n => Register name
            _communication_output_buffer[5] = (char) (register_module_attribute_registers[registerAddress].settable ? (REGISTER_BOOLEAN_VALUE_TRUE >> 8) : (REGISTER_BOOLEAN_VALUE_FALSE >> 8));
            _communication_output_buffer[6] = (char) (register_module_attribute_registers[registerAddress].settable ? (REGISTER_BOOLEAN_VALUE_TRUE & 0xFF) : (REGISTER_BOOLEAN_VALUE_FALSE & 0xFF));
            _communication_output_buffer[7] = (char) (register_module_attribute_registers[registerAddress].queryable ? (REGISTER_BOOLEAN_VALUE_TRUE >> 8) : (REGISTER_BOOLEAN_VALUE_FALSE >> 8));
            _communication_output_buffer[8] = (char) (register_module_attribute_registers[registerAddress].queryable ? (REGISTER_BOOLEAN_VALUE_TRUE & 0xFF) : (REGISTER_BOOLEAN_VALUE_FALSE & 0xFF));
            _communication_output_buffer[9] = (char) strlen(register_module_attribute_registers[registerAddress].name);

            byte_pointer = 10;
            byte_counter = 10;

            for (uint8_t char_pos = 0; char_pos < strlen(register_module_attribute_registers[registerAddress].name); char_pos++) {
                _communication_output_buffer[byte_pointer] = (char) register_module_attribute_registers[registerAddress].name[char_pos];

                byte_pointer++;
                byte_counter++;
            }
        }

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, byte_counter) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device register structure"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with register structure"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, byte_counter);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read structure for undefined registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting reading single register
 *
 * 0 => Received packet identifier      => COMMUNICATION_PACKET_READ_SINGLE_REGISTER_STRUCTURE
 * 1 => Register type
 * 2 => High byte of register address
 * 3 => Low byte of register address
 */
void _communicationReadSingleRegisterStructureHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register read address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    switch (register_type)
    {

        #if REGISTER_MAX_INPUT_REGISTERS_SIZE
            case REGISTER_TYPE_INPUT:
                _communicationReadSingleRegisterStructure(payload, register_address, REGISTER_TYPE_INPUT);
                break;
        #endif

        #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE
            case REGISTER_TYPE_OUTPUT:
                _communicationReadSingleRegisterStructure(payload, register_address, REGISTER_TYPE_OUTPUT);
                break;
        #endif

        #if REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case REGISTER_TYPE_ATTRIBUTE:
                _communicationReadSingleRegisterStructure(payload, register_address, REGISTER_TYPE_ATTRIBUTE);
                break;
        #endif

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined register type"));
            #endif

            _communicationReplyWithException(payload);

    }
}

#endif

// -----------------------------------------------------------------------------
// DEVICE DISCOVER
// -----------------------------------------------------------------------------

/**
 * Parse received payload - Provide device info
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_DISCOVER
 */
void _communicationDiscoverHandler(
    uint8_t * payload,
    uint16_t length
) {
    // Discovery reply is available only in pairing mode
    if (!firmwareIsPairing()) {
        return;
    }

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0      => Packet identifier
    // 1      => Device bus address
    // 2      => Device packet max length
    // 3      => Device status
    // 4      => Device SN length
    // 5-m    => Device parsed SN
    // m+1    => Device hardware version length
    // m+2-n  => Device hardware version
    // n+1    => Device hardware model length
    // n+2-o  => Device hardware model
    // o+1    => Device hardware manufacturer length
    // o+2-p  => Device hardware manufacturer
    // p+1    => Device firmware version length
    // p+2-q  => Device firmware version
    // q+1    => Device firmware manufacturer length
    // q+2-r  => Device firmware manufacturer
    // r+1    => Device inputs size
    // r+2    => Device outputs size
    // r+3    => Device attributes size
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_DISCOVER;
    _communication_output_buffer[1] = (char) _communication_bus.device_id();
    _communication_output_buffer[2] = (char) PJON_PACKET_MAX_LENGTH;
    _communication_output_buffer[3] = (char) firmwareGetDeviceState();

    // Add device SN info
    _communication_output_buffer[4] = (char) strlen((char *) DEVICE_SERIAL_NO);

    uint8_t byte_pointer = 5;
    uint8_t byte_counter = 5;

    for (uint8_t i = 0; i < strlen((char *) DEVICE_SERIAL_NO); i++) {
        _communication_output_buffer[byte_pointer] = ((char *) DEVICE_SERIAL_NO)[i];

        byte_pointer++;
        byte_counter++;
    }

    char * description_content;

    // Add device version info
    description_content = (char *) DEVICE_VERSION;

    _communication_output_buffer[byte_pointer] = (char) strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add device model info
    description_content = (char *) SYSTEM_DEVICE_NAME;

    _communication_output_buffer[byte_pointer] = (char) strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add device manufacturer info
    description_content = (char *) SYSTEM_DEVICE_MANUFACTURER;

    _communication_output_buffer[byte_pointer] = (char) strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add firmware version info
    description_content = (char *) FIRMWARE_VERSION;

    _communication_output_buffer[byte_pointer] = (char) strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add firmware manufacturer info
    description_content = (char *) FIRMWARE_MANUFACTURER;

    _communication_output_buffer[byte_pointer] = (char) strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add registers sizes info

    _communication_output_buffer[byte_pointer] = (char) REGISTER_MAX_INPUT_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) REGISTER_MAX_OUTPUT_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, byte_counter) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device search request"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with device search request"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, byte_counter);
    #endif
}

// -----------------------------------------------------------------------------
// MASTER PING PONG
// -----------------------------------------------------------------------------

/**
 * Parse received payload - Master ping
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PING
 */
void _communicationPingHandler(
    uint8_t * payload,
    uint16_t length
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PONG;

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 1) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device pong reply"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with pong"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 1);
    #endif
}

// -----------------------------------------------------------------------------
// COMMUNICATION
// -----------------------------------------------------------------------------

void _communicationReceiverHandler(
    uint8_t * payload,
    const uint16_t length,
    const PJON_Packet_Info &packetInfo
) {
    // Device is not in running or pairing mode, all received packets are ignored
    if (firmwareIsRunning() == false && firmwareIsPairing() == false) {
        return;
    }

    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINTLN();
        DPRINTLN(F("[COMMUNICATION] ==============="));
        DPRINTLN(F("[COMMUNICATION] Received packet"));
        DPRINTLN(F("[COMMUNICATION] ==============="));
    #endif

    if (length <= 0) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Received packet is without content"));
        #endif

        return;
    }

    #if DEBUG_COMMUNICATION_SUPPORT
        for (uint8_t i = 0; i < length; i++) {
            DPRINT(F("[COMMUNICATION] RAW DATA:"));
            DPRINT((uint8_t) payload[i]);
            DPRINTLN();
        }
    #endif

    // Protocol version must be on first byte
    uint8_t protocol_version = (uint8_t) payload[0];

    if (protocol_version != COMMUNICATION_PROTOCOL_VERSION) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION] Received packet for unsupported version "));
            DPRINT(protocol_version);
            DPRINTLN();
        #endif

        return;
    }

    #if COMMUNICATION_USE_CRC
        char crc_payload[length - 3];

        for (uint8_t i = 0; i < length - 3; i++) {
            crc_payload[i] = payload[i];
        }

        // CRC calculation
        uint16_t calculated_crc = uCRC16Lib::calculate(crc_payload, length - 3);

        word in_packet_crc = (word) payload[length - 2] << 8 | (word) payload[length - 3];

        if (calculated_crc != (uint16_t) in_packet_crc) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] CRC hash check failed "));
                DPRINT(calculated_crc);
                DPRINT(F(" vs "));
                DPRINT(in_packet_crc);
                DPRINTLN();
            #endif

            return;
        }

        uint16_t data_length = length - 4;
    #else
        uint16_t data_length = length - 2;
    #endif

    uint8_t data_payload[data_length];

    uint8_t byte_position = 0;

    #if COMMUNICATION_USE_CRC
        for (uint8_t i = 1; i < length - 3; i++){
            data_payload[byte_position] = payload[i];

            byte_position++;
        }
    #else
        for (uint8_t i = 1; i < length - 1; i++){
            data_payload[byte_position] = payload[i];

            byte_position++;
        }
    #endif

    #if DEBUG_COMMUNICATION_SUPPORT
        for (uint8_t i = 0; i < data_length; i++) {
            DPRINT(F("[COMMUNICATION] DATA:"));
            DPRINT((uint8_t) data_payload[i]);
            DPRINTLN();
        }
    #endif

    // Packed ID must be on second byte
    uint8_t packet_id = (uint8_t) data_payload[0];

    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Received packet: "));
        DPRINTLN(packet_id);
    #endif

    uint8_t sender_address = PJON_NOT_ASSIGNED;

    // Get sender address from header
    if (packetInfo.header & PJON_TX_INFO_BIT) {
        sender_address = packetInfo.sender_id;
    }
    
    // Only packets from master are accepted
    if (sender_address != COMMUNICATION_BUS_MASTER_ADDR) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Received packet from unknown master address: "));
            DPRINTLN(sender_address);
        #endif

        return;
    }

    // Reset master lost detection
    _communication_master_lost = false;
    _communication_master_last_request = millis();

    switch (packet_id)
    {

    /**
     * MISC
     */

        case COMMUNICATION_PACKET_PING:
            _communicationPingHandler(data_payload, data_length);
            break;

        case COMMUNICATION_PACKET_DISCOVER:
            _communicationDiscoverHandler(data_payload, data_length);
            break;

    /**
     * REGISTERS
     */

        #if REGISTER_MAX_INPUT_REGISTERS_SIZE || REGISTER_MAX_OUTPUT_REGISTERS_SIZE || REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case COMMUNICATION_PACKET_READ_SINGLE_REGISTER_VALUES:
                _communicationReadSingleRegisterValueHandler(data_payload);
                break;

            case COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS_VALUES:
                _communicationReadMultipleRegistersValuesHandler(data_payload);
                break;
        #endif

        #if REGISTER_MAX_OUTPUT_REGISTERS_SIZE || REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER_VALUE:
                _communicationWriteSingleRegisterValueHandler(data_payload);
                break;
    
            case COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS_VALUES:
                _communicationWriteMultipleRegistersValuesHandler(data_payload);
                break;
        #endif

        #if REGISTER_MAX_INPUT_REGISTERS_SIZE || REGISTER_MAX_OUTPUT_REGISTERS_SIZE || REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
            case COMMUNICATION_PACKET_READ_SINGLE_REGISTER_STRUCTURE:
                _communicationReadSingleRegisterStructureHandler(data_payload);
                break;
        #endif

    /**
     * OTHER
     */

        default:
            _communicationReplyWithException(data_payload);

    }

    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINTLN(F("[COMMUNICATION] ================================="));
        DPRINTLN(F("[COMMUNICATION] Handling received packet finished"));
        DPRINTLN(F("[COMMUNICATION] ================================="));
        DPRINTLN();
    #endif
}

// -----------------------------------------------------------------------------

void _communicationErrorHandler(
    const uint8_t code,
    const uint16_t data,
    void * customPointer
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        if (code == PJON_CONNECTION_LOST) {
            _communication_master_lost = true;

            DPRINTLN(F("[COMMUNICATION][ERR] Connection lost with master"));

        } else if (code == PJON_PACKETS_BUFFER_FULL) {
            DPRINTLN(F("[COMMUNICATION][ERR] Buffer is full"));

        } else if (code == PJON_CONTENT_TOO_LONG) {
            DPRINTLN(F("[COMMUNICATION][ERR] Content is long"));

        } else {
            DPRINTLN(F("[COMMUNICATION][ERR] Unknown error"));
        }
    #endif
}
// -----------------------------------------------------------------------------

uint16_t _communicationFinalizeAndSendPacket(
    const char * payload,
    const uint8_t length,
    const uint8_t address,
    const bool isReply
) {
    #if COMMUNICATION_USE_CRC
        uint16_t final_length = length + 4;
    #else
        uint16_t final_length = length + 2;
    #endif

    char final_payload[final_length];

    // Add protocol version
    final_payload[0] = COMMUNICATION_PROTOCOL_VERSION;

    uint8_t byte_position = 1;

    for (uint8_t i = 0; i < length; i++){
        final_payload[byte_position] = payload[i];

        byte_position++;
    }

    #if COMMUNICATION_USE_CRC
        uint16_t crc = uCRC16Lib::calculate(final_payload, byte_position); // CRC calculation hash

        // Add CRC to packet body
        final_payload[byte_position] = (char) (crc & 0xFF);
        byte_position++;
        final_payload[byte_position] = (char) (crc >> 8);
        byte_position++;
    #endif

    // Be sure to set the null terminator!!!
    final_payload[byte_position] = COMMUNICATION_PACKET_TERMINATOR;

    if (isReply) {
        return _communication_bus.reply(
            final_payload,      // Content
            final_length        // Content length
        );

    } else {
        return _communication_bus.send_packet(
            address,            // Recepient address
            final_payload,      // Content
            final_length        // Content length
        );
    }
}

// -----------------------------------------------------------------------------

uint16_t _communicationFinalizeAndSendPacket(
    const char * payload,
    const uint8_t length,
    const uint8_t address
) {
    return _communicationFinalizeAndSendPacket(payload, length, address, false);
}

// -----------------------------------------------------------------------------

uint16_t _communicationFinalizeAndSendPacket(
    const char * payload,
    const uint8_t length,
    const bool isReply
) {
    return _communicationFinalizeAndSendPacket(payload, length, 0, isReply);
}

// -----------------------------------------------------------------------------

bool _communicationSendPacket(
    const uint8_t address,
    const char * payload,
    const uint8_t length
) {
    uint16_t result = _communicationFinalizeAndSendPacket(payload, length, address);

    if (result != PJON_ACK) {
        if (result == PJON_BUSY ) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Sending packet to address: "));
                DPRINT(address);
                DPRINT(F(" failed, bus is busy, packet: "));
                DPRINTLN((uint8_t) payload[0]);
            #endif

        } else if (result == PJON_FAIL) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Sending packet to address: "));
                DPRINT(address);
                DPRINT(F(" failed, packet: "));
                DPRINTLN((uint8_t) payload[0]);
            #endif

        } else {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Sending packet to address: "));
                DPRINT(address);
                DPRINT(F(" failed, unknonw error, packet: "));
                DPRINTLN((uint8_t) payload[0]);
            #endif
        }

        return false;
    }

    if (address == PJON_BROADCAST) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION] Successfully sent broadcast packet "));
            DPRINTLN((uint8_t) payload[0]);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION] Successfully sent packet: "));
            DPRINT((uint8_t) payload[0]);
            DPRINT(F(" to address: "));
            DPRINTLN(address);
        #endif
    }

    return true;
}

// -----------------------------------------------------------------------------

bool _communicationReplyToPacket(
    char * payload,
    const uint8_t length
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Preparing reply packet: "));
        DPRINTLN((uint8_t) payload[0]);
    #endif

    uint16_t result =_communicationFinalizeAndSendPacket(payload, length, true);

    if (result == PJON_FAIL) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Sending replypacket failed"));
        #endif

        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------

bool _communicationBroadcastPacket(
    char * payload,
    const uint8_t length
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Preparing broadcast packet: "));
        DPRINTLN((uint8_t) payload[0]);
    #endif

    uint8_t address = PJON_BROADCAST;

    uint16_t result = _communicationFinalizeAndSendPacket(payload, length, address);

    if (result == PJON_FAIL) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Sending broadcast failed"));
        #endif

        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

bool communicationHasAssignedAddress()
{
    return _communication_bus.device_id() != PJON_NOT_ASSIGNED;
}

// -----------------------------------------------------------------------------

bool communicationIsMasterLost()
{
    return ((millis() - _communication_master_last_request) > COMMUNICATION_MASTER_PING_TIMEOUT || _communication_master_lost);
}

// -----------------------------------------------------------------------------
// REGISTERS
// -----------------------------------------------------------------------------

/**
 * Report value of register to master
 */
bool communicationReportRegister(
    const uint8_t registerType,
    const uint8_t registerAddress
) {
    if (firmwareIsRunning() == false && firmwareIsPairing() == false) {
        return false;
    }

    uint8_t register_value[4];
    
    if (registerReadRegister(registerType, registerAddress, register_value) == false) {
        return false;
    }

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Register type
    // 2    => High byte of register address
    // 3    => Low byte of register address
    // 4-7  => Register value
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER_VALUE;
    _communication_output_buffer[1] = (char) registerType;
    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = (char) register_value[0];
    _communication_output_buffer[5] = (char) register_value[1];
    _communication_output_buffer[6] = (char) register_value[2];
    _communication_output_buffer[7] = (char) register_value[3];

    if (_communicationSendPacket(COMMUNICATION_BUS_MASTER_ADDR, _communication_output_buffer, 8) == true) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully sent"));
        #endif

        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void communicationSetup()
{
    #if COMMUNICATION_BUS_HARDWARE_SERIAL
        #if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_STM32F2)
            _communication_bus.strategy.set_serial(&Serial1);
        #else
            _communication_bus.strategy.set_serial(&Serial);
        #endif
    #else
        _communication_serial_bus.begin(COMMUNICATION_SERIAL_BAUDRATE);
    
        _communication_bus.strategy.set_serial(&_communication_serial_bus);
    #endif

    _communication_bus.set_synchronous_acknowledge(false);
    _communication_bus.set_asynchronous_acknowledge(false);
    //_communication_bus.set_acknowledge(true);

    // Communication callbacks
    _communication_bus.set_receiver(_communicationReceiverHandler);
    _communication_bus.set_error(_communicationErrorHandler);

    //_communication_bus.include_sender_info(true);

    _communication_bus.begin();

    uint8_t device_address;

    registerReadRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_ADDR_ADDRESS, device_address);

    #if DEBUG_COMMUNICATION_SUPPORT
        if (device_address == PJON_NOT_ASSIGNED) {
            DPRINTLN(F("[COMMUNICATION] Unaddressed device"));

        } else {
            DPRINT(F("[COMMUNICATION] Stored device address: "));
            DPRINTLN(device_address);
        }
    #endif

    if (device_address != PJON_NOT_ASSIGNED && device_address > 0 && device_address < 250) {
        _communication_bus.set_id(device_address);
    }
}

// -----------------------------------------------------------------------------

void communicationLoop()
{
    if (!_communication_initial_state_to_master) {
        // Get actual timestamp
        uint32_t time = millis();

        // Little delay before gateway start
        if (time > COMMUNICATION_NOTIFY_STATE_DELAY) {
            communicationReportRegister(REGISTER_TYPE_ATTRIBUTE, COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS);

            _communication_initial_state_to_master = true;
        }
    }

    // -------------------------------------------------------------------------
    // Bus communication
    // -------------------------------------------------------------------------
    _communication_bus.update();
    _communication_bus.receive();
}
