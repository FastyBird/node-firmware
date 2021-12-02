/*

COMMUNICATION MODULE

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

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
bool _communication_pairing_enabled = false;
bool _communication_initial_state_to_master = false;

char _communication_output_buffer[PJON_PACKET_MAX_LENGTH];

#if COMMUNICATION_PUB_SUB_SUB_SUPPORT && COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS && COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
    communication_pub_sub_subscription_t _communication_pub_sub_subscriptions[COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS];

    communication_pub_sub_condition_t _communication_pub_sub_conditions[COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS * COMMUNICATION_PUB_SUB_MAX_CONDITIONS];
    communication_pub_sub_action_t _communication_pub_sub_actions[COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS * COMMUNICATION_PUB_SUB_MAX_ACTIONS];
#endif

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

#if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE || COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE

// -----------------------------------------------------------------------------
// REGISTERS HELPERS
// -----------------------------------------------------------------------------

/**
 * Read value from register & transform it to 4 bytes
 */
void _communicationReadValueForTransfer(
    const uint8_t registerType,
    const uint8_t dataType,
    const uint8_t address,
    char * value
) {
    switch (dataType)
    {
        case COMMUNICATION_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_read_value;

            communicationReadRegister(registerType, address, uint8_read_value.number);

            memcpy(value, uint8_read_value.bytes, 1);
            break;

        case COMMUNICATION_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_read_value;

            communicationReadRegister(registerType, address, uint16_read_value.number);

            memcpy(value, uint16_read_value.bytes, 2);
            break;

        case COMMUNICATION_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_read_value;

            communicationReadRegister(registerType, address, uint32_read_value.number);

            memcpy(value, uint32_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_INT8:
            INT8_UNION_t int8_read_value;

            communicationReadRegister(registerType, address, int8_read_value.number);

            memcpy(value, int8_read_value.bytes, 1);
            break;

        case COMMUNICATION_DATA_TYPE_INT16:
            INT16_UNION_t int16_read_value;

            communicationReadRegister(registerType, address, int16_read_value.number);

            memcpy(value, int16_read_value.bytes, 2);
            break;

        case COMMUNICATION_DATA_TYPE_INT32:
            INT32_UNION_t int32_read_value;

            communicationReadRegister(registerType, address, int32_read_value.number);

            memcpy(value, int32_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_read_value;

            communicationReadRegister(registerType, address, float_read_value.number);

            memcpy(value, float_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_BOOLEAN:
            BOOLEAN_UNION_t bool_read_value;

            communicationReadRegister(registerType, address, bool_read_value.number);

            memcpy(value, bool_read_value.bytes, 2);
            break;
    
        default:
            char buffer[4] = { 0, 0, 0, 0 };

            memcpy(value, buffer, 4);
            break;
    }
}

// -----------------------------------------------------------------------------

/**
 * Transform received 4 bytes value into register value
 */
void _communicationWriteValueFromTransfer(
    const uint8_t registerType,
    const uint8_t dataType,
    const uint8_t address,
    char * value
) {
    switch (dataType)
    {
        case COMMUNICATION_DATA_TYPE_UINT8:
            uint8_t uint8_stored_value;
            UINT8_UNION_t uint8_write_value;

            uint8_write_value.bytes[0] = value[0];
            uint8_write_value.bytes[1] = 0;
            uint8_write_value.bytes[2] = 0;
            uint8_write_value.bytes[3] = 0;

            communicationReadRegister(registerType, address, uint8_stored_value);

            if (uint8_stored_value != uint8_write_value.number) {
                communicationWriteRegister(registerType, address, uint8_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_UINT16:
            uint16_t uint16_stored_value;
            UINT16_UNION_t uint16_write_value;

            uint16_write_value.bytes[0] = value[0];
            uint16_write_value.bytes[1] = value[1];
            uint16_write_value.bytes[2] = 0;
            uint16_write_value.bytes[3] = 0;

            communicationReadRegister(registerType, address, uint16_stored_value);

            if (uint16_stored_value != uint16_write_value.number) {
                communicationWriteRegister(registerType, address, uint16_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_UINT32:
            uint32_t uint32_stored_value;
            UINT32_UNION_t uint32_write_value;

            uint32_write_value.bytes[0] = value[0];
            uint32_write_value.bytes[1] = value[1];
            uint32_write_value.bytes[2] = value[2];
            uint32_write_value.bytes[3] = value[3];

            communicationReadRegister(registerType, address, uint32_stored_value);

            if (uint32_stored_value != uint32_write_value.number) {
                communicationWriteRegister(registerType, address, uint32_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_INT8:
            int8_t int8_stored_value;
            INT8_UNION_t int8_write_value;

            int8_write_value.bytes[0] = value[0];
            int8_write_value.bytes[1] = 0;
            int8_write_value.bytes[2] = 0;
            int8_write_value.bytes[3] = 0;

            communicationReadRegister(registerType, address, int8_stored_value);

            if (int8_stored_value != int8_write_value.number) {
                communicationWriteRegister(registerType, address, int8_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_INT16:
            int16_t int16_stored_value;
            INT16_UNION_t int16_write_value;

            int16_write_value.bytes[0] = value[0];
            int16_write_value.bytes[1] = value[1];
            int16_write_value.bytes[2] = 0;
            int16_write_value.bytes[3] = 0;

            communicationReadRegister(registerType, address, int16_stored_value);

            if (int16_stored_value != int16_write_value.number) {
                communicationWriteRegister(registerType, address, int16_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_INT32:
            int32_t int32_stored_value;
            INT32_UNION_t int32_write_value;

            int32_write_value.bytes[0] = value[0];
            int32_write_value.bytes[1] = value[1];
            int32_write_value.bytes[2] = value[2];
            int32_write_value.bytes[3] = value[3];

            communicationReadRegister(registerType, address, int32_stored_value);

            if (int32_stored_value != int32_write_value.number) {
                communicationWriteRegister(registerType, address, int32_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_FLOAT32:
            float float_stored_value;
            FLOAT32_UNION_t float_write_value;

            float_write_value.bytes[0] = value[0];
            float_write_value.bytes[1] = value[1];
            float_write_value.bytes[2] = value[2];
            float_write_value.bytes[3] = value[3];

            communicationReadRegister(registerType, address, float_stored_value);

            if (float_stored_value != float_write_value.number) {
                communicationWriteRegister(registerType, address, float_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_BOOLEAN:
            uint16_t bool_stored_value;
            BOOLEAN_UNION_t bool_write_value;

            bool_write_value.bytes[0] = value[0];
            bool_write_value.bytes[1] = value[1];
            bool_write_value.bytes[2] = 0;
            bool_write_value.bytes[3] = 0;

            communicationReadRegister(registerType, address, bool_stored_value);

            if (bool_stored_value != bool_write_value.number) {
                communicationWriteRegister(registerType, address, bool_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into: "));
                DPRINT(registerType);
                DPRINTLN(F(" register is same as stored. Write skipped"));
            #endif
            }
            break;

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Provided unknown data type for writing into register"));
            #endif
            break;
    }
}

#endif

#if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE

// -----------------------------------------------------------------------------
// WRITING MULTIPLE REGISTERS
// -----------------------------------------------------------------------------

void _communicationWriteMultipleRegisters(
    const word registerStartAddress,
    const word writeLength,
    uint8_t * payload
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
        register_address < COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
        // Write end address have to be same or smaller as register size
        && (register_address + write_length) <= COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
    ) {
        uint8_t write_byte = 1;
        uint8_t data_byte;

        uint8_t write_address = register_address;

        while (
            write_address < (register_address + write_length)
            && write_address < COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
            && write_byte <= bytes_count
        ) {
            // TODO: Finish implementation
        }

        // 0 => Packet identifier
        // 1 => High byte of register address
        // 2 => Low byte of register address
        // 3 => High byte of write byte length
        // 4 => Low byte of write byte length
        _communication_output_buffer[0] = packetId;
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
 * 0    => Received packet identifier       => COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS
 * 1    => Register type
 * 2    => High byte of register address
 * 3    => Low byte of register address
 * 4    => High byte of registers length
 * 5    => Low byte of registers length
 * 6-n  => Data to write into registers
 */
void _communicationWriteMultipleRegistersHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register write address
    word register_start_address = (word) payload[2] << 8 | (word) payload[3];

    // Number of registers to write
    word write_length = (word) payload[4] << 8 | (word) payload[5];

    switch (register_type)
    {

        case COMMUNICATION_REGISTER_TYPE_INPUT:
            _communicationWriteMultipleRegisters(register_start_address, write_length, payload);
            break;

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

void _communicationWriteSingleRegister(
    uint8_t * payload,
    char * writeValue,
    const word registerAddress,
    const uint8_t registerType
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested writing single value to output register at address: "));
        DPRINTLN(registerAddress);
    #endif

    if (
        // Write address must be between <0, registers_size>
        registerAddress < COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
    ) {
        _communicationWriteValueFromTransfer(
            COMMUNICATION_REGISTER_TYPE_OUTPUT,
            communication_module_output_registers[registerAddress].data_type,
            registerAddress,
            writeValue
        );

        char stored_value[4] = { 0, 0, 0, 0 };

        _communicationReadValueForTransfer(
            COMMUNICATION_REGISTER_TYPE_OUTPUT,
            communication_module_output_registers[registerAddress].data_type,
            registerAddress,
            stored_value
        );

        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Register type
        // 2    => High byte of register address
        // 3    => Low byte of register address
        // 4-7  => Written value
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER;
        _communication_output_buffer[1] = (char) COMMUNICATION_REGISTER_TYPE_OUTPUT;
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

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined output register address"));
        #endif

        _communicationReplyWithException(payload);
    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting writing single register
    *
    * 0        => Received packet identifier       => COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER
    * 1        => Register type
    * 2        => High byte of register address
    * 3        => Low byte of register address
    * 4-5(7)   => Data to write into register
    */
void _communicationWriteSingleRegisterHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register write address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    switch (register_type)
    {

        case COMMUNICATION_REGISTER_TYPE_OUTPUT:
        {
            char write_value[4] = { 0, 0, 0, 0 };

            write_value[0] = payload[4];
            write_value[1] = payload[5];
            write_value[2] = payload[6];
            write_value[3] = payload[7];

            _communicationWriteSingleRegister(payload, write_value, register_address, register_type);
            break;
        }

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined registers type"));
            #endif

            _communicationReplyWithException(payload);

    }
}
#endif

#if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE || COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE

// -----------------------------------------------------------------------------
// READING MULTIPLE REGISTERS
// -----------------------------------------------------------------------------

void _communicationReplyMultipleRegister(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t readLength,
    const uint8_t registerType
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from multiple"));
        if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
            DPRINT(F(" inputs "));
        } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
            DPRINT(F(" outputs "));
        } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
            DPRINT(F(" attributes "));
        } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
            DPRINT(F(" settings "));
        }
        DPRINT(F("registers at address: "));
        DPRINT(registerAddress);
        DPRINT(F(" and length: "));
        DPRINTLN(readLength);
    #endif

    uint8_t registers_size = 0;

    if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
        registers_size = COMMUNICATION_MAX_INPUT_REGISTERS_SIZE;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
        registers_size = COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
        registers_size = COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
        registers_size = COMMUNICATION_MAX_SETTING_REGISTERS_SIZE;
    }

    if (
        // Read start address mus be between <0, buffer.size()>
        registerAddress < registers_size
        // Read length have to be same or smaller as registers size
        && (registerAddress + readLength) <= registers_size
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of registers
        // 4-n  => Packet data
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS;
        _communication_output_buffer[1] = (char) registerType;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
        _communication_output_buffer[4] = (char) 0; // Temporary value, will be updated after collecting all

        uint8_t byte_counter = 5;
        uint8_t byte_pointer = 5;
        uint8_t registers_counter = 0;

        char read_value[4] = { 0, 0, 0, 0 };

        for (uint8_t i = registerAddress; i < (registerAddress + readLength) && i < registers_size; i++) {
            #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
                if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
                    _communicationReadValueForTransfer(registerType, communication_module_input_registers[i].data_type, i, read_value);
                }
            #endif

            #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
                if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
                    _communicationReadValueForTransfer(registerType, communication_module_output_registers[i].data_type, i, read_value);
                }
            #endif

            #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
                if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
                    _communicationReadValueForTransfer(registerType, communication_module_attribute_registers[i].data_type, i, read_value);
                }
            #endif

            #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
                if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
                    _communicationReadValueForTransfer(registerType, communication_module_setting_registers[i].data_type, i, read_value);
                }
            #endif

            _communication_output_buffer[byte_pointer] = read_value[0];
            byte_pointer++;
            _communication_output_buffer[byte_pointer] = read_value[1];
            byte_pointer++;
            _communication_output_buffer[byte_pointer] = read_value[2];
            byte_pointer++;
            _communication_output_buffer[byte_pointer] = read_value[3];
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

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting reading multiple registers
 *
 * 0 => Received packet identifier      => COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS
 * 1 => Register type
 * 2 => High byte of register address
 * 3 => Low byte of register address
 * 4 => High byte of registers length
 * 5 => Low byte of registers length
 */
void _communicationReadMultipleRegistersHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register read start address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    // Number of registers to read
    word read_length = (word) payload[4] << 8 | (word) payload[5];

    switch (register_type)
    {

        #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_INPUT:
                _communicationReplyMultipleRegister(payload, register_address, read_length, COMMUNICATION_REGISTER_TYPE_INPUT);
                break;
        #endif

        #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_OUTPUT:
                _communicationReplyMultipleRegister(payload, register_address, read_length, COMMUNICATION_REGISTER_TYPE_OUTPUT);
                break;
        #endif

        #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_ATTRIBUTE:
                _communicationReplyMultipleRegister(payload, register_address, read_length, COMMUNICATION_REGISTER_TYPE_ATTRIBUTE);
                break;
        #endif

        #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_SETTING:
                _communicationReplyMultipleRegister(payload, register_address, read_length, COMMUNICATION_REGISTER_TYPE_SETTING);
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

void _communicationReplySingleRegister(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t registerType
) {    
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from single"));
        if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
            DPRINT(F(" inputs "));
        } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
            DPRINT(F(" outputs "));
        } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
            DPRINT(F(" attributes "));
        } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
            DPRINT(F(" settings "));
        }
        DPRINT(F("register at address: "));
        DPRINTLN(registerAddress);
    #endif

    uint8_t registers_size = 0;

    if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
        registers_size = COMMUNICATION_MAX_INPUT_REGISTERS_SIZE;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
        registers_size = COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
        registers_size = COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
        registers_size = COMMUNICATION_MAX_SETTING_REGISTERS_SIZE;
    }

    if (
        // Read start address mus be between <0, registers_size>
        registerAddress < registers_size
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Register type
        // 2    => High byte of register address
        // 3    => Low byte of register address
        // 4-7  => Register value
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_SINGLE_REGISTER;
        _communication_output_buffer[1] = (char) registerType;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);

        char read_value[4] = { 0, 0, 0, 0 };

        #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
            if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
                _communicationReadValueForTransfer(
                    registerType,
                    communication_module_input_registers[registerAddress].data_type,
                    registerAddress,
                    read_value
                );
            }
        #endif

        #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
            if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
                _communicationReadValueForTransfer(
                    registerType,
                    communication_module_output_registers[registerAddress].data_type,
                    registerAddress,
                    read_value
                );
            }
        #endif

        #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
            if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
                _communicationReadValueForTransfer(
                    registerType,
                    communication_module_attribute_registers[registerAddress].data_type,
                    registerAddress,
                    read_value
                );
            }
        #endif

        #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
            if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
                _communicationReadValueForTransfer(
                    registerType,
                    communication_module_setting_registers[registerAddress].data_type,
                    registerAddress,
                    read_value
                );
            }
        #endif

        _communication_output_buffer[4] = read_value[0];
        _communication_output_buffer[5] = read_value[1];
        _communication_output_buffer[6] = read_value[2];
        _communication_output_buffer[7] = read_value[3];

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

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting reading single register
 *
 * 0 => Received packet identifier      => COMMUNICATION_PACKET_READ_SINGLE_REGISTER
 * 1 => Register type
 * 2 => High byte of register address
 * 3 => Low byte of register address
 */
void _communicationReadSingleRegisterHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register read address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    switch (register_type)
    {

        #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_INPUT:
                _communicationReplySingleRegister(payload, register_address, COMMUNICATION_REGISTER_TYPE_INPUT);
                break;
        #endif

        #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_OUTPUT:
                _communicationReplySingleRegister(payload, register_address, COMMUNICATION_REGISTER_TYPE_OUTPUT);
                break;
        #endif

        #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_ATTRIBUTE:
                _communicationReplySingleRegister(payload, register_address, COMMUNICATION_REGISTER_TYPE_ATTRIBUTE);
                break;
        #endif

        #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
            case COMMUNICATION_REGISTER_TYPE_SETTING:
                _communicationReplySingleRegister(payload, register_address, COMMUNICATION_REGISTER_TYPE_SETTING);
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
// DEVICE PAIRING MODE
// -----------------------------------------------------------------------------

/**
 * Parse received payload - Provide device info
 * 
 * 0    => Received packet identifier       => COMMUNICATION_PACKET_DISCOVER
 * 1    => Pairing command                  => COMMUNICATION_DISCOVER_RESPONSE_SEARCH
 */
void _communicationDiscoverSearchDevice(
    uint8_t * payload,
    uint16_t length
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0      => Packet identifier
    // 1      => Command identifier
    // 2      => Device current bus address
    // 3      => Device max supported packet size
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
    // r+4    => Device settings size
    // r+5    => High byte of PubSub - PUB support
    // r+6    => Low byte of PubSub - PUB support
    // r+7    => High byte of PubSub - SUB support
    // r+8    => Low byte of PubSub - SUB support
    // r+9    => Maximum supported subscriptions count
    // r+10   => Maximum supported conditions count per subscriptions
    // r+11   => Maximum supported actions count per subscriptions
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_DISCOVER;
    _communication_output_buffer[1] = (char) COMMUNICATION_DISCOVER_RESPONSE_SEARCH;
    _communication_output_buffer[2] = _communication_bus.device_id();
    _communication_output_buffer[3] = (char) PJON_PACKET_MAX_LENGTH;

    // Add device SN info
    _communication_output_buffer[4] = strlen((char *) DEVICE_SERIAL_NO);

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

    _communication_output_buffer[byte_pointer] = strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add device model info
    description_content = (char *) DEVICE_NAME;

    _communication_output_buffer[byte_pointer] = strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add device manufacturer info
    description_content = (char *) DEVICE_MANUFACTURER;

    _communication_output_buffer[byte_pointer] = strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add firmware version info
    description_content = (char *) FIRMWARE_VERSION;

    _communication_output_buffer[byte_pointer] = strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    // Add firmware manufacturer info
    description_content = (char *) FIRMWARE_MANUFACTURER;

    _communication_output_buffer[byte_pointer] = strlen(description_content);

    byte_pointer++;
    byte_counter++;

    for (uint8_t i = 0; i < strlen(description_content); i++) {
        _communication_output_buffer[byte_pointer] = description_content[i];

        byte_pointer++;
        byte_counter++;
    }

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_MAX_INPUT_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_MAX_SETTING_REGISTERS_SIZE;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) ((COMMUNICATION_PUB_SUB_PUB_SUPPORT ? COMMUNICATION_BOOLEAN_VALUE_TRUE : COMMUNICATION_BOOLEAN_VALUE_FALSE) >> 8);

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) ((COMMUNICATION_PUB_SUB_PUB_SUPPORT ? COMMUNICATION_BOOLEAN_VALUE_TRUE : COMMUNICATION_BOOLEAN_VALUE_FALSE) & 0xFF);

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) ((COMMUNICATION_PUB_SUB_SUB_SUPPORT ? COMMUNICATION_BOOLEAN_VALUE_TRUE : COMMUNICATION_BOOLEAN_VALUE_FALSE) >> 8);

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) ((COMMUNICATION_PUB_SUB_SUB_SUPPORT ? COMMUNICATION_BOOLEAN_VALUE_TRUE : COMMUNICATION_BOOLEAN_VALUE_FALSE) & 0xFF);

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_PUB_SUB_SUB_SUPPORT ? COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS : 0;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_PUB_SUB_SUB_SUPPORT ? COMMUNICATION_PUB_SUB_MAX_CONDITIONS : 0;

    byte_pointer++;
    byte_counter++;

    _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_PUB_SUB_SUB_SUPPORT ? COMMUNICATION_PUB_SUB_MAX_ACTIONS : 0;

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

/**
 * Parse received payload - Confirm write assigned address
 * 
 * 0    => Received packet identifier       => COMMUNICATION_PACKET_DISCOVER
 * 1    => Pairing command                  => COMMUNICATION_DISCOVER_CMD_WRITE_ADDRESS
 * 2    => Master assigned address          => 1-254
 * 3    => Device SN length                 => 1-255
 * 4-n  => Device SN                        => (a,b,c,...)
 */
void _communicationDiscoverDeviceWriteAddress(
    uint8_t * payload,
    uint16_t length
) {
    // Extract address assigned by master
    const uint8_t address = (uint8_t) payload[2];

    uint8_t device_sn_length = (uint8_t) payload[3];

    // TODO: Validate packet lenght

    // Initialize serial number buffer
    char device_sn[device_sn_length];
    memset(device_sn, 0, device_sn_length);

    // Extract serial number from payload
    for (uint8_t i = 0; i < device_sn_length; i++) {
        device_sn[i] = (char) payload[i + 4];
        device_sn[i + 1] = 0x00; // Add a NULL after each character
    }

    // Check if received packet is for this device
    if (strcmp(DEVICE_SERIAL_NO, device_sn) != 0) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION][INFO] Packet is for other device: \""));
            DPRINT(device_sn);
            DPRINTLN(F("\""));
            DPRINT(F("[COMMUNICATION][INFO] Device SN is: \""));
            DPRINT((char *) DEVICE_SERIAL_NO);
            DPRINTLN(F("\""));
        #endif

        return;
    }

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Command response content type
    // 2-n  => Device SN
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_DISCOVER;
    _communication_output_buffer[1] = (char) COMMUNICATION_DISCOVER_RESPONSE_WRITE_ADDRESS;
    _communication_output_buffer[2] = strlen((char *) DEVICE_SERIAL_NO);

    uint8_t byte_pointer = 3;
    uint8_t byte_counter = 3;

    for (uint8_t i = 0; i < strlen((char *) DEVICE_SERIAL_NO); i++) {
        _communication_output_buffer[byte_pointer] = ((char *) DEVICE_SERIAL_NO)[i];

        byte_pointer++;
        byte_counter++;
    }

    // Reply to master
    if (_communicationReplyToPacket(_communication_output_buffer, byte_counter) == true) {
        // Set device address to address received from master
        _communication_bus.set_id(address);

        #if !COMMUNICATION_DISABLE_ADDRESS_STORING
            EEPROM.update(FLASH_ADDRESS_DEVICE_ADDRESS, address);
        #endif

        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN();
            DPRINTLN(F("[COMMUNICATION] ==========================="));
            DPRINT(F("[COMMUNICATION] Device address was set to: "));
            DPRINTLN(address);
            DPRINTLN(F("[COMMUNICATION] ==========================="));
            DPRINTLN();

            DPRINTLN(F("[COMMUNICATION] Replied to master with accepted address"));
        #endif

    #if DEBUG_COMMUNICATION_SUPPORT
    } else {
        DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device address confirmation"));
    #endif
    }
}

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE || COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE

/**
 * Parse received payload - Provide register structure
 * 
 * 0 => Received packet identifier       => COMMUNICATION_PACKET_DISCOVER
 * 1 => Pairing command                  => COMMUNICATION_DISCOVER_CMD_PROVIDE_REGISTER_STRUCTURE
 * 2 => Registers type                   => COMMUNICATION_REGISTER_TYPE_INPUT | COMMUNICATION_REGISTER_TYPE_OUTPUT | COMMUNICATION_REGISTER_TYPE_ATTRIBUTE | COMMUNICATION_REGISTER_TYPE_SETTING
 * 3 => High byte of register address
 * 4 => Low byte of register address
 */
void _communicationDiscoverDeviceProvideRegisterStructure(
    uint8_t * payload,
    uint16_t length
) {
    uint8_t register_type = (uint8_t) payload[2];

    if (
        register_type != COMMUNICATION_REGISTER_TYPE_INPUT
        && register_type != COMMUNICATION_REGISTER_TYPE_OUTPUT
        && register_type != COMMUNICATION_REGISTER_TYPE_ATTRIBUTE
        && register_type != COMMUNICATION_REGISTER_TYPE_SETTING
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Requested register type structure is not supported"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    // Register read start address
    word register_address = (word) payload[3] << 8 | (word) payload[4];

    uint8_t registers_size = 0;

    #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_INPUT) {
            registers_size = COMMUNICATION_MAX_INPUT_REGISTERS_SIZE;
        }
    #endif

    #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
            registers_size = COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE;
        }
    #endif

    #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
            registers_size = COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE;
        }
    #endif

    #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_SETTING) {
            registers_size = COMMUNICATION_MAX_SETTING_REGISTERS_SIZE;
        }
    #endif

    if (
        // Read start address mus be between <0, registers_size>
        register_address < registers_size
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0 => Packet identifier
        // 1 => Command response content type
        // 2 => Registers type
        // 3 => High byte of register address
        // 4 => Low byte of register address
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_DISCOVER;
        _communication_output_buffer[1] = (char) COMMUNICATION_DISCOVER_RESPONSE_PROVIDE_REGISTER_STRUCTURE;
        _communication_output_buffer[2] = (char) register_type;
        _communication_output_buffer[3] = (char) (register_address >> 8);
        _communication_output_buffer[4] = (char) (register_address & 0xFF);

        uint8_t byte_pointer = 5;
        uint8_t byte_counter = 5;

        if (register_type == COMMUNICATION_REGISTER_TYPE_INPUT) {
            // 5 => Register data type
            _communication_output_buffer[5] = communication_module_input_registers[register_address].data_type;

            byte_pointer++;
            byte_counter++;
            
        } else if (register_type == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
            // 5 => Register data type
            _communication_output_buffer[5] = communication_module_output_registers[register_address].data_type;

            byte_pointer++;
            byte_counter++;

        } else if (register_type == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
            // 5    => Register data type
            // 6    => High byte of register settable flag
            // 7    => Low byte of register settable flag
            // 8    => High byte of register queryable flag
            // 9    => Low byte of register queryable flag
            // 10   => Register name length
            // 1+-n => Register name
            _communication_output_buffer[5] = (char) communication_module_attribute_registers[register_address].data_type;
            _communication_output_buffer[6] = (char) (communication_module_attribute_registers[register_address].settable ? (COMMUNICATION_BOOLEAN_VALUE_TRUE >> 8) : (COMMUNICATION_BOOLEAN_VALUE_FALSE >> 8));
            _communication_output_buffer[7] = (char) (communication_module_attribute_registers[register_address].settable ? (COMMUNICATION_BOOLEAN_VALUE_TRUE & 0xFF) : (COMMUNICATION_BOOLEAN_VALUE_FALSE & 0xFF));
            _communication_output_buffer[8] = (char) (communication_module_attribute_registers[register_address].queryable ? (COMMUNICATION_BOOLEAN_VALUE_TRUE >> 8) : (COMMUNICATION_BOOLEAN_VALUE_FALSE >> 8));
            _communication_output_buffer[9] = (char) (communication_module_attribute_registers[register_address].queryable ? (COMMUNICATION_BOOLEAN_VALUE_TRUE & 0xFF) : (COMMUNICATION_BOOLEAN_VALUE_FALSE & 0xFF));
            _communication_output_buffer[10] = strlen(communication_module_attribute_registers[register_address].name);

            byte_pointer = 11;
            byte_counter = 11;

            for (uint8_t char_pos = 0; char_pos < strlen(communication_module_attribute_registers[register_address].name); char_pos++) {
                _communication_output_buffer[byte_pointer] = (char) communication_module_attribute_registers[register_address].name[char_pos];

                byte_pointer++;
                byte_counter++;
            }

        } else if (register_type == COMMUNICATION_REGISTER_TYPE_SETTING) {
            // 5   => Register data type
            // 6   => Register name length
            // 7-n => Register name
            _communication_output_buffer[5] = (char) communication_module_setting_registers[register_address].data_type;
            _communication_output_buffer[6] = strlen(communication_module_setting_registers[register_address].name);

            byte_pointer = 7;
            byte_counter = 7;

            for (uint8_t char_pos = 0; char_pos < strlen(communication_module_setting_registers[register_address].name); char_pos++) {
                _communication_output_buffer[byte_pointer] = (char) communication_module_setting_registers[register_address].name[char_pos];

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
#endif

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Pairing is finished
 * 
 * 0 => Received packet identifier   => COMMUNICATION_PACKET_DISCOVER
 * 1 => Pairing command
 */
void _communicationDiscoverDeviceFinished(
    uint8_t * payload,
    uint16_t length
) {
    communicationDisablePairingMode();

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Command response content type
    // 2 => Device actual state
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_DISCOVER;
    _communication_output_buffer[1] = (char) COMMUNICATION_DISCOVER_RESPONSE_PAIRING_FINISHED;
    _communication_output_buffer[2] = (char) _communicationGetDeviceState();

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 3) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive pairing finished reply"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with pairing finished"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 3);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Discover device
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_DISCOVER
 * 1 => Pairing command
 */
void _communicationDiscoverDeviceHandler(
    uint8_t * payload,
    uint16_t length
) {
    if (_communication_pairing_enabled == false) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][INFO] Device is not in pairing mode"));
        #endif

        return;
    }

    // Extract pairing command
    const uint8_t pairing_command = (uint8_t) payload[1];

    switch (pairing_command)
    {

        case COMMUNICATION_DISCOVER_CMD_SEARCH:
            _communicationDiscoverSearchDevice(payload, length);
            break;

        case COMMUNICATION_DISCOVER_CMD_WRITE_ADDRESS:
            _communicationDiscoverDeviceWriteAddress(payload, length);
            break;

        #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE || COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
            case COMMUNICATION_DISCOVER_CMD_PROVIDE_REGISTER_STRUCTURE:
                _communicationDiscoverDeviceProvideRegisterStructure(payload, length);
                break;
        #endif

        case COMMUNICATION_DISCOVER_CMD_PAIRING_FINISHED:
            _communicationDiscoverDeviceFinished(payload, length);
            break;

        default:
            _communicationReplyWithException(payload);

    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Set device state
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_WRITE_STATE
 * 1 => Device new state
 */
void _communicationWriteStateHandler(
    uint8_t * payload,
    uint16_t length
) {
    // Device is stopped by operator and state can not be changed by gateway
    if (firmwareGetDeviceState() == DEVICE_STATE_STOPPED_BY_OPERATOR) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][WARN] Device is forced to be stopped"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    // Extract pairing command
    const uint8_t device_state = (uint8_t) payload[1];

    switch (device_state)
    {

        case DEVICE_STATE_RUNNING:
            firmwareSetDeviceState(DEVICE_STATE_RUNNING);
            break;

        case DEVICE_STATE_STOPPED:
            firmwareSetDeviceState(DEVICE_STATE_STOPPED);
            break;

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master send invalid state"));
            #endif

            _communicationReplyWithException(payload);

            return;

    }

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Device actual state
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_STATE;
    _communication_output_buffer[1] = (char) _communicationGetDeviceState();

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 2) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device state reply"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with state"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 2);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Provide device state
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_WRITE_STATE
 */
