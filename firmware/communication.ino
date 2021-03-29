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

#if COMMUNICATION_PUB_SUB_SUB_SUPPORT && COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS && (COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_AO_REGISTER_SIZE)
    communication_pub_sub_subscription_t _communication_pub_sub_subscriptions[COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS];

    communication_pub_sub_condition_t _communication_pub_sub_conditions[COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS * COMMUNICATION_PUB_SUB_MAX_CONDITIONS];
    communication_pub_sub_action_t _communication_pub_sub_actions[COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS * COMMUNICATION_PUB_SUB_MAX_ACTIONS];
#endif

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

#if DEBUG_COMMUNICATION_SUPPORT
bool _communicationIsPacketInGroup(
    const uint8_t packetId
) {
    for (uint8_t i = 0; i < COMMUNICATION_PACKET_MAX; i++) {
        if ((uint8_t) pgm_read_byte(communication_packets[i]) == packetId) {
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

uint8_t _communicationGetPacketIndexInGroup(
    const uint8_t packetId
) {
    for (uint8_t i = 0; i < COMMUNICATION_PACKET_MAX; i++) {
        if ((uint8_t) pgm_read_byte(communication_packets[i]) == packetId) {
            return i;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------

void _communicationDebugLogPacket(
    const uint8_t packetId
) {
    DPRINT(packetId);
    DPRINT(F(" => ("));

    char buffer[80];

    if (_communicationIsPacketInGroup(packetId)) {
        strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_string[_communicationGetPacketIndexInGroup(packetId)]));

    } else {
        strncpy_P(buffer, "unknown", sizeof(buffer));
    }

    DPRINT(buffer);
    DPRINTLN(F(")"));
}
#endif

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

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_SETTINGS_DEVICE_SIZE
/**
 * Get device settings buffer size
    */
uint8_t _communicationGetDeviceSettingsBufferSize()
{
    return COMMUNICATION_MAX_SETTINGS_DEVICE_SIZE;
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_SETTINGS_REGISTERS_SIZE
/**
 * Get registers settings buffer size
    */
uint8_t _communicationGetRegistersSettingsBufferSize()
{
    return COMMUNICATION_MAX_SETTINGS_REGISTERS_SIZE;
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_DI_REGISTER_SIZE
/**
 * Get DI or DO buffer size
    */
uint8_t _communicationGetDigitalBufferSize(
    const bool output
) {
    if (output) {
        return COMMUNICATION_MAX_DO_REGISTER_SIZE;
    }

    return COMMUNICATION_MAX_DI_REGISTER_SIZE;
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
/**
 * Get AI or AO buffer size
    */
uint8_t _communicationGetAnalogBufferSize(
    const bool output
) {
    if (output) {
        return COMMUNICATION_MAX_AO_REGISTER_SIZE;
    }

    return COMMUNICATION_MAX_AI_REGISTER_SIZE;
}
#endif

// -----------------------------------------------------------------------------
// ANALOG REGISTERS HELPERS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
/**
 * Read value from analog register & transform it to 4 bytes
    */
void _communicationReadAnalogForTransfer(
    const bool output,
    const uint8_t dataType,
    const uint8_t address,
    char * value
) {
    switch (dataType)
    {
        case COMMUNICATION_DATA_TYPE_UINT8:
            UINT8_UNION_t uint8_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, uint8_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, uint8_read_value.number);
                #endif
            }

            memcpy(value, uint8_read_value.bytes, 1);
            break;

        case COMMUNICATION_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, uint16_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, uint16_read_value.number);
                #endif
            }

            memcpy(value, uint16_read_value.bytes, 2);
            break;

        case COMMUNICATION_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, uint32_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, uint32_read_value.number);
                #endif
            }

            memcpy(value, uint32_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_INT8:
            INT8_UNION_t int8_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, int8_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, int8_read_value.number);
                #endif
            }

            memcpy(value, int8_read_value.bytes, 1);
            break;

        case COMMUNICATION_DATA_TYPE_INT16:
            INT16_UNION_t int16_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, int16_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, int16_read_value.number);
                #endif
            }

            memcpy(value, int16_read_value.bytes, 2);
            break;

        case COMMUNICATION_DATA_TYPE_INT32:
            INT32_UNION_t int32_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, int32_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, int32_read_value.number);
                #endif
            }

            memcpy(value, int32_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_read_value;

            if (output) {
                #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                    communicationReadAnalogOutput(address, float_read_value.number);
                #endif

            } else {
                #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                    communicationReadAnalogInput(address, float_read_value.number);
                #endif
            }

            memcpy(value, float_read_value.bytes, 4);
            break;
    
        default:
            char buffer[4] = { 0, 0, 0, 0 };

            memcpy(value, buffer, 4);
            break;
    }
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE
/**
 * Transform received 4 bytes value into register value
    */