void _communicationReadStateHandler(
    uint8_t * payload,
    uint16_t length
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Device actual state
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_WRITE_STATE;
    _communication_output_buffer[1] = (char) _communicationGetDeviceState();

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 2) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device state reply"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with state"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 2);
    #endif
}

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
// DEVICE PUB SUB
// -----------------------------------------------------------------------------

#if COMMUNICATION_PUB_SUB_PUB_SUPPORT
/**
 * Parse received payload - Write generated register key for PUB_SUB
 * 
 * 0    => Received packet identifier        => COMMUNICATION_PACKET_PUB_SUB_WRITE_REGISTER_KEY
 * 1    => Registers type                    => COMMUNICATION_REGISTER_TYPE_INPUT | COMMUNICATION_REGISTER_TYPE_OUTPUT
 * 2    => High byte of register address
 * 3    => Low byte of register address
 * 4    => Key lenght                        => 1-255
 * 5-n  => Key bytes                         => (a,b,c,...)
 */

void _communicationPubSubWriteRegisterKeyHandler(
    uint8_t * payload,
    uint16_t length
) {
    uint8_t register_type = (uint8_t) payload[1];

    if (
        register_type != COMMUNICATION_REGISTER_TYPE_INPUT
        && register_type != COMMUNICATION_REGISTER_TYPE_OUTPUT
        && register_type != COMMUNICATION_REGISTER_TYPE_ATTRIBUTE
        && register_type != COMMUNICATION_REGISTER_TYPE_SETTING
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Requested register type is not supported"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    // Register read start address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    uint8_t registers_size = 0;

    #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_INPUT) {
            registers_size = COMMUNICATION_MAX_INPUT_REGISTERS_SIZE;
        }
    #endif

    #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
            registers_size = COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE;
        }
    #endif

    #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
            registers_size = COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE;
        }
    #endif

    #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
        // Get total registers count
        if (register_type == COMMUNICATION_REGISTER_TYPE_SETTING) {
            registers_size = COMMUNICATION_MAX_SETTING_REGISTERS_SIZE;
        }
    #endif

    if (
        // Write address mus be between <0, registers_size>
        register_address < registers_size
    ) {
        uint8_t register_key_length = (uint8_t) payload[4];

        if (register_key_length <= COMMUNICATION_REGISTER_KEY_LENGTH) {
            // Initialize register key buffer
            char register_key[COMMUNICATION_REGISTER_KEY_LENGTH + 1];
            memset(register_key, 0, COMMUNICATION_REGISTER_KEY_LENGTH + 1);

            // Extract register key from payload
            for (uint8_t i = 0; i < register_key_length; i++) {
                register_key[i] = (char) payload[i + 5];
                register_key[i + 1] = 0x00; // Add a NULL after each character
            }

            #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
                if (register_type == COMMUNICATION_REGISTER_TYPE_INPUT) {
                    if (register_address <= COMMUNICATION_MAX_INPUT_REGISTERS_SIZE) {
                        memcpy(communication_module_input_registers[register_address].key, register_key, register_key_length + 1);
                    }
                }
            #endif

            #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
                if (register_type == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
                    if (register_address <= COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE) {
                        memcpy(communication_module_output_registers[register_address].key, register_key, register_key_length + 1);
                    }
                }
            #endif

            #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
                if (register_type == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
                    if (register_address <= COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE) {
                        memcpy(communication_module_attribute_registers[register_address].key, register_key, register_key_length + 1);
                    }
                }
            #endif

            #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
                if (register_type == COMMUNICATION_REGISTER_TYPE_SETTING) {
                    if (register_address <= COMMUNICATION_MAX_SETTING_REGISTERS_SIZE) {
                        memcpy(communication_module_setting_registers[register_address].key, register_key, register_key_length + 1);
                    }
                }
            #endif

            memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

            // 0 => Packet identifier
            // 1 => Command response content type
            // 2 => Registers type
            // 3 => High byte of register address
            // 4 => Low byte of register address
            _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PUB_SUB_WRITE_REGISTER_KEY;
            _communication_output_buffer[1] = (char) register_type;
            _communication_output_buffer[2] = (char) (register_address >> 8);
            _communication_output_buffer[3] = (char) (register_address & 0xFF);

            #if DEBUG_COMMUNICATION_SUPPORT
                // Reply to master
                if (_communicationReplyToPacket(_communication_output_buffer, 4) == false) {
                    DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive register key confirmation"));

                } else {
                    DPRINTLN(F("[COMMUNICATION] Replied to master with register key confirmation"));
                }
            #else
                // Reply to master
                _communicationReplyToPacket(_communication_output_buffer, 4);
            #endif

        } else {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write key which is too long"));
                DPRINT(F("[COMMUNICATION][ERR] Expected length: "));
                DPRINT(COMMUNICATION_REGISTER_KEY_LENGTH);
                DPRINT(F(" but received key with length: "));
                DPRINTLN(register_key_length);
                DPRINTLN(length);
            #endif

            _communicationReplyWithException(payload);
        }

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write key for undefined register"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

#if COMMUNICATION_PUB_SUB_SUB_SUPPORT && COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS && COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE

/**
 * Parse received payload - Process received broadcast
 * 
 * 0        => Received packet identifier       => COMMUNICATION_PACKET_PUB_SUB_BROADCAST_REGISTER_VALUE
 * 1-n      => Message key                      => (a-Z0-9)
 * n+1      => Message data type
 * n+2-n+3  => Message value
 */
void _communicationPubSubCheckPublishedMessageHandler(
    uint8_t * payload
) {
    uint8_t message_data_type = (uint8_t) payload[COMMUNICATION_REGISTER_KEY_LENGTH + 1];

    // Check if data type is supported
    if (
        message_data_type != COMMUNICATION_DATA_TYPE_UINT8
        && message_data_type != COMMUNICATION_DATA_TYPE_UINT16
        && message_data_type != COMMUNICATION_DATA_TYPE_UINT32
        && message_data_type != COMMUNICATION_DATA_TYPE_INT8
        && message_data_type != COMMUNICATION_DATA_TYPE_INT16
        && message_data_type != COMMUNICATION_DATA_TYPE_INT32
        && message_data_type != COMMUNICATION_DATA_TYPE_FLOAT32
        && message_data_type != COMMUNICATION_DATA_TYPE_BOOLEAN
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Received broadcastd message with with unsupported data type"));
        #endif

        return;
    }

    char message_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        message_key[i] = payload[i + 1];
    }

    uint8_t subscription_index = INDEX_NONE;

    for (uint8_t i = 0; i < sizeof(_communication_pub_sub_subscriptions) / sizeof(communication_pub_sub_subscription_t); i++) {
        for (uint8_t j = 0; j < sizeof(_communication_pub_sub_subscriptions[i].conditions) / sizeof(uint8_t); j++) {
            uint8_t condition_index = _communication_pub_sub_subscriptions[i].conditions[j];

            if (
                strcmp(_communication_pub_sub_conditions[condition_index].key, message_key) == 0
                && _communication_pub_sub_conditions[condition_index].data_type == message_data_type
            ) {
                if (message_data_type == COMMUNICATION_DATA_TYPE_BOOLEAN) {
                    word message_value = (word) payload[COMMUNICATION_REGISTER_KEY_LENGTH + 2] << 8 | (word) payload[COMMUNICATION_REGISTER_KEY_LENGTH + 3];

                    word condition_value = (word) _communication_pub_sub_conditions[condition_index].value[0] << 8 | (word) _communication_pub_sub_conditions[condition_index].value[1];

                    _communication_pub_sub_conditions[condition_index].fullfiled = condition_value == message_value;

                } else {
                    char message_value[4] = { 0, 0, 0, 0 };

                    message_value[0] = payload[COMMUNICATION_REGISTER_KEY_LENGTH + 2];
                    message_value[1] = payload[COMMUNICATION_REGISTER_KEY_LENGTH + 3];
                    message_value[2] = payload[COMMUNICATION_REGISTER_KEY_LENGTH + 4];
                    message_value[3] = payload[COMMUNICATION_REGISTER_KEY_LENGTH + 5];

                    _communication_pub_sub_conditions[condition_index].fullfiled = true;

                    for (uint8_t val_pos = 0;  val_pos < 4; val_pos++) {
                        if (message_value[val_pos] != _communication_pub_sub_conditions[condition_index].value[val_pos]) {
                            _communication_pub_sub_conditions[condition_index].fullfiled = false;
                        } 
                    }
                }

                subscription_index = i;

                continue;
            }
        }

        if (subscription_index != INDEX_NONE) {
            continue;
        }
    }

    if (subscription_index != INDEX_NONE) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Device is subscribed for this message"));
        #endif

        communication_pub_sub_subscription_t subscription = _communication_pub_sub_subscriptions[subscription_index];

        bool is_fullfiled = true;

        for (uint8_t i = 0; i < sizeof(subscription.conditions) / sizeof(uint8_t); i++) {
            uint8_t condition_index = subscription.conditions[i];

            if (_communication_pub_sub_conditions[condition_index].fullfiled == false) {
                is_fullfiled = false;
            }
        }

        if (is_fullfiled == true) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] All conditions for subscription were met"));
            #endif

            for (uint8_t i = 0; i < sizeof(subscription.actions) / sizeof(uint8_t); i++) {
                uint8_t action_index = subscription.actions[i];

                switch (_communication_pub_sub_actions[action_index].registerType)
                {
                    #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
                        case COMMUNICATION_REGISTER_TYPE_OUTPUT:
                        {
                            if (
                                // Action register address must be between <0, registers_size>
                                _communication_pub_sub_actions[action_index].register_address < COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
                            ) {
                                _communicationWriteValueFromTransfer(
                                    COMMUNICATION_REGISTER_TYPE_OUTPUT,
                                    communication_module_output_registers[_communication_pub_sub_actions[action_index].register_address].data_type,
                                    _communication_pub_sub_actions[action_index].register_address,
                                    _communication_pub_sub_actions[action_index].value
                                );
                            }
                            break;
                        }
                    #endif

                    default:
                        #if DEBUG_COMMUNICATION_SUPPORT
                            DPRINTLN(F("[COMMUNICATION][ERR] Configured action for uknown register"));
                        #endif
                        break;
                }
            }
        }

    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Subscribe to condition
 * 
 * 0            => Received packet identifier       => COMMUNICATION_PACKET_PUB_SUB_SUBSCRIBE
 * 1-n          => Key bytes                        => (a-Z0-9)
 * n+1          => Num of conditions                => 1-255
 * n+2          => Num of actions                   => 1-255
 * n+3-m        => Condition key                    => (a-Z0-9)
 * m+1          => Condition data type
 * m+2-m+2/4    => Condition value
 *              => n Condition
 *              => Action register type
 *              => Action register address
 *              => Action value
 *              => n Action
 */