void _communicationWriteAnalogFromTransfer(
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

            communicationReadAnalogOutput(address, uint8_stored_value);

            if (uint8_stored_value != uint8_write_value.number) {
                communicationWriteAnalogOutput(address, uint8_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
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

            communicationReadAnalogOutput(address, uint16_stored_value);

            if (uint16_stored_value != uint16_write_value.number) {
                communicationWriteAnalogOutput(address, uint16_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
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

            communicationReadAnalogOutput(address, uint32_stored_value);

            if (uint32_stored_value != uint32_write_value.number) {
                communicationWriteAnalogOutput(address, uint32_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
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

            communicationReadAnalogOutput(address, int8_stored_value);

            if (int8_stored_value != int8_write_value.number) {
                communicationWriteAnalogOutput(address, int8_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
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

            communicationReadAnalogOutput(address, int16_stored_value);

            if (int16_stored_value != int16_write_value.number) {
                communicationWriteAnalogOutput(address, int16_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
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

            communicationReadAnalogOutput(address, int32_stored_value);

            if (int32_stored_value != int32_write_value.number) {
                communicationWriteAnalogOutput(address, int32_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
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

            communicationReadAnalogOutput(address, float_stored_value);

            if (float_stored_value != float_write_value.number) {
                communicationWriteAnalogOutput(address, float_write_value.number);
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] Value was written into AO register"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped"));
            #endif
            }
            break;

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Provided unknown data type for writing into analog register"));
            #endif
            break;
    }
}
#endif

// -----------------------------------------------------------------------------
// DIGITAL REGISTERS
// -----------------------------------------------------------------------------

/*
void _communicationWriteMultipleDigitalOutputs(
    const uint8_t packetId,
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word write_length = (word) payload[3] << 8 | (word) payload[4];

    uint8_t bytes_count = (uint8_t) payload[5];

    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested write to DO register at address: "));
        DPRINT(register_address);
        DPRINT(F(" and length: "));
        DPRINTLN(write_length);
    #endif

    if (
        // Write start address mus be between <0, buffer.size()>
        register_address < _communicationGetDigitalBufferSize(true)
        // Write end address have to be same or smaller as register size
        && (register_address + write_length) <= _communicationGetDigitalBufferSize(true)
    ) {
        uint8_t write_byte = 1;
        uint8_t data_byte;

        uint8_t write_address = register_address;

        bool write_value = false;

        while (
            write_address < (register_address + write_length)
            && write_address < _communicationGetDigitalBufferSize(true)
            && write_byte <= bytes_count
        ) {
            data_byte = (uint8_t) payload[5 + write_byte];

            for (uint8_t i = 0; i < 8; i++) {
                write_value = (data_byte >> i) & 0x01 ? true : false;

                if (communicationReadDigitalOutput(write_address) != write_value) {
                    communicationWriteDigitalOutput(write_address, write_value);
                #if DEBUG_COMMUNICATION_SUPPORT
                    DPRINT(F("[COMMUNICATION] Value was written into DO register at address: "));
                    DPRINTLN(write_address);

                } else {
                    DPRINT(F("[COMMUNICATION] Value to write into DO register at address: "));
                    DPRINT(write_address);
                    DPRINTLN(F(" is same as stored. Write skipped"));
                #endif
                }

                write_address++;

                if (write_address >= _communicationGetDigitalBufferSize(true)) {
                    break;
                }
            }

            write_byte++;
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
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive multiple DO register write result"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with multiple DO register write result"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, 5);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined DO registers range"));
        #endif

        // TODO: Send exception
    }
}
*/

// -----------------------------------------------------------------------------
// ANALOG REGISTERS
// -----------------------------------------------------------------------------

/*
void _communicationWriteMultipleAnalogOutputs(
    const uint8_t packetId,
    uint8_t * payload
) {
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
        register_address < _communicationGetDigitalBufferSize(true)
        // Write end address have to be same or smaller as register size
        && (register_address + write_length) <= _communicationGetDigitalBufferSize(true)
    ) {
        uint8_t write_byte = 1;
        uint8_t data_byte;

        uint8_t write_address = register_address;

        while (
            write_address < (register_address + write_length)
            && write_address < _communicationGetDigitalBufferSize(true)
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
}
*/

// -----------------------------------------------------------------------------
// WRITING MULTIPLE REGISTERS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE
void _communicationWriteMultipleDigitalRegisters(
    const word registerStartAddress,
    const word writeLength,
    uint8_t * payload
) {

}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE
void _communicationWriteMultipleAnalogRegisters(
    const word registerStartAddress,
    const word writeLength,
    uint8_t * payload
) {

}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_AO_REGISTER_SIZE
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
void _communicationWriteMultipleOutputRegistersHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register write address
    word register_start_address = (word) payload[2] << 8 | (word) payload[3];

    // Number of registers to write
    word write_length = (word) payload[4] << 8 | (word) payload[5];

    switch (register_type)
    {

        #if COMMUNICATION_MAX_DO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_DO:
                _communicationWriteMultipleDigitalRegisters(register_start_address, write_length, payload);
                break;
        #endif

        #if COMMUNICATION_MAX_AO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_AI:
                _communicationWriteMultipleAnalogRegisters(register_start_address, write_length, payload);
                break;
        #endif

        default:
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined registers type"));
            #endif

            _communicationReplyWithException(payload);

    }
}
#endif

// -----------------------------------------------------------------------------
// WRITING SINGLE REGISTER
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE
void _communicationWriteSingleDigitalRegister(
    uint8_t * payload,
    const word registerAddress,
    const word writeValue
) {  
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested writing single value to DO register at address: "));
        DPRINTLN(registerAddress);
    #endif

    // Check if value is TRUE|FALSE or 1|0
    if (writeValue != 0xFF00 && writeValue != 0x0000) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] DO register accept only bool value"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    bool transformed_write_value = writeValue == 0xFF00;

    if (
        // Write address must be between <0, buffer.size()>
        registerAddress < _communicationGetDigitalBufferSize(true)
    ) {
        if (communicationReadDigitalOutput(registerAddress) != transformed_write_value) {
            communicationWriteDigitalOutput(registerAddress, transformed_write_value);
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Value was written into DO register"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Value to write into DO register is same as stored. Write skipped"));
        #endif
        }
    
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0 => Packet identifier
        // 1 => Register type
        // 2 => High byte of register address
        // 3 => Low byte of register address
        // 4 => High byte of written value
        // 5 => Low byte of written value
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER;
        _communication_output_buffer[1] = (char) COMMUNICATION_REGISTER_TYPE_DO;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
        _communication_output_buffer[4] = (char) (writeValue >> 8);
        _communication_output_buffer[5] = (char) (writeValue & 0xFF);

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, 6) == false) {
                // Device was not able to notify master about its address
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive DO register write result"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with DO register write result"));
            }
        #else
            _communicationReplyToPacket(_communication_output_buffer, 6);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined DO register address"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE
void _communicationWriteSingleAnalogRegister(
    uint8_t * payload,
    const word registerAddress,
    char * writeValue
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested writing single value to AO register at address: "));
        DPRINTLN(registerAddress);
    #endif

    if (
        // Write address must be between <0, buffer.size()>
        registerAddress < _communicationGetAnalogBufferSize(true)
    ) {
        _communicationWriteAnalogFromTransfer(communication_module_ao_registers[registerAddress].data_type, registerAddress, writeValue);

        char stored_value[4] = { 0, 0, 0, 0 };

        _communicationReadAnalogForTransfer(true, communication_module_ao_registers[registerAddress].data_type, registerAddress, stored_value);

        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Register type
        // 2    => High byte of register address
        // 3    => Low byte of register address
        // 4-7  => Written value
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER;
        _communication_output_buffer[1] = (char) COMMUNICATION_REGISTER_TYPE_AO;
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
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive AO register write result"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with AO register write result"));
            }
        #else
            _communicationReplyToPacket(_communication_output_buffer, 8);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write to undefined AO register address"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_AO_REGISTER_SIZE
/**
 * Parse received payload - Requesting writing single register
    *
    * 0        => Received packet identifier       => COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER
    * 1        => Register type
    * 2        => High byte of register address
    * 3        => Low byte of register address
    * 4-5(7)   => Data to write into register
    */
void _communicationWriteSingleOutputRegisterHandler(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[1];

    // Register write address
    word register_address = (word) payload[2] << 8 | (word) payload[3];

    switch (register_type)
    {

        #if COMMUNICATION_MAX_DO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_DO:
            {
                word write_digital_value = (word) payload[4] << 8 | (word) payload[5];

                _communicationWriteSingleDigitalRegister(payload, register_address, write_digital_value);
                break;
            }
        #endif

        #if COMMUNICATION_MAX_AO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_AI:
            {
                char write_analog_value[4] = { 0, 0, 0, 0 };

                write_analog_value[0] = payload[4];
                write_analog_value[1] = payload[5];
                write_analog_value[2] = payload[6];
                write_analog_value[3] = payload[7];

                _communicationWriteSingleAnalogRegister(payload, register_address, write_analog_value);
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

// -----------------------------------------------------------------------------
// READING MULTIPLE REGISTERS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_DI_REGISTER_SIZE
void _communicationReplyMultipleDigitalRegister(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t readLength,
    const bool output
) {    
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from multiple digital"));
        if (output) {
            DPRINT(F(" outputs (DO) "));
        } else {
            DPRINT(F(" inputs (DI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINT(registerAddress);
        DPRINT(F(" and length: "));
        DPRINTLN(readLength);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        registerAddress < _communicationGetDigitalBufferSize(output)
        // Read length have to be same or smaller as registers size
        && (registerAddress + readLength) <= _communicationGetDigitalBufferSize(output)
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Register type
        // 2    => High byte of register address
        // 3    => Low byte of register address
        // 4    => Count of data bytes
        // 5-n  => Packet data
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS;
        _communication_output_buffer[1] = (char) output ? COMMUNICATION_REGISTER_TYPE_DO : COMMUNICATION_REGISTER_TYPE_DI;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
        _communication_output_buffer[4] = (char) 0; // Temporary value, will be updated after collecting all

        uint8_t byte_pointer = 5;
        uint8_t bit_counter = 0;
        uint8_t byte_counter = 0;
        uint8_t write_byte = 0;

        bool byte_buffer[8];

        // Reset bit buffer
        for (uint8_t j = 0; j < 8; j++) {
            byte_buffer[j] = 0;
        }

        for (uint8_t i = registerAddress; i < (registerAddress + readLength) && i < _communicationGetDigitalBufferSize(output); i++) {
            #if COMMUNICATION_MAX_DO_REGISTER_SIZE
                if (output) {
                    byte_buffer[bit_counter] = communicationReadDigitalOutput(i) ? 1 : 0;
                }
            #endif

            #if COMMUNICATION_MAX_DI_REGISTER_SIZE
                if (!output) {
                    byte_buffer[bit_counter] = communicationReadDigitalInput(i) ? 1 : 0;
                }
            #endif

            bit_counter++;

            // bit counter reached size of one byte
            if (bit_counter >= 8) {
                bit_counter = 0;

                // Converting BIT array to BYTE => decimal number
                write_byte = 0;

                for (uint8_t wr = 0; wr < 8; wr++) {
                    write_byte |= byte_buffer[wr] << wr;
                }

                // Write converted decimal number to output buffer
                _communication_output_buffer[byte_pointer] = write_byte;

                byte_counter++;
                byte_pointer++;

                // Reset bit buffer
                for (uint8_t j = 0; j < 8; j++) {
                    byte_buffer[j] = 0;
                }
            }
        }

        if (readLength % 8 != 0) {
            // Converting BIT array to BYTE => decimal number
            write_byte = 0;

            for (uint8_t wr = 0; wr < 8; wr++) {
                write_byte |= byte_buffer[wr] << wr;
            }

            // Write converted decimal number to output buffer
            _communication_output_buffer[byte_pointer] = write_byte;

            byte_counter++;
            byte_pointer++;
        }

        // Update data bytes length
        _communication_output_buffer[4] = (char) byte_counter;

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 5)) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive multiple digital registers reading"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with multiple digital registers content"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 5));
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined digital registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
void _communicationReplyMultipleAnalogRegister(
    uint8_t * payload,
    const word registerAddress,
    const uint8_t readLength,
    const bool output
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from multiple analog"));
        if (output) {
            DPRINT(F(" outputs (AO) "));
        } else {
            DPRINT(F(" inputs (AI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINT(registerAddress);
        DPRINT(F(" and length: "));
        DPRINTLN(readLength);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        registerAddress < _communicationGetAnalogBufferSize(output)
        // Read length have to be same or smaller as registers size
        && (registerAddress + readLength) <= _communicationGetAnalogBufferSize(output)
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of data bytes
        // 4-n  => Packet data
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS;
        _communication_output_buffer[1] = (char) output ? COMMUNICATION_REGISTER_TYPE_AO : COMMUNICATION_REGISTER_TYPE_AI;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
        _communication_output_buffer[4] = (char) 0; // Temporary value, will be updated after collecting all

        uint8_t byte_counter = 0;
        uint8_t byte_pointer = 5;

        char read_value[4] = { 0, 0, 0, 0 };

        for (uint8_t i = registerAddress; i < (registerAddress + readLength) && i < _communicationGetAnalogBufferSize(output); i++) {
            #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                if (output) {
                    _communicationReadAnalogForTransfer(output, communication_module_ao_registers[i].data_type, i, read_value);
                }
            #endif

            #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                if (!output) {
                    _communicationReadAnalogForTransfer(output, communication_module_ai_registers[i].data_type, i, read_value);
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
        }

        // Update data bytes length
        _communication_output_buffer[4] = (char) byte_counter;

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 5)) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive multiple analog registers reading"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with multiple analog registers content"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 5));
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined analog registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

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

        #if COMMUNICATION_MAX_DI_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_DI:
                _communicationReplyMultipleDigitalRegister(payload, register_address, read_length, false);
                break;
        #endif

        #if COMMUNICATION_MAX_DO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_DO:
                _communicationReplyMultipleDigitalRegister(payload, register_address, read_length, true);
                break;
        #endif

        #if COMMUNICATION_MAX_AI_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_AI:
                _communicationReplyMultipleAnalogRegister(payload, register_address, read_length, false);
                break;
        #endif

        #if COMMUNICATION_MAX_AO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_AO:
                _communicationReplyMultipleAnalogRegister(payload, register_address, read_length, true);
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

#if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_DI_REGISTER_SIZE
void _communicationReplySingleDigitalRegister(
    uint8_t * payload,
    const word registerAddress,
    const bool output
) {
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from single digital"));
        if (output) {
            DPRINT(F(" output (DO) "));
        } else {
            DPRINT(F(" input (DI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINTLN(registerAddress);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        registerAddress < _communicationGetDigitalBufferSize(output)
    ) {
        word read_value = 0x0000;

        #if COMMUNICATION_MAX_DO_REGISTER_SIZE
            if (output) {
                read_value = communicationReadDigitalOutput(registerAddress) ? 0xFF00 : 0x0000;
            }
        #endif

        #if COMMUNICATION_MAX_DI_REGISTER_SIZE
            if (!output) {
                read_value = communicationReadDigitalInput(registerAddress) ? 0xFF00 : 0x0000;
            }
        #endif

        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Register type
        // 2    => High byte of register address
        // 3    => Low byte of register address
        // 4-5  => Register value
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_SINGLE_REGISTER;
        _communication_output_buffer[1] = (char) output ? COMMUNICATION_REGISTER_TYPE_DO : COMMUNICATION_REGISTER_TYPE_DI;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
        _communication_output_buffer[4] = (char) (read_value >> 8);
        _communication_output_buffer[5] = (char) (read_value & 0xFF);

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, 6) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive digital register reading"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with one digital register content"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, 6);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined digital registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
void _communicationReplySingleAnalogRegister(
    uint8_t * payload,
    const word registerAddress,
    const bool output
) {    
    #if DEBUG_COMMUNICATION_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from single analog"));
        if (output) {
            DPRINT(F(" output (AO) "));
        } else {
            DPRINT(F(" input (AI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINTLN(registerAddress);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        registerAddress < _communicationGetAnalogBufferSize(output)
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Register type
        // 2    => High byte of register address
        // 3    => Low byte of register address
        // 4-7  => Register value
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_READ_SINGLE_REGISTER;
        _communication_output_buffer[1] = (char) output ? COMMUNICATION_REGISTER_TYPE_DO : COMMUNICATION_REGISTER_TYPE_DI;
        _communication_output_buffer[2] = (char) (registerAddress >> 8);
        _communication_output_buffer[3] = (char) (registerAddress & 0xFF);

        char read_value[4] = { 0, 0, 0, 0 };

        #if COMMUNICATION_MAX_AO_REGISTER_SIZE
            if (output) {
                _communicationReadAnalogForTransfer(output, communication_module_ao_registers[registerAddress].data_type, registerAddress, read_value);
            }
        #endif

        #if COMMUNICATION_MAX_AI_REGISTER_SIZE
            if (!output) {
                _communicationReadAnalogForTransfer(output, communication_module_ai_registers[registerAddress].data_type, registerAddress, read_value);
            }
        #endif

        _communication_output_buffer[4] = read_value[0];
        _communication_output_buffer[5] = read_value[1];
        _communication_output_buffer[6] = read_value[2];
        _communication_output_buffer[7] = read_value[3];

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, 8) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive analog register reading"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with one analog register content"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, 8);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read from undefined analog registers range"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

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

        #if COMMUNICATION_MAX_DI_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_DI:
                _communicationReplySingleDigitalRegister(payload, register_address, false);
                break;
        #endif

        #if COMMUNICATION_MAX_DO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_DO:
                _communicationReplySingleDigitalRegister(payload, register_address, true);
                break;
        #endif

        #if COMMUNICATION_MAX_AO_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_AI:
                _communicationReplySingleAnalogRegister(payload, register_address, false);
                break;
        #endif

        #if COMMUNICATION_MAX_AI_REGISTER_SIZE
            case COMMUNICATION_REGISTER_TYPE_AO:
                _communicationReplySingleAnalogRegister(payload, register_address, true);
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
// DEVICE PAIRING MODE
// -----------------------------------------------------------------------------

/**
 * Parse received payload - Requesting address info
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command             => COMMUNICATION_PAIRING_CMD_PROVIDE_ADDRESS
 */
void _communicationPairDeviceProvideAddress(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Command response content type
    // 2    => Device current bus address
    // 3-n  => Device parsed SN
    _communication_output_buffer[0] = (uint8_t) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (uint8_t) COMMUNICATION_PAIRING_RESPONSE_DEVICE_ADDRESS;
    _communication_output_buffer[2] = _communication_bus.device_id();

    uint8_t byte_pointer = 3;
    uint8_t byte_counter = 0;

    for (uint8_t i = 0; i < strlen((char *) DEVICE_SERIAL_NO); i++) {
        _communication_output_buffer[byte_pointer] = ((char *) DEVICE_SERIAL_NO)[i];

        byte_pointer++;
        byte_counter++;
    }

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 3)) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device actual address"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with device actual address"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 3));
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Confirming address
 * 
 * 0    => Received packet identifier       => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1    => Pairing command                  => COMMUNICATION_PAIRING_CMD_SET_ADDRESS
 * 2    => Master assigned address
 * 3-n  => Device SN
 */
void _communicationPairDeviceSetAddress(
    uint8_t * payload
) {
    // Extract address assigned by master
    const uint8_t address = (uint8_t) payload[2];

    uint8_t device_sn_length = (uint8_t) sizeof(payload) - 3;

    // Initialize serial number buffer
    char device_sn[device_sn_length];
    memset(device_sn, 0, device_sn_length);

    // Extract serial number from payload
    for (uint8_t i = 0; i <= (uint8_t) payload[3]; i++) {
        device_sn[i] = (char) payload[i + 4];
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

    // Free used memory
    free(device_sn);

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Command response content type
    // 2-n  => Device SN
    _communication_output_buffer[0] = (uint8_t) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (uint8_t) COMMUNICATION_PAIRING_RESPONSE_ADDRESS_ACCEPTED;

    uint8_t byte_pointer = 2;
    uint8_t byte_counter = 0;

    for (uint8_t i = 0; i < strlen((char *) DEVICE_SERIAL_NO); i++) {
        _communication_output_buffer[byte_pointer] = ((char *) DEVICE_SERIAL_NO)[i];

        byte_pointer++;
        byte_counter++;
    }

    // Reply to master
    if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 2)) == true) {
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

/**
 * Parse received payload - Requesting about device info
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command             => COMMUNICATION_PAIRING_CMD_PROVIDE_ABOUT_INFO
 */
void _communicationPairDeviceProvideAboutInfo(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Command response content type
    // 2 => High byte of max packet length
    // 3 => Low byte of max packet length
    // 4 => High byte of Device description support
    // 5 => Low byte of Device description support
    // 6 => High byte of Device configuration support
    // 7 => Low byte of Device configuration support
    // 8 => High byte of PubSub support
    // 9 => Low byte of PubSub support
    _communication_output_buffer[0] = (uint8_t) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (uint8_t) COMMUNICATION_PAIRING_RESPONSE_ABOUT_INFO;
    _communication_output_buffer[2] = (char) (PJON_PACKET_MAX_LENGTH >> 8);
    _communication_output_buffer[3] = (char) (PJON_PACKET_MAX_LENGTH & 0xFF);
    _communication_output_buffer[4] = (char) ((COMMUNICATION_DESCRIPTION_SUPPORT ? 0xFF00 : 0x0000) >> 8);
    _communication_output_buffer[5] = (char) ((COMMUNICATION_DESCRIPTION_SUPPORT ? 0xFF00 : 0x0000) & 0xFF);
    _communication_output_buffer[6] = (char) ((COMMUNICATION_SETTINGS_SUPPORT ? 0xFF00 : 0x0000) >> 8);
    _communication_output_buffer[7] = (char) ((COMMUNICATION_SETTINGS_SUPPORT ? 0xFF00 : 0x0000) & 0xFF);
    _communication_output_buffer[8] = (char) ((COMMUNICATION_PUB_SUB_PUB_SUPPORT ? 0xFF00 : 0x0000) >> 8);
    _communication_output_buffer[9] = (char) ((COMMUNICATION_PUB_SUB_PUB_SUPPORT ? 0xFF00 : 0x0000) & 0xFF);
    _communication_output_buffer[10] = (char) ((COMMUNICATION_PUB_SUB_SUB_SUPPORT ? 0xFF00 : 0x0000) >> 8);
    _communication_output_buffer[11] = (char) ((COMMUNICATION_PUB_SUB_SUB_SUPPORT ? 0xFF00 : 0x0000) & 0xFF);

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 10) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive maximum packet size"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with maximum packet size"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 10);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Device description
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command             => COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MODEL | COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MANUFACTURER | COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_VERSION | COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_MANUFACTURER | COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_VERSION
 */
void _communicationPairDeviceReportDescription(
    uint8_t * payload,
    const uint8_t commandType,
    const char * descriptionContent
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Command response content type
    // 2-n  => String content
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (char) 0; // Temporary value, will be updated after

    uint8_t byte_pointer = 2;
    uint8_t byte_counter = 0;

    for (uint8_t i = 0; i < strlen(descriptionContent); i++) {
        _communication_output_buffer[byte_pointer] = descriptionContent[i];

        byte_pointer++;
        byte_counter++;
    }

    switch (commandType)
    {

        case COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MODEL:
            _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_DEVICE_MODEL;
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MANUFACTURER:
            _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_DEVICE_MANUFACTURER;
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_VERSION:
            _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_DEVICE_VERSION;
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_MANUFACTURER:
            _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_FIRMWARE_MANUFACTURER;
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_VERSION:
            _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_FIRMWARE_VERSION;
            break;

        default:
            _communicationReplyWithException(payload);

            return;

    }

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 2)) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device description"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with device description"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 2));
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Registers sizes
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command             => COMMUNICATION_PAIRING_CMD_PROVIDE_REGISTERS_SIZE
 */
void _communicationPairDeviceReportRegistersSize(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Command response content type
    // 2 => DI buffer size
    // 3 => DO buffer size
    // 4 => AI buffer size
    // 5 => AO buffer size
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_REGISTERS_SIZE;
    _communication_output_buffer[2] = (char) COMMUNICATION_MAX_DI_REGISTER_SIZE;
    _communication_output_buffer[3] = (char) COMMUNICATION_MAX_DO_REGISTER_SIZE;
    _communication_output_buffer[4] = (char) COMMUNICATION_MAX_AI_REGISTER_SIZE;
    _communication_output_buffer[5] = (char) COMMUNICATION_MAX_AO_REGISTER_SIZE;

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 6) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device registers sizes"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with registers sizes"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 6);
    #endif
}

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
/**
 * Parse received payload - Registers structure
    * 
    * 0 => Received packet identifier      => COMMUNICATION_PACKET_PAIR_DEVICE
    * 1 => Pairing command                 => COMMUNICATION_PAIRING_CMD_PROVIDE_REGISTERS_STRUCTURE
    * 2 => Registers type
    * 3 => High byte of register address
    * 4 => Low byte of register address
    * 5 => High byte of registers length
    * 6 => Low byte of registers length
    */
void _communicationPairDeviceReportRegistersStructure(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[2];

    if (
        register_type != COMMUNICATION_REGISTER_TYPE_AI
        && register_type != COMMUNICATION_REGISTER_TYPE_AO
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Requested register type structure is not supported"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    // Register read start address
    word register_address = (word) payload[3] << 8 | (word) payload[4];

    // Number of registers to read
    word read_length = (word) payload[5] << 8 | (word) payload[6];

    uint8_t registers_size = 0;

    // Get total registers count
    if (register_type == COMMUNICATION_REGISTER_TYPE_AO) {
        registers_size = _communicationGetAnalogBufferSize(true);

    } else {
        registers_size = _communicationGetAnalogBufferSize(false);
    }

    if (
        // Read start address mus be between <0, buffer.size()>
        register_address < registers_size
        // Read length have to be same or smaller as registers size
        && (register_address + read_length) <= registers_size
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Command response content type
        // 2    => Registers type
        // 3    => High byte of register address
        // 4    => Low byte of register address
        // 5    => Register length
        // 6-n  => Register data type
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
        _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_REGISTERS_STRUCTURE;
        _communication_output_buffer[2] = (char) register_type;
        _communication_output_buffer[3] = (char) (register_address >> 8);
        _communication_output_buffer[4] = (char) (register_address & 0xFF);
        _communication_output_buffer[5] = (char) (read_length >> 8);
        _communication_output_buffer[6] = (char) (read_length & 0xFF);

        uint8_t byte_pointer = 7;
        uint8_t byte_counter = 0;

        for (uint8_t i = register_address; i < (register_address + read_length) && i < registers_size; i++) {
            if (register_type == COMMUNICATION_REGISTER_TYPE_AO) {
                _communication_output_buffer[byte_pointer] = communication_module_ao_registers[i].data_type;

            } else {
                _communication_output_buffer[byte_pointer] = communication_module_ai_registers[i].data_type;
            }

            byte_pointer++;
            byte_counter++;
        }

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 7)) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device registers structure"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with registers structure"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 7));
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