void _communicationPubSubSubscribeHandler(
    uint8_t * payload
) {
    // Extract count of items
    const uint8_t conditions_count = (uint8_t) payload[COMMUNICATION_REGISTER_KEY_LENGTH + 1];
    const uint8_t actions_count = (uint8_t) payload[COMMUNICATION_REGISTER_KEY_LENGTH + 2];

    if (conditions_count == 0 || actions_count == 0) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][WARN] At least one condition and one action have to configured"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    char subscription_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        subscription_key[i] = payload[i + 1];
    }

    for (uint8_t i = 0; i < (sizeof(_communication_pub_sub_subscriptions) / sizeof(communication_pub_sub_subscription_t)); i++) {
        if (strcmp(_communication_pub_sub_subscriptions[i].key, subscription_key) == 0) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][WARN] Subscription with same key is stored"));
            #endif

            _communicationReplyWithException(payload);

            return;
        }
    }

    uint8_t byte_position = COMMUNICATION_REGISTER_KEY_LENGTH + 3;

    communication_pub_sub_subscription_t subscription;
    memcpy(subscription_key, subscription.key, COMMUNICATION_REGISTER_KEY_LENGTH);

    for (uint8_t i = 0; i < conditions_count; i++) {
        char condition_key[COMMUNICATION_REGISTER_KEY_LENGTH];

        for (uint8_t i = byte_position; i < (byte_position + COMMUNICATION_REGISTER_KEY_LENGTH); i++) {
            condition_key[i] = payload[i];

            byte_position++;
        }

        uint8_t condition_data_type = payload[byte_position];
        byte_position++;

        char condition_value[4] = { 0, 0, 0, 0 };

        if (
            condition_data_type == COMMUNICATION_DATA_TYPE_UINT8
            || condition_data_type == COMMUNICATION_DATA_TYPE_INT8
        ) {
            condition_value[0] = payload[byte_position];
            byte_position++;

        } else if (
            condition_data_type == COMMUNICATION_DATA_TYPE_UINT16
            || condition_data_type == COMMUNICATION_DATA_TYPE_INT16
            || condition_data_type == COMMUNICATION_DATA_TYPE_BOOLEAN
        ) {
            condition_value[0] = payload[byte_position];
            byte_position++;
            condition_value[1] = payload[byte_position];
            byte_position++;

        } else if (
            condition_data_type == COMMUNICATION_DATA_TYPE_UINT32
            || condition_data_type == COMMUNICATION_DATA_TYPE_INT32
            || condition_data_type == COMMUNICATION_DATA_TYPE_FLOAT32
        ) {
            condition_value[0] = payload[byte_position];
            byte_position++;
            condition_value[1] = payload[byte_position];
            byte_position++;
            condition_value[2] = payload[byte_position];
            byte_position++;
            condition_value[3] = payload[byte_position];
            byte_position++;
        }

        communication_pub_sub_condition_t condition;
        memcpy(condition_key, condition.key, COMMUNICATION_REGISTER_KEY_LENGTH);
        condition.data_type = condition_data_type;
        condition.value[0] = condition_value[0];
        condition.value[1] = condition_value[1];
        condition.value[2] = condition_value[2];
        condition.value[3] = condition_value[3];
        condition.fullfiled = false;

        _communication_pub_sub_conditions[sizeof(_communication_pub_sub_conditions) / sizeof(communication_pub_sub_condition_t)] = condition;

        uint8_t condition_index = sizeof(_communication_pub_sub_conditions) / sizeof(communication_pub_sub_condition_t);

        subscription.conditions[sizeof(subscription.conditions) / sizeof(uint8_t)] = condition_index;
    }

    for (uint8_t i = 0; i < actions_count; i++) {
        uint8_t action_registerType = payload[byte_position];
        byte_position++;

        uint8_t action_register_address = payload[byte_position];
        byte_position++;

        char action_value[4] = { 0, 0, 0, 0 };

        switch (action_registerType)
        {
            #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
                case COMMUNICATION_REGISTER_TYPE_OUTPUT:
                {
                    if (
                        // Action register address must be between <0, registers_size>
                        action_register_address < COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
                    ) {
                        if (
                            communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_UINT8
                            || communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_INT8
                        ) {
                            action_value[0] = payload[byte_position];
                            byte_position++;

                        } else if (
                            communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_UINT16
                            || communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_INT16
                        ) {
                            action_value[0] = payload[byte_position];
                            byte_position++;
                            action_value[1] = payload[byte_position];
                            byte_position++;

                        } else if (
                            communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_UINT32
                            || communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_INT32
                            || communication_module_output_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_FLOAT32
                        ) {
                            action_value[0] = payload[byte_position];
                            byte_position++;
                            action_value[1] = payload[byte_position];
                            byte_position++;
                            action_value[2] = payload[byte_position];
                            byte_position++;
                            action_value[3] = payload[byte_position];
                            byte_position++;

                        } else {
                            #if DEBUG_COMMUNICATION_SUPPORT
                                DPRINTLN(F("[COMMUNICATION][WARN] Unsupported register data type"));
                            #endif

                            _communicationReplyWithException(payload);

                            return;
                        }

                    } else {
                        #if DEBUG_COMMUNICATION_SUPPORT
                            DPRINTLN(F("[COMMUNICATION][WARN] Invalid register address for action"));
                        #endif

                        _communicationReplyWithException(payload);

                        return;
                    }
                    break;
                }
            #endif
        
            default:
                #if DEBUG_COMMUNICATION_SUPPORT
                    DPRINTLN(F("[COMMUNICATION][WARN] Invalid register type"));
                #endif

                _communicationReplyWithException(payload);

                return;
        }

        communication_pub_sub_action_t action;
        action.register_address = action_register_address;
        action.registerType = action_registerType;
        action.value[0] = action_value[0];
        action.value[1] = action_value[1];
        action.value[2] = action_value[2];
        action.value[3] = action_value[3];

        _communication_pub_sub_actions[sizeof(_communication_pub_sub_actions) / sizeof(communication_pub_sub_action_t)] = action;

        uint8_t action_index = sizeof(_communication_pub_sub_actions) / sizeof(communication_pub_sub_action_t);

        subscription.actions[sizeof(subscription.actions) / sizeof(uint8_t))] = action_index;
    }

    _communication_pub_sub_subscriptions[sizeof(_communication_pub_sub_subscriptions) / sizeof(communication_pub_sub_subscription_t)] = subscription;

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PUB_SUB_SUBSCRIBE;

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 1) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive subscribe reply"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with accepted subscribe request"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 1);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Unsubscribe from condition
 * 
 * 0    => Received packet identifier       => COMMUNICATION_PACKET_PUB_SUB_UNSUBSCRIBE
 * 1-n  => Key bytes                        => (a-Z0-9)
 */