#if COMMUNICATION_PUB_SUB_PUB_SUPPORT
/**
 * Parse received payload - Write generated register key for PUB_SUB
    * 
    * 0    => Received packet identifier       => COMMUNICATION_PACKET_PAIR_DEVICE
    * 1    => Pairing command                  => COMMUNICATION_PAIRING_CMD_WRITE_REGISTER_KEY
    * 2    => Registers type
    * 3    => High byte of register address
    * 4    => Low byte of register address
    * 5-n  => Key bytes                        => (a-Z0-9)
    */
void _communicationPairDeviceWriteRegisterKey(
    uint8_t * payload
) {
    uint8_t register_type = (uint8_t) payload[2];

    if (
        register_type != COMMUNICATION_REGISTER_TYPE_DI
        && register_type != COMMUNICATION_REGISTER_TYPE_DO
        && register_type != COMMUNICATION_REGISTER_TYPE_AI
        && register_type != COMMUNICATION_REGISTER_TYPE_AO
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Requested register type is not supported"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    // Register read start address
    word register_address = (word) payload[3] << 8 | (word) payload[4];

    uint8_t registers_size = 0;

    // Get total registers count
    if (register_type == COMMUNICATION_REGISTER_TYPE_DO) {
        #if COMMUNICATION_MAX_DO_REGISTER_SIZE
            registers_size = _communicationGetDigitalBufferSize(true);
        #endif

    } else if (register_type == COMMUNICATION_REGISTER_TYPE_DI) {
        #if COMMUNICATION_MAX_DI_REGISTER_SIZE
            registers_size = _communicationGetDigitalBufferSize(false);
        #endif

    } else if (register_type == COMMUNICATION_REGISTER_TYPE_AO) {
        #if COMMUNICATION_MAX_AO_REGISTER_SIZE
            registers_size = _communicationGetAnalogBufferSize(true);
        #endif

    } else {
        #if COMMUNICATION_MAX_AI_REGISTER_SIZE
            registers_size = _communicationGetAnalogBufferSize(false);
        #endif
    }

    if (
        // Write address mus be between <0, buffer.size()>
        register_address < registers_size
    ) {
        char register_key[COMMUNICATION_REGISTER_KEY_LENGTH];

        for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
            register_key[i] = payload[i + 5];
        }

        if (register_type == COMMUNICATION_REGISTER_TYPE_DO) {
            #if COMMUNICATION_MAX_DO_REGISTER_SIZE
                if (register_address <= _communicationGetDigitalBufferSize(true)) {
                    memcpy(communication_module_do_registers[register_address].key, register_key, COMMUNICATION_REGISTER_KEY_LENGTH);
                    communication_module_do_registers[register_address].publish_as_event = true;
                }
            #endif

        } else if (register_type == COMMUNICATION_REGISTER_TYPE_DI) {
            #if COMMUNICATION_MAX_DI_REGISTER_SIZE
                if (register_address <= _communicationGetDigitalBufferSize(false)) {
                    memcpy(communication_module_di_registers[register_address].key, register_key, COMMUNICATION_REGISTER_KEY_LENGTH);
                    communication_module_di_registers[register_address].publish_as_event = true;
                }
            #endif

        } else if (register_type == COMMUNICATION_REGISTER_TYPE_AO) {
            #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                if (register_address <= _communicationGetAnalogBufferSize(true)) {
                    memcpy(communication_module_ao_registers[register_address].key, register_key, COMMUNICATION_REGISTER_KEY_LENGTH);
                    communication_module_ao_registers[register_address].publish_as_event = true;
                }
            #endif

        } else {
            #if COMMUNICATION_MAX_AI_REGISTER_SIZE
                if (register_address <= _communicationGetAnalogBufferSize(false)) {
                    memcpy(communication_module_ai_registers[register_address].key, register_key, COMMUNICATION_REGISTER_KEY_LENGTH);
                    communication_module_ai_registers[register_address].publish_as_event = true;
                }
            #endif
        }

        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0 => Packet identifier
        // 1 => Command response content type
        // 2 => Device settings buffer size
        // 3 => Registers settings buffer size
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
        _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_WRITE_REGISTER_KEY;
        _communication_output_buffer[2] = (char) register_type;
        _communication_output_buffer[3] = (char) (register_address >> 8);
        _communication_output_buffer[4] = (char) (register_address & 0xFF);

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, 5) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive register key confirmation"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with register key confirmation"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, 5);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to write key for undefined register"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Settings sizes
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command             => COMMUNICATION_PAIRING_CMD_PROVIDE_SETTINGS_SIZE
 */