void _communicationPubSubUnsubscribeHandler(
    uint8_t * payload
) {
    char subscription_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        subscription_key[i] = payload[i + 1];
    }

    uint8_t subscription_index = INDEX_NONE;

    for (uint8_t i = 0; i < (sizeof(_communication_pub_sub_subscriptions) / sizeof(communication_pub_sub_subscription_t)); i++) {
        if (strcmp(_communication_pub_sub_subscriptions[i].key, subscription_key) == 0) {
            subscription_index = i;
        }
    }

    if (subscription_index != INDEX_NONE) {
        // TODO: Do the unsubscribe

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][WARN] Unknown subscription"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------
// COMMUNICATION
// -----------------------------------------------------------------------------

void _communicationReceiverHandler(
    uint8_t * payload,
    const uint16_t length,
    const PJON_Packet_Info &packetInfo
) {
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

        case COMMUNICATION_PACKET_WRITE_STATE:
            _communicationReadStateHandler(data_payload, data_length);
            break;

        case COMMUNICATION_PACKET_READ_STATE:
            _communicationWriteStateHandler(data_payload, data_length);
            break;

        case COMMUNICATION_PACKET_DISCOVER:
            _communicationDiscoverDeviceHandler(data_payload, data_length);
            break;

    /**
     * REGISTERS
     */

        case COMMUNICATION_PACKET_READ_SINGLE_REGISTER:
            _communicationReadSingleRegisterHandler(data_payload);
            break;

        case COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS:
            _communicationReadMultipleRegistersHandler(data_payload);
            break;

        #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
            case COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER:
                _communicationWriteSingleRegisterHandler(data_payload);
                break;
    
            case COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS:
                _communicationWriteMultipleRegistersHandler(data_payload);
                break;
        #endif

    /**
     * PUB/SUB
     */

        #if COMMUNICATION_PUB_SUB_PUB_SUPPORT
            case COMMUNICATION_PACKET_PUB_SUB_WRITE_REGISTER_KEY:
                _communicationPubSubWriteRegisterKeyHandler(data_payload, data_length);
                break;
        #endif

        #if COMMUNICATION_PUB_SUB_SUB_SUPPORT && COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS && COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
            case COMMUNICATION_PACKET_PUB_SUB_BROADCAST_REGISTER_VALUE:
                _communicationPubSubCheckPublishedMessageHandler(data_payload);
                break;

            case COMMUNICATION_PACKET_PUB_SUB_SUBSCRIBE:
                _communicationPubSubSubscribeHandler(data_payload);
                break;

            case COMMUNICATION_PACKET_PUB_SUB_UNSUBSCRIBE:
                _communicationPubSubUnsubscribeHandler(data_payload);
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

uint8_t _communicationGetDeviceState() {
    uint8_t current_state = firmwareGetDeviceState() == DEVICE_STATE_STOPPED_BY_OPERATOR ? DEVICE_STATE_STOPPED : firmwareGetDeviceState();

    if (communicationIsInPairingMode()) {
        current_state = DEVICE_STATE_PAIRING;
    }

    return current_state;
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

void communicationEnablePairingMode()
{
    _communication_pairing_enabled = true;

    #if SYSTEM_DEVICE_COMMUNICATION_LED != INDEX_NONE
        ledSetMode(SYSTEM_DEVICE_COMMUNICATION_LED, LED_MODE_PAIRING);
    #endif
}

// -----------------------------------------------------------------------------

void communicationDisablePairingMode()
{
    _communication_pairing_enabled = false;

    #if SYSTEM_DEVICE_COMMUNICATION_LED != INDEX_NONE
        ledSetMode(SYSTEM_DEVICE_COMMUNICATION_LED, LED_MODE_BUS);
    #endif
}

// -----------------------------------------------------------------------------

bool communicationIsInPairingMode()
{
    return _communication_pairing_enabled;
}

// -----------------------------------------------------------------------------

bool communicationReportDeviceState()
{
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Device actual state
    _communication_output_buffer[0] = COMMUNICATION_PACKET_REPORT_STATE;
    _communication_output_buffer[1] = (char) _communicationGetDeviceState();

    if (_communicationSendPacket(COMMUNICATION_BUS_MASTER_ADDR, _communication_output_buffer, 2) == true) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Device state was successfully sent"));
        #endif

        return true;
    }

    return true;
}

// -----------------------------------------------------------------------------
// REGISTERS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE || COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE || COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE || COMMUNICATION_MAX_SETTING_REGISTERS_SIZE

/**
 * Report value of register to master
 */
bool _communicationReportRegister(
    const uint8_t registerType,
    const uint8_t registerAddress
) {
    if (!firmwareIsRunning()) {
        return false;
    }

    char read_value[4] = { 0, 0, 0, 0 };

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Register type
    // 2    => High byte of register address
    // 3    => Low byte of register address
    // 4-7  => Register value
    _communication_output_buffer[0] = COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER;

    if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_INPUT;

        _communicationReadValueForTransfer(registerType, communication_module_input_registers[registerAddress].data_type, registerAddress, read_value);

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_OUTPUT;

        _communicationReadValueForTransfer(registerType, communication_module_output_registers[registerAddress].data_type, registerAddress, read_value);

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_ATTRIBUTE;

        _communicationReadValueForTransfer(registerType, communication_module_attribute_registers[registerAddress].data_type, registerAddress, read_value);

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_SETTING;

        _communicationReadValueForTransfer(registerType, communication_module_setting_registers[registerAddress].data_type, registerAddress, read_value);
    }

    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = read_value[0];
    _communication_output_buffer[5] = read_value[1];
    _communication_output_buffer[6] = read_value[2];
    _communication_output_buffer[7] = read_value[3];

    if (_communicationSendPacket(COMMUNICATION_BUS_MASTER_ADDR, _communication_output_buffer, 8) == true) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully sent"));
        #endif

        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------

/**
 * Broadcast value of register to all devices
 */
bool _communicationBroadcastRegister(
    const uint8_t registerType,
    const uint8_t registerAddress
) {
    if (!firmwareIsRunning()) {
        return false;
    }

    char read_value[4] = { 0, 0, 0, 0 };

    char register_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    #if COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
        if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
            _communicationReadValueForTransfer(
                registerType,
                communication_module_input_registers[registerAddress].data_type,
                registerAddress,
                read_value
            );

            memcpy(register_key, communication_module_input_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    #if COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
        if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
            _communicationReadValueForTransfer(
                registerType,
                communication_module_output_registers[registerAddress].data_type,
                registerAddress,
                read_value
            );

            memcpy(register_key, communication_module_output_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    #if COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
        if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
            _communicationReadValueForTransfer(
                registerType,
                communication_module_attribute_registers[registerAddress].data_type,
                registerAddress,
                read_value
            );

            memcpy(register_key, communication_module_attribute_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    #if COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
        if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
            _communicationReadValueForTransfer(
                registerType,
                communication_module_setting_registers[registerAddress].data_type,
                registerAddress,
                read_value
            );

            memcpy(register_key, communication_module_setting_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0        => Packet identifier
    // 1        => Register key lenght
    // 2-n      => Register key
    // n+1      => Register data type
    // n+2-n+3  => Register value
    _communication_output_buffer[0] = COMMUNICATION_PACKET_PUB_SUB_BROADCAST_REGISTER_VALUE;
    _communication_output_buffer[1] = COMMUNICATION_REGISTER_KEY_LENGTH;

    uint8_t byte_pointer = 2;
    uint8_t byte_counter = 2;

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        _communication_output_buffer[byte_pointer] = register_key[i];

        byte_pointer++;
        byte_counter++;
    }

    if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
        _communication_output_buffer[byte_pointer] = communication_module_input_registers[registerAddress].data_type;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
        _communication_output_buffer[byte_pointer] = communication_module_output_registers[registerAddress].data_type;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
        _communication_output_buffer[byte_pointer] = communication_module_attribute_registers[registerAddress].data_type;

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
        _communication_output_buffer[byte_pointer] = communication_module_setting_registers[registerAddress].data_type;
    }
    byte_pointer++;
    byte_counter++;
    _communication_output_buffer[byte_pointer] = (char) read_value[0];
    byte_pointer++;
    byte_counter++;
    _communication_output_buffer[byte_pointer] = (char) read_value[1];
    byte_pointer++;
    byte_counter++;
    _communication_output_buffer[byte_pointer] = (char) read_value[2];
    byte_pointer++;
    byte_counter++;
    _communication_output_buffer[byte_pointer] = (char) read_value[3];
    byte_pointer++;
    byte_counter++;

    if (_communicationBroadcastPacket(_communication_output_buffer, byte_counter) == true) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully broadcasted"));
        #endif

        return true;
    }

    return true;
}

// -----------------------------------------------------------------------------

void communicationReadRegister(
    const uint8_t registerType,
    const uint8_t registerAddress,
    void * value,
    const uint8_t size
) {
    if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
        if (
            registerAddress < COMMUNICATION_MAX_INPUT_REGISTERS_SIZE
            && communication_module_input_registers[registerAddress].size == size
        ) {
            memcpy(value, communication_module_input_registers[registerAddress].value, size);

            return;
        }

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
        if (
            registerAddress < COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE
            && communication_module_output_registers[registerAddress].size == size
        ) {
            memcpy(value, communication_module_output_registers[registerAddress].value, size);

            return;
        }

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_ATTRIBUTE) {
        if (
            registerAddress < COMMUNICATION_MAX_ATTRIBUTE_REGISTERS_SIZE
            && communication_module_attribute_registers[registerAddress].size == size
        ) {
            memcpy(value, communication_module_attribute_registers[registerAddress].value, size);

            return;
        }

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_SETTING) {
        if (
            registerAddress < COMMUNICATION_MAX_SETTING_REGISTERS_SIZE
            && communication_module_setting_registers[registerAddress].size == size
        ) {
            memcpy(value, communication_module_setting_registers[registerAddress].value, size);

            return;
        }
    }

    char default_value[4] = { 0, 0, 0, 0 };

    memcpy(value, default_value, size);
}

// -----------------------------------------------------------------------------

bool communicationWriteRegister(
    const uint8_t registerType,
    const uint8_t registerAddress,
    const void * value,
    const uint8_t size
) {
    if (registerType == COMMUNICATION_REGISTER_TYPE_INPUT) {
        if (registerAddress > COMMUNICATION_MAX_INPUT_REGISTERS_SIZE) {
            return false;
        }

        if (communication_module_input_registers[registerAddress].size == size) {
            char stored_value[4] = { 0, 0, 0, 0 };

            _communicationReadValueForTransfer(
                registerType,
                communication_module_input_registers[registerAddress].data_type,
                registerAddress,
                stored_value
            );

            memcpy(communication_module_input_registers[registerAddress].value, value, size);

            if (memcmp((const void *) stored_value, (const void *) value, sizeof(stored_value)) != 0) {
                #if COMMUNICATION_PUB_SUB_SUB_SUPPORT
                if (strlen(communication_module_input_registers[registerAddress].key) > 0) {
                    _communicationBroadcastRegister(registerType, registerAddress);

                } else {
                #endif
                    _communicationReportRegister(registerType, registerAddress);
                #if COMMUNICATION_PUB_SUB_SUB_SUPPORT
                }
                #endif
            }
        }

    } else if (registerType == COMMUNICATION_REGISTER_TYPE_OUTPUT) {
        if (registerAddress > COMMUNICATION_MAX_OUTPUT_REGISTERS_SIZE) {
            return false;
        }

        if (communication_module_output_registers[registerAddress].size == size) {
            char stored_value[4] = { 0, 0, 0, 0 };

            _communicationReadValueForTransfer(
                registerType,
                communication_module_output_registers[registerAddress].data_type,
                registerAddress,
                stored_value
            );

            memcpy(communication_module_output_registers[registerAddress].value, value, size);

            if (memcmp((const void *) stored_value, (const void *) value, sizeof(stored_value)) != 0) {
                #if COMMUNICATION_PUB_SUB_SUB_SUPPORT
                if (strlen(communication_module_output_registers[registerAddress].key) > 0) {
                    _communicationBroadcastRegister(registerType, registerAddress);

                } else {
                #endif
                    _communicationReportRegister(registerType, registerAddress);
                #if COMMUNICATION_PUB_SUB_SUB_SUPPORT
                }
                #endif
            }
        }
    }

    return true;
}
#endif

// Specialized convenience setters (these do not cost memory because of inlining)
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const uint8_t value) { return communicationWriteRegister(registerType, registerAddress, &value, 1); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const uint16_t value) { return communicationWriteRegister(registerType, registerAddress, &value, 2); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const uint32_t value) { return communicationWriteRegister(registerType, registerAddress, &value, 4); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const int8_t value) { return communicationWriteRegister(registerType, registerAddress, &value, 1); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const int16_t value) { return communicationWriteRegister(registerType, registerAddress, &value, 2); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const int32_t value) { return communicationWriteRegister(registerType, registerAddress, &value, 4); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const float value) { return communicationWriteRegister(registerType, registerAddress, &value, 4); }
bool communicationWriteRegister(const uint8_t registerType, const uint8_t registerAddress, const bool value) { return communicationWriteRegister(registerType, registerAddress, &value, 2); }

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, uint8_t &value) { communicationReadRegister(registerType, registerAddress, &value, 1); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, uint16_t &value) { communicationReadRegister(registerType, registerAddress, &value, 2); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, uint32_t &value) { communicationReadRegister(registerType, registerAddress, &value, 4); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, int8_t &value) { communicationReadRegister(registerType, registerAddress, &value, 1); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, int16_t &value) { communicationReadRegister(registerType, registerAddress, &value, 2); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, int32_t &value) { communicationReadRegister(registerType, registerAddress, &value, 4); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, float &value) { communicationReadRegister(registerType, registerAddress, &value, 4); }
void communicationReadRegister(const uint8_t registerType, const uint8_t registerAddress, bool &value) { communicationReadRegister(registerType, registerAddress, &value, 2); }

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
        _communication_serial_bus.begin(SERIAL_BAUDRATE);
    
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

    #if COMMUNICATION_DISABLE_ADDRESS_STORING
        // uint8_t device_address = PJON_NOT_ASSIGNED;
        uint8_t device_address = 1;
    #else
        uint8_t device_address = (uint8_t) EEPROM.read(FLASH_ADDRESS_DEVICE_ADDRESS);
    #endif

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
            communicationReportDeviceState();

            _communication_initial_state_to_master = true;
        }
    }

    // -------------------------------------------------------------------------
    // Bus communication
    // -------------------------------------------------------------------------
    _communication_bus.update();
    _communication_bus.receive();
}