void _communicationPairDeviceReportSettingsSize(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Command response content type
    // 2 => Device settings buffer size
    // 3 => Registers settings buffer size
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_SETTINGS_SIZE;
    _communication_output_buffer[2] = (char) COMMUNICATION_MAX_SETTINGS_DEVICE_SIZE;
    _communication_output_buffer[3] = (char) COMMUNICATION_MAX_SETTINGS_REGISTERS_SIZE;

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, 4) == false) {
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device settings sizes"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with settings sizes"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, 4);
    #endif
}

// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_SETTINGS_DEVICE_SIZE || COMMUNICATION_MAX_SETTINGS_REGISTERS_SIZE
/**
 * Parse received payload - Settubgs structure
    * 
    * 0    => Received packet identifier       => COMMUNICATION_PACKET_PAIR_DEVICE
    * 1    => Pairing command                  => COMMUNICATION_PAIRING_CMD_PROVIDE_SETTINGS_STRUCTURE
    * 2    => Settings type
    * 3    => High byte of register address
    * 4    => Low byte of register address
    * 5    => High byte of settings length
    * 6    => Low byte of settings length
    */
void _communicationPairDeviceReportSettingsStructure(
    uint8_t * payload
) {
    uint8_t settings_type = (uint8_t) payload[2];

    if (
        settings_type != COMMUNICATION_SETTINGS_TYPE_DEVICE
        && settings_type != COMMUNICATION_SETTINGS_TYPE_REGISTER
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Requested settings type structure is not supported"));
        #endif

        _communicationReplyWithException(payload);

        return;
    }

    // Register read start address
    word settings_address = (word) payload[3] << 8 | (word) payload[4];

    // Number of settings to read
    word read_length = (word) payload[5] << 8 | (word) payload[6];

    uint8_t settings_size = 0;

    // Get total settings count
    if (settings_type == COMMUNICATION_SETTINGS_TYPE_DEVICE) {
        #if COMMUNICATION_MAX_SETTINGS_DEVICE_SIZE
            settings_size = _communicationGetDeviceSettingsBufferSize();
        #endif

    } else {
        #if COMMUNICATION_MAX_SETTINGS_REGISTERS_SIZE
            settings_size = _communicationGetRegistersSettingsBufferSize();
        #endif
    }

    if (
        // Read start address mus be between <0, buffer.size()>
        settings_address < settings_size
        // Read length have to be same or smaller as settings size
        && (settings_address + read_length) <= settings_size
    ) {
        memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

        // 0    => Packet identifier
        // 1    => Command response content type
        // 2    => Settings type
        // 3    => High byte of settings address
        // 4    => Low byte of settings address
        // 5    => High byte of settings length
        // 6    => Low byte of settings length
        // 7-n  => Settings configuration
        _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
        _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_SETTINGS_STRUCTURE;
        _communication_output_buffer[2] = (char) settings_type;
        _communication_output_buffer[3] = (char) (settings_address >> 8);
        _communication_output_buffer[4] = (char) (settings_address & 0xFF);
        _communication_output_buffer[5] = (char) (read_length >> 8);
        _communication_output_buffer[6] = (char) (read_length & 0xFF);

        uint8_t byte_pointer = 7;
        uint8_t byte_counter = 0;

        for (uint8_t i = settings_address; i < (settings_address + read_length) && i < settings_size; i++) {
            if (settings_type == COMMUNICATION_SETTINGS_TYPE_DEVICE) {
                _communication_output_buffer[byte_pointer] = (char) communication_module_settings_device[i].data_type;

                byte_pointer++;
                byte_counter++;

                for (uint8_t char_pos = 0; char_pos < strlen(communication_module_settings_device[i].name); char_pos++) {
                    _communication_output_buffer[byte_pointer] = (char) communication_module_settings_device[i].name[char_pos];

                    byte_pointer++;
                    byte_counter++;
                }

                _communication_output_buffer[byte_pointer] = COMMUNICATION_PACKET_DATA_SPACE;

            } else {
                _communication_output_buffer[byte_pointer] = (char) (communication_module_settings_registers[i].register_address >> 8);

                byte_pointer++;
                byte_counter++;

                _communication_output_buffer[byte_pointer] = (char) (communication_module_settings_registers[i].register_address & 0xFF);

                byte_pointer++;
                byte_counter++;

                _communication_output_buffer[byte_pointer] = (char) communication_module_settings_registers[i].register_type;

                byte_pointer++;
                byte_counter++;

                _communication_output_buffer[byte_pointer] = (char) communication_module_settings_registers[i].data_type;

                byte_pointer++;
                byte_counter++;

                for (uint8_t char_pos = 0; char_pos < strlen(communication_module_settings_registers[i].name); char_pos++) {
                    _communication_output_buffer[byte_pointer] = (char) communication_module_settings_registers[i].name[char_pos];

                    byte_pointer++;
                    byte_counter++;
                }

                _communication_output_buffer[byte_pointer] = (char) COMMUNICATION_PACKET_DATA_SPACE;
            }

            byte_pointer++;
            byte_counter++;
        }

        #if DEBUG_COMMUNICATION_SUPPORT
            // Reply to master
            if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 7)) == false) {
                DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive settings structure"));

            } else {
                DPRINTLN(F("[COMMUNICATION] Replied to master with settings structure"));
            }
        #else
            // Reply to master
            _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 7));
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][ERR] Master is trying to read structure for undefined settings range"));
        #endif

        _communicationReplyWithException(payload);
    }
}
#endif

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Finish pairing
 * 
 * 0 => Received packet identifier   => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command
 */
void _communicationPairDeviceFinishPairing(
    uint8_t * payload
) {
    communicationDisablePairingMode();

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Command response content type
    // 2 => Device actual state
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PAIR_DEVICE;
    _communication_output_buffer[1] = (char) COMMUNICATION_PAIRING_RESPONSE_FINISHED;
    _communication_output_buffer[2] = (char) firmwareGetDeviceState() == DEVICE_STATE_STOPPED_BY_OPERATOR ? DEVICE_STATE_STOPPED : firmwareGetDeviceState();

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
 * Parse received payload - Provide device info
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PAIR_DEVICE
 * 1 => Pairing command
 */
void _communicationPairDeviceHandler(
    uint8_t * payload
) {
    if (_communication_pairing_enabled == false) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION][WARN] Device is not in pairing mode"));
        #endif

        return;
    }

    // Extract pairing command
    const uint8_t pairing_command = (uint8_t) payload[1];

    switch (pairing_command)
    {

        case COMMUNICATION_PAIRING_CMD_PROVIDE_ADDRESS:
            _communicationPairDeviceProvideAddress(payload);
            break;

        case COMMUNICATION_PAIRING_CMD_SET_ADDRESS:
            _communicationPairDeviceSetAddress(payload);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_ABOUT_INFO:
            _communicationPairDeviceProvideAboutInfo(payload);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MODEL:
            _communicationPairDeviceReportDescription(payload, COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MODEL, (char *) DEVICE_NAME);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MANUFACTURER:
            _communicationPairDeviceReportDescription(payload, COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_MANUFACTURER, (char *) DEVICE_MANUFACTURER);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_VERSION:
            _communicationPairDeviceReportDescription(payload, COMMUNICATION_PAIRING_CMD_PROVIDE_DEVICE_VERSION, (char *) DEVICE_VERSION);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_MANUFACTURER:
            _communicationPairDeviceReportDescription(payload, COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_MANUFACTURER, (char *) FIRMWARE_MANUFACTURER);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_VERSION:
            _communicationPairDeviceReportDescription(payload, COMMUNICATION_PAIRING_CMD_PROVIDE_FIRMWARE_VERSION, (char *) FIRMWARE_VERSION);
            break;

        case COMMUNICATION_PAIRING_CMD_PROVIDE_REGISTERS_SIZE:
            _communicationPairDeviceReportRegistersSize(payload);
            break;

        #if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
            case COMMUNICATION_PAIRING_CMD_PROVIDE_REGISTERS_STRUCTURE:
                _communicationPairDeviceReportRegistersStructure(payload);
                break;
        #endif

        #if COMMUNICATION_PUB_SUB_PUB_SUPPORT
            case COMMUNICATION_PAIRING_CMD_WRITE_REGISTER_KEY:
                _communicationPairDeviceWriteRegisterKey(payload);
                break;
        #endif

        case COMMUNICATION_PAIRING_CMD_PROVIDE_SETTINGS_SIZE:
            _communicationPairDeviceReportSettingsSize(payload);
            break;

        #if COMMUNICATION_MAX_SETTINGS_DEVICE_SIZE || COMMUNICATION_MAX_SETTINGS_REGISTERS_SIZE
            case COMMUNICATION_PAIRING_CMD_PROVIDE_SETTINGS_STRUCTURE:
                _communicationPairDeviceReportSettingsStructure(payload);
                break;
        #endif

        case COMMUNICATION_PAIRING_CMD_FINISHED:
            _communicationPairDeviceFinishPairing(payload);
            break;

        default:
            _communicationReplyWithException(payload);

    }
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Master hello
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_GET_STATE
 * 1 => Device new state
 */
void _communicationSetStateHandler(
    uint8_t * payload
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
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_SET_STATE;
    _communication_output_buffer[1] = (char) firmwareGetDeviceState() == DEVICE_STATE_STOPPED_BY_OPERATOR ? DEVICE_STATE_STOPPED : firmwareGetDeviceState();

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
 * Parse received payload - Master hello
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_GET_STATE
 */
void _communicationGetStateHandler(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    // 1 => Device actual state
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_GET_STATE;
    _communication_output_buffer[1] = (char) firmwareGetDeviceState() == DEVICE_STATE_STOPPED_BY_OPERATOR ? DEVICE_STATE_STOPPED : firmwareGetDeviceState();

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
 * Parse received payload - Master hello
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_HELLO
 */
void _communicationHelloHandler(
    uint8_t * payload
) {
    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 2    => Device SN length
    // 3-n  => Device SN
    _communication_output_buffer[0] = (uint8_t) COMMUNICATION_PACKET_HELLO;
    _communication_output_buffer[1] = (uint8_t) strlen((char *) DEVICE_SERIAL_NO);

    uint8_t byte_pointer = 2;
    uint8_t byte_counter = 0;

    for (uint8_t i = 0; i < strlen((char *) DEVICE_SERIAL_NO); i++) {
        _communication_output_buffer[byte_pointer] = ((char *) DEVICE_SERIAL_NO)[i];

        byte_pointer++;
        byte_counter++;
    }

    #if DEBUG_COMMUNICATION_SUPPORT
        // Reply to master
        if (_communicationReplyToPacket(_communication_output_buffer, (byte_counter + 2)) == false) {
            // Device was not able to notify master about its address
            DPRINTLN(F("[COMMUNICATION][ERR] Master could not receive device hello reply"));

        } else {
            DPRINTLN(F("[COMMUNICATION] Replied to master with hello"));
        }
    #else
        // Reply to master
        _communicationReplyToPacket(_communication_output_buffer, (byte_counter + 2));
    #endif
}

// -----------------------------------------------------------------------------

/**
 * Parse received payload - Master ping
 * 
 * 0 => Received packet identifier  => COMMUNICATION_PACKET_PING
 */
void _communicationPingHandler(
    uint8_t * payload
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

#if COMMUNICATION_PUB_SUB_SUB_SUPPORT && COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS && (COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_AO_REGISTER_SIZE)
/**
 * Parse received payload - Process received broadcast
    * 
    * 0        => Received packet identifier       => COMMUNICATION_PACKET_PUBSUB_BROADCAST
    * 1-n      => Message key                      => (a-Z0-9)
    * n+1      => Message data type
    * n+2-n+3  => Message value
    */
void _communicationCheckPublishedMessageHandler(
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

    for (uint8_t i = 0; i < sizeof(_communication_pub_sub_subscriptions) / sizeof(* _communication_pub_sub_subscriptions); i++) {
        for (uint8_t j = 0; j < sizeof(_communication_pub_sub_subscriptions[i].conditions) / sizeof(* _communication_pub_sub_subscriptions[i].conditions); j++) {
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

        for (uint8_t i = 0; i < sizeof(subscription.conditions) / sizeof(* subscription.conditions); i++) {
            uint8_t condition_index = subscription.conditions[i];

            if (_communication_pub_sub_conditions[condition_index].fullfiled == false) {
                is_fullfiled = false;
            }
        }

        if (is_fullfiled == true) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINTLN(F("[COMMUNICATION] All conditions for subscription were met"));
            #endif

            for (uint8_t i = 0; i < sizeof(subscription.actions) / sizeof(* subscription.actions); i++) {
                uint8_t action_index = subscription.actions[i];

                switch (_communication_pub_sub_actions[action_index].register_type)
                {
                    #if COMMUNICATION_MAX_DO_REGISTER_SIZE
                        case COMMUNICATION_REGISTER_TYPE_DO:
                        {
                            if (
                                // Action register address must be between <0, buffer.size()>
                                _communication_pub_sub_actions[action_index].register_address < _communicationGetDigitalBufferSize(true)
                            ) {
                                word action_value = (word) _communication_pub_sub_actions[action_index].value[0] << 8 | (word) _communication_pub_sub_actions[action_index].value[1];

                                communicationWriteDigitalOutput(
                                    _communication_pub_sub_actions[action_index].register_address,
                                    (action_value == 0xFF00)
                                );
                            }
                            break;
                        }
                    #endif

                    #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                        case COMMUNICATION_REGISTER_TYPE_AO:
                        {
                            if (
                                // Action register address must be between <0, buffer.size()>
                                _communication_pub_sub_actions[action_index].register_address < _communicationGetAnalogBufferSize(true)
                            ) {
                                _communicationWriteAnalogFromTransfer(
                                    communication_module_ao_registers[_communication_pub_sub_actions[action_index].register_address].data_type,
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
    * 0            => Received packet identifier       => COMMUNICATION_PACKET_PUBSUB_SUBSCRIBE
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
void _communicationSubscribePubSubHandler(
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

    for (uint8_t i = 0; i < (sizeof(_communication_pub_sub_subscriptions) / sizeof(* _communication_pub_sub_subscriptions)); i++) {
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

        _communication_pub_sub_conditions[sizeof(_communication_pub_sub_conditions) / sizeof(* _communication_pub_sub_conditions)] = condition;

        uint8_t condition_index = sizeof(_communication_pub_sub_conditions) / sizeof(* _communication_pub_sub_conditions);

        subscription.conditions[sizeof(subscription.conditions) / sizeof(* subscription.conditions)] = condition_index;
    }

    for (uint8_t i = 0; i < actions_count; i++) {
        uint8_t action_register_type = payload[byte_position];
        byte_position++;

        uint8_t action_register_address = payload[byte_position];
        byte_position++;

        char action_value[4] = { 0, 0, 0, 0 };

        switch (action_register_type)
        {
            #if COMMUNICATION_MAX_DO_REGISTER_SIZE
                case COMMUNICATION_REGISTER_TYPE_DO:
                {
                    if (
                        // Action register address must be between <0, buffer.size()>
                        action_register_address < _communicationGetDigitalBufferSize(true)
                    ) {
                        action_value[0] = payload[byte_position];
                        byte_position++;
                        action_value[1] = payload[byte_position];
                        byte_position++;

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

            #if COMMUNICATION_MAX_AO_REGISTER_SIZE
                case COMMUNICATION_REGISTER_TYPE_AO:
                {
                    if (
                        // Action register address must be between <0, buffer.size()>
                        action_register_address < _communicationGetDigitalBufferSize(true)
                    ) {
                        if (
                            communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_UINT8
                            || communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_INT8
                        ) {
                            action_value[0] = payload[byte_position];
                            byte_position++;

                        } else if (
                            communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_UINT16
                            || communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_INT16
                        ) {
                            action_value[0] = payload[byte_position];
                            byte_position++;
                            action_value[1] = payload[byte_position];
                            byte_position++;

                        } else if (
                            communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_UINT32
                            || communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_INT32
                            || communication_module_ao_registers[action_register_address].data_type == COMMUNICATION_DATA_TYPE_FLOAT32
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
        action.register_type = action_register_type;
        action.value[0] = action_value[0];
        action.value[1] = action_value[1];
        action.value[2] = action_value[2];
        action.value[3] = action_value[3];

        _communication_pub_sub_actions[sizeof(_communication_pub_sub_actions) / sizeof(* _communication_pub_sub_actions)] = action;

        uint8_t action_index = sizeof(_communication_pub_sub_actions) / sizeof(* _communication_pub_sub_actions);

        subscription.actions[sizeof(subscription.actions) / sizeof(* subscription.actions)] = action_index;
    }

    _communication_pub_sub_subscriptions[sizeof(_communication_pub_sub_subscriptions) / sizeof(* _communication_pub_sub_subscriptions)] = subscription;

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0 => Packet identifier
    _communication_output_buffer[0] = (char) COMMUNICATION_PACKET_PUBSUB_SUBSCRIBE;

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
    * 0    => Received packet identifier       => COMMUNICATION_PACKET_PUBSUB_UNSUBSCRIBE
    * 1-n  => Key bytes                        => (a-Z0-9)
    */
void _communicationUnsubscribePubSubHandler(
    uint8_t * payload
) {
    char subscription_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        subscription_key[i] = payload[i + 1];
    }

    uint8_t subscription_index = INDEX_NONE;

    for (uint8_t i = 0; i < (sizeof(_communication_pub_sub_subscriptions) / sizeof(* _communication_pub_sub_subscriptions)); i++) {
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
        _communicationDebugLogPacket(packet_id);
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

        case COMMUNICATION_PACKET_PING:
            _communicationPingHandler(data_payload);
            break;

        case COMMUNICATION_PACKET_HELLO:
            _communicationHelloHandler(data_payload);
            break;

        case COMMUNICATION_PACKET_GET_STATE:
            _communicationGetStateHandler(data_payload);
            break;

        case COMMUNICATION_PACKET_SET_STATE:
            _communicationSetStateHandler(data_payload);
            break;

        case COMMUNICATION_PACKET_PAIR_DEVICE:
            _communicationPairDeviceHandler(data_payload);
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

        #if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_AO_REGISTER_SIZE
            case COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER:
                _communicationWriteSingleOutputRegisterHandler(data_payload);
                break;
    
            case COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS:
                _communicationWriteMultipleOutputRegistersHandler(data_payload);
                break;
        #endif

    /**
     * PUB/SUB
     */

        #if COMMUNICATION_PUB_SUB_SUB_SUPPORT && COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS && (COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_AO_REGISTER_SIZE)
            case COMMUNICATION_PACKET_PUBSUB_BROADCAST:
                _communicationCheckPublishedMessageHandler(data_payload);
                break;

            case COMMUNICATION_PACKET_PUBSUB_SUBSCRIBE:
                _communicationSubscribePubSubHandler(data_payload);
                break;

            case COMMUNICATION_PACKET_PUBSUB_UNSUBSCRIBE:
                _communicationUnsubscribePubSubHandler(data_payload);
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
            final_payload,          // Content
            sizeof(final_payload)   // Content length
        );

    } else {
        return _communication_bus.send_packet(
            address,                // Recepient address
            final_payload,          // Content
            sizeof(final_payload)   // Content length
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
                _communicationDebugLogPacket((uint8_t) payload[0]);
            #endif

        } else if (result == PJON_FAIL) {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Sending packet to address: "));
                DPRINT(address);
                DPRINT(F(" failed, packet: "));
                _communicationDebugLogPacket((uint8_t) payload[0]);
            #endif

        } else {
            #if DEBUG_COMMUNICATION_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Sending packet to address: "));
                DPRINT(address);
                DPRINT(F(" failed, unknonw error, packet: "));
                _communicationDebugLogPacket((uint8_t) payload[0]);
            #endif
        }

        return false;
    }

    if (address == PJON_BROADCAST) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION] Successfully sent broadcast packet "));
            _communicationDebugLogPacket((uint8_t) payload[0]);
        #endif

    } else {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINT(F("[COMMUNICATION] Successfully sent packet to address: "));
            DPRINT(address);
            DPRINT(F(" "));
            _communicationDebugLogPacket((uint8_t) payload[0]);
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
        _communicationDebugLogPacket((uint8_t) payload[0]);
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
        _communicationDebugLogPacket((uint8_t) payload[0]);
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
    _communication_output_buffer[1] = (char) firmwareGetDeviceState() == DEVICE_STATE_STOPPED_BY_OPERATOR ? DEVICE_STATE_STOPPED : firmwareGetDeviceState();

    if (
        _communicationSendPacket(
            COMMUNICATION_BUS_MASTER_ADDR,
            _communication_output_buffer,
            2
        ) == true
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Device state was successfully sent"));
        #endif

        return true;
    }

    return true;
}

// -----------------------------------------------------------------------------
// DIGITAL REGISTERS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE || COMMUNICATION_MAX_DI_REGISTER_SIZE
/**
 * Report value of digital register to master
    */
bool communicationReportDigitalRegister(
    const bool output,
    const uint8_t registerAddress
) {
    if (!firmwareIsRunning()) {
        return false;
    }

    word read_value = 0x0000;

    #if COMMUNICATION_MAX_DO_REGISTER_SIZE
        if (output) {
            read_value = communicationReadDigitalOutput(registerAddress) ? 0xFF00 : 0x0000;
        }
    #endif

    #if COMMUNICATION_MAX_DI_REGISTER_SIZE
        if (!output) {
            read_value = communicationReadDigitalInput(registerAddress) ? 0xFF00 : 0x0000;
        }
    #endif

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Register type
    // 2    => High byte of register address
    // 3    => Low byte of register address
    // 4-5  => Register value
    _communication_output_buffer[0] = COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER;

    if (output) {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_DO;

    } else {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_DI;
    }

    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = (char) (read_value >> 8);
    _communication_output_buffer[5] = (char) (read_value & 0xFF);

    if (
        _communicationSendPacket(
            COMMUNICATION_BUS_MASTER_ADDR,
            _communication_output_buffer,
            6
        ) == true
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully sent"));
        #endif

        return true;
    }

    return true;
}

// -----------------------------------------------------------------------------

/**
 * Broadcast value of digital register to all devices
    */
bool communicationBroadcastDigitalRegister(
    const bool output,
    const uint8_t registerAddress
) {
    if (!firmwareIsRunning()) {
        return false;
    }

    word read_value = 0x0000;

    char register_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    #if COMMUNICATION_MAX_DO_REGISTER_SIZE
        if (output) {
            read_value = communicationReadDigitalOutput(registerAddress) ? 0xFF00 : 0x0000;
            memcpy(register_key, communication_module_do_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    #if COMMUNICATION_MAX_DI_REGISTER_SIZE
        if (!output) {
            read_value = communicationReadDigitalInput(registerAddress) ? 0xFF00 : 0x0000;
            memcpy(register_key, communication_module_di_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0        => Packet identifier
    // 1-n      => Register key
    // n+1      => Register data type
    // n+2-n+3  => Register value
    _communication_output_buffer[0] = COMMUNICATION_PACKET_PUBSUB_BROADCAST;

    uint8_t byte_pointer = 1;
    uint8_t byte_counter = 1;

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        _communication_output_buffer[byte_pointer] = register_key[i];

        byte_pointer++;
        byte_counter++;
    }

    _communication_output_buffer[byte_pointer] = COMMUNICATION_DATA_TYPE_BOOLEAN;
    byte_pointer++;
    byte_counter++;
    _communication_output_buffer[byte_pointer] = (char) (read_value >> 8);
    byte_pointer++;
    byte_counter++;
    _communication_output_buffer[byte_pointer] = (char) (read_value & 0xFF);

    if (
        _communicationBroadcastPacket(
            _communication_output_buffer,
            byte_counter
        ) == true
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully broadcasted"));
        #endif

        return true;
    }

    return true;
}
#endif

// -----------------------------------------------------------------------------
// DIGITAL INPUTS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DI_REGISTER_SIZE
bool communicationWriteDigitalInput(
    const uint8_t registerAddress,
    const bool value
) {
    if (registerAddress > _communicationGetDigitalBufferSize(false)) {
        return false;
    }

    if (communication_module_di_registers[registerAddress].value != value) {
        communication_module_di_registers[registerAddress].value = value;

        if (sizeof(communication_module_di_registers[registerAddress].key) / sizeof(* communication_module_di_registers[registerAddress].key) > 0) {
            communicationBroadcastDigitalRegister(false, registerAddress);

        } else if (communication_module_di_registers[registerAddress].publish_as_event) {
            communicationReportDigitalRegister(false, registerAddress);
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalInput(
    const uint8_t registerAddress
) {
    if (registerAddress > _communicationGetDigitalBufferSize(false)) {
        return false;
    }

    return communication_module_di_registers[registerAddress].value;
}
#endif

// -----------------------------------------------------------------------------
// DIGITAL OUTPUTS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_DO_REGISTER_SIZE
bool communicationWriteDigitalOutput(
    const uint8_t registerAddress,
    const bool value
) {
    if (registerAddress > _communicationGetDigitalBufferSize(true)) {
        return false;
    }

    if (communication_module_do_registers[registerAddress].value != value) {
        communication_module_do_registers[registerAddress].value = value;

        if (communication_module_do_registers[registerAddress].publish_as_event) {
            if (sizeof(communication_module_do_registers[registerAddress].key) / sizeof(* communication_module_do_registers[registerAddress].key) > 0) {
                communicationBroadcastDigitalRegister(true, registerAddress);

            } else if (communication_module_do_registers[registerAddress].publish_as_event) {
                communicationReportDigitalRegister(true, registerAddress);
            }
        }
    }

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalOutput(
    const uint8_t registerAddress
) {
    if (registerAddress > _communicationGetDigitalBufferSize(true)) {
        return false;
    }

    return communication_module_do_registers[registerAddress].value;
}
#endif

// -----------------------------------------------------------------------------
// ANALOG REGISTERS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE || COMMUNICATION_MAX_AI_REGISTER_SIZE
/**
 * Report value of analog register to master
    */
bool communicationReportAnalogRegister(
    const bool output,
    const uint8_t registerAddress
) {
    if (!firmwareIsRunning()) {
        return false;
    }

    char read_value[4] = { 0, 0, 0, 0 };

    _communicationReadAnalogForTransfer(output, communication_module_ai_registers[registerAddress].data_type, registerAddress, read_value);    

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0    => Packet identifier
    // 1    => Register type
    // 2    => High byte of register address
    // 3    => Low byte of register address
    // 4-7  => Register value
    _communication_output_buffer[0] = COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER;

    if (output) {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_AO;

    } else {
        _communication_output_buffer[1] = COMMUNICATION_REGISTER_TYPE_AI;
    }

    _communication_output_buffer[2] = (char) (registerAddress >> 8);
    _communication_output_buffer[3] = (char) (registerAddress & 0xFF);
    _communication_output_buffer[4] = read_value[0];
    _communication_output_buffer[5] = read_value[1];
    _communication_output_buffer[6] = read_value[2];
    _communication_output_buffer[7] = read_value[3];

    if (
        _communicationSendPacket(
            COMMUNICATION_BUS_MASTER_ADDR,
            _communication_output_buffer,
            8
        ) == true
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully sent"));
        #endif

        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------

/**
 * Broadcast value of analog register to all devices
    */
bool communicationBroadcastAnalogRegister(
    const bool output,
    const uint8_t registerAddress
) {
    if (!firmwareIsRunning()) {
        return false;
    }

    char read_value[4] = { 0, 0, 0, 0 };

    char register_key[COMMUNICATION_REGISTER_KEY_LENGTH];

    #if COMMUNICATION_MAX_AO_REGISTER_SIZE
        if (output) {
            _communicationReadAnalogForTransfer(true, communication_module_ao_registers[registerAddress].data_type, registerAddress, read_value);

            memcpy(register_key, communication_module_ao_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    #if COMMUNICATION_MAX_AI_REGISTER_SIZE
        if (!output) {
            _communicationReadAnalogForTransfer(false, communication_module_ai_registers[registerAddress].data_type, registerAddress, read_value);

            memcpy(register_key, communication_module_ai_registers[registerAddress].key, COMMUNICATION_REGISTER_KEY_LENGTH);
        }
    #endif

    memset(_communication_output_buffer, 0, PJON_PACKET_MAX_LENGTH);

    // 0        => Packet identifier
    // 1-n      => Register key
    // n+1      => Register data type
    // n+2-n+3  => Register value
    _communication_output_buffer[0] = COMMUNICATION_PACKET_PUBSUB_BROADCAST;

    uint8_t byte_pointer = 1;
    uint8_t byte_counter = 1;

    for (uint8_t i = 0; i < COMMUNICATION_REGISTER_KEY_LENGTH; i++) {
        _communication_output_buffer[byte_pointer] = register_key[i];

        byte_pointer++;
        byte_counter++;
    }

    if (output) {
        _communication_output_buffer[byte_pointer] = communication_module_ao_registers[registerAddress].data_type;
    } else {
        _communication_output_buffer[byte_pointer] = communication_module_ai_registers[registerAddress].data_type;
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

    if (
        _communicationBroadcastPacket(
            _communication_output_buffer,
            byte_counter
        ) == true
    ) {
        #if DEBUG_COMMUNICATION_SUPPORT
            DPRINTLN(F("[COMMUNICATION] Register value was successfully broadcasted"));
        #endif

        return true;
    }

    return true;
}

// -----------------------------------------------------------------------------

void communicationReadAnalogRegister(
    const bool output,
    const uint8_t registerAddress,
    void * value,
    const uint8_t size
) {
    if (output) {
        if (
            registerAddress < _communicationGetAnalogBufferSize(true)
            && communication_module_ao_registers[registerAddress].size == size
        ) {
            memcpy(value, communication_module_ao_registers[registerAddress].value, size);

            return;
        }

    } else {
        if (
            registerAddress < _communicationGetAnalogBufferSize(false)
            && communication_module_ai_registers[registerAddress].size == size
        ) {
            memcpy(value, communication_module_ai_registers[registerAddress].value, size);

            return;
        }
    }

    char default_value[4] = { 0, 0, 0, 0 };

    memcpy(value, default_value, size);
}

// -----------------------------------------------------------------------------

bool communicationWriteAnalogRegister(
    const bool output,
    const uint8_t registerAddress,
    const void * value,
    const uint8_t size
) {
    if (output) {
        if (registerAddress > _communicationGetAnalogBufferSize(true)) {
            return false;
        }

        if (communication_module_ao_registers[registerAddress].size == size) {
            char stored_value[4] = { 0, 0, 0, 0 };

            _communicationReadAnalogForTransfer(false, communication_module_ao_registers[registerAddress].data_type, registerAddress, stored_value);

            memcpy(communication_module_ao_registers[registerAddress].value, value, size);

            if (memcmp((const void *) stored_value, (const void *) value, sizeof(stored_value)) != 0) {
                if (sizeof(communication_module_ao_registers[registerAddress].key) / sizeof(* communication_module_ao_registers[registerAddress].key) > 0) {
                    communicationBroadcastAnalogRegister(output, registerAddress);

                } else if (communication_module_ao_registers[registerAddress].publish_as_event) {
                    communicationReportAnalogRegister(output, registerAddress);
                }
            }
        }

    } else {
        if (registerAddress > _communicationGetAnalogBufferSize(false)) {
            return false;
        }

        if (communication_module_ai_registers[registerAddress].size == size) {
            char stored_value[4] = { 0, 0, 0, 0 };

            _communicationReadAnalogForTransfer(output, communication_module_ai_registers[registerAddress].data_type, registerAddress, stored_value);

            memcpy(communication_module_ai_registers[registerAddress].value, value, size);

            if (memcmp((const void *) stored_value, (const void *) value, sizeof(stored_value)) != 0) {
                if (sizeof(communication_module_ai_registers[registerAddress].key) / sizeof(* communication_module_ai_registers[registerAddress].key) > 0) {
                    communicationBroadcastAnalogRegister(output, registerAddress);

                } else if (communication_module_ai_registers[registerAddress].publish_as_event) {
                    communicationReportAnalogRegister(output, registerAddress);
                }
            }
        }
    }

    return true;
}
#endif

// -----------------------------------------------------------------------------
// ANALOG INPUTS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AI_REGISTER_SIZE
// Specialized convenience setters (these do not cost memory because of inlining)
bool communicationWriteAnalogInput(const uint8_t registerAddress, const uint8_t value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 1); }
bool communicationWriteAnalogInput(const uint8_t registerAddress, const uint16_t value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 2); }
bool communicationWriteAnalogInput(const uint8_t registerAddress, const uint32_t value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 4); }
bool communicationWriteAnalogInput(const uint8_t registerAddress, const int8_t value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 1); }
bool communicationWriteAnalogInput(const uint8_t registerAddress, const int16_t value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 2); }
bool communicationWriteAnalogInput(const uint8_t registerAddress, const int32_t value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 4); }
bool communicationWriteAnalogInput(const uint8_t registerAddress, const float value) { return communicationWriteAnalogRegister(false, registerAddress, &value, 4); }

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
void communicationReadAnalogInput(const uint8_t registerAddress, uint8_t &value) { communicationReadAnalogRegister(false, registerAddress, &value, 1); }
void communicationReadAnalogInput(const uint8_t registerAddress, uint16_t &value) { communicationReadAnalogRegister(false, registerAddress, &value, 2); }
void communicationReadAnalogInput(const uint8_t registerAddress, uint32_t &value) { communicationReadAnalogRegister(false, registerAddress, &value, 4); }
void communicationReadAnalogInput(const uint8_t registerAddress, int8_t &value) { communicationReadAnalogRegister(false, registerAddress, &value, 1); }
void communicationReadAnalogInput(const uint8_t registerAddress, int16_t &value) { communicationReadAnalogRegister(false, registerAddress, &value, 2); }
void communicationReadAnalogInput(const uint8_t registerAddress, int32_t &value) { communicationReadAnalogRegister(false, registerAddress, &value, 4); }
void communicationReadAnalogInput(const uint8_t registerAddress, float &value) { communicationReadAnalogRegister(false, registerAddress, &value, 4); }
#endif

// -----------------------------------------------------------------------------
// ANALOG OUTPUTS
// -----------------------------------------------------------------------------

#if COMMUNICATION_MAX_AO_REGISTER_SIZE
// Specialized convenience setters (these do not cost memory because of inlining)
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const uint8_t value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 1); }
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const uint16_t value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 2); }
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const uint32_t value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 4); }
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const int8_t value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 1); }
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const int16_t value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 2); }
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const int32_t value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 4); }
bool communicationWriteAnalogOutput(const uint8_t registerAddress, const float value) { return communicationWriteAnalogRegister(true, registerAddress, &value, 4); }

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
void communicationReadAnalogOutput(const uint8_t registerAddress, uint8_t &value) { communicationReadAnalogRegister(true, registerAddress, &value, 1); }
void communicationReadAnalogOutput(const uint8_t registerAddress, uint16_t &value) { communicationReadAnalogRegister(true, registerAddress, &value, 2); }
void communicationReadAnalogOutput(const uint8_t registerAddress, uint32_t &value) { communicationReadAnalogRegister(true, registerAddress, &value, 4); }
void communicationReadAnalogOutput(const uint8_t registerAddress, int8_t &value) { communicationReadAnalogRegister(true, registerAddress, &value, 1); }
void communicationReadAnalogOutput(const uint8_t registerAddress, int16_t &value) { communicationReadAnalogRegister(true, registerAddress, &value, 2); }
void communicationReadAnalogOutput(const uint8_t registerAddress, int32_t &value) { communicationReadAnalogRegister(true, registerAddress, &value, 4); }
void communicationReadAnalogOutput(const uint8_t registerAddress, float &value) { communicationReadAnalogRegister(true, registerAddress, &value, 4); }
#endif

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
