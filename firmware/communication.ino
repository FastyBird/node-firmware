/*

COMMUNICATION MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <PJON.h>
#include <EEPROM.h>

PJON<ThroughSerial> _communication_bus(PJON_NOT_ASSIGNED);

SoftwareSerial _communication_serial_bus(COMMUNICATION_BUS_TX_PIN, COMMUNICATION_BUS_RX_PIN);

communication_register_t _communication_register;

uint32_t _communication_last_node_search_request_time;
uint32_t _communication_last_new_node_search_request_time;
uint32_t _communication_master_last_request = 0;
bool _communication_master_lost = false;
bool _communication_address_confirmed = false;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

void _communicationSetAddress(
    const uint8_t address
) {
    _communication_bus.set_id(address);

    #if !COMMUNICATION_DISABLE_ADDRESS_STORING
        EEPROM.update(FLASH_ADDRESS_NODE_ADDRESS, address);
    #endif
}

// -----------------------------------------------------------------------------

bool _communicationIsPacketInGroup(
    const uint8_t packetId,
    const int * group,
    const uint8_t length
) {
    for (uint8_t i = 0; i < length; i++) {
        if ((uint8_t) pgm_read_byte(&group[i]) == packetId) {
            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

uint8_t _communicationGetPacketIndexInGroup(
    const uint8_t packetId,
    const int * group,
    const uint8_t length
) {
    for (uint8_t i = 0; i < length; i++) {
        if ((uint8_t) pgm_read_byte(&group[i]) == packetId) {
            return i;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------

#if DEBUG_SUPPORT
    void _communicationDebugLogPacket(
        const uint8_t packetId
    ) {
        DPRINT(packetId);
        DPRINT(F(" => ("));

        char buffer[50];

        if (_communicationIsPacketInGroup(packetId, communication_packets_addresing, COMMUNICATION_PACKET_ADDRESS_MAX)) {
            strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_addresing_string[_communicationGetPacketIndexInGroup(packetId, communication_packets_addresing, COMMUNICATION_PACKET_ADDRESS_MAX)]));

        } else if (_communicationIsPacketInGroup(packetId, communication_packets_node_initialization, COMMUNICATION_PACKET_NODE_INIT_MAX)) {
            strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_node_initialization_string[_communicationGetPacketIndexInGroup(packetId, communication_packets_node_initialization, COMMUNICATION_PACKET_NODE_INIT_MAX)]));

        } else if (_communicationIsPacketInGroup(packetId, communication_packets_registers_initialization, COMMUNICATION_PACKET_REGISTERS_INIT_MAX)) {
            strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_registers_initialization_string[_communicationGetPacketIndexInGroup(packetId, communication_packets_registers_initialization, COMMUNICATION_PACKET_REGISTERS_INIT_MAX)]));

        } else if (_communicationIsPacketInGroup(packetId, communication_packets_registers_reading, COMMUNICATION_PACKET_REGISTERS_REDING_MAX)) {
            strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_registers_reading_string[_communicationGetPacketIndexInGroup(packetId, communication_packets_registers_reading, COMMUNICATION_PACKET_REGISTERS_REDING_MAX)]));

        } else if (_communicationIsPacketInGroup(packetId, communication_packets_registers_writing, COMMUNICATION_PACKET_REGISTERS_WRITING_MAX)) {
            strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_registers_writing_string[_communicationGetPacketIndexInGroup(packetId, communication_packets_registers_writing, COMMUNICATION_PACKET_REGISTERS_WRITING_MAX)]));

        } else if (_communicationIsPacketInGroup(packetId, communication_packets_misc, COMMUNICATION_PACKET_MISC_MAX)) {
            strcpy_P(buffer, (char *) pgm_read_word(&communication_packets_misc_string[_communicationGetPacketIndexInGroup(packetId, communication_packets_misc, COMMUNICATION_PACKET_MISC_MAX)]));

        } else {
            strncpy_P(buffer, "unknown", sizeof(buffer));
        }

        DPRINT(buffer);
        DPRINTLN(F(")"));
    }
#endif

// -----------------------------------------------------------------------------

/**
 * Get DI or DO buffer size
 */
uint8_t _communicationGetDigitalBufferSize(
    const bool output
) {
    if (output) {
        return _communication_register.digital_outputs.size();
    }

    return _communication_register.digital_inputs.size();
}

// -----------------------------------------------------------------------------

/**
 * Get AI or AO buffer size
 */
uint8_t _communicationGetAnalogBufferSize(
    const bool output
) {
    if (output) {
        return _communication_register.analog_outputs.size();
    }

    return _communication_register.analog_inputs.size();
}

// -----------------------------------------------------------------------------
// DIGITAL REGISTERS
// -----------------------------------------------------------------------------

void _communicationReportSingleDigitalRegisters(
    const uint8_t packetId,
    uint8_t * payload,
    const bool output
) {
    char output_content[6];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from single digital"));
        if (output) {
            DPRINT(F(" output (DO) "));
        } else {
            DPRINT(F(" input (DI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINTLN(register_address);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        register_address < _communicationGetDigitalBufferSize(output)
    ) {
        word read_value = 0x0000;

        if (output) {
            read_value = communicationReadDigitalOutput(register_address) ? 0xFF00 : 0x0000;

        } else {
            read_value = communicationReadDigitalInput(register_address) ? 0xFF00 : 0x0000;
        }

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Register data type
        // 4-5  => Register value
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) (read_value >> 8);
        output_content[4] = (char) (read_value & 0xFF);

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 5) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive digital register reading\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with digital registers content\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, 5);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read from undefined digital registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationReportMultiDigitalRegisters(
    const uint8_t packetId,
    uint8_t * payload,
    const bool output
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word read_length = (word) payload[3] << 8 | (word) payload[4];
    
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from multiple digital"));
        if (output) {
            DPRINT(F(" outputs (DO) "));
        } else {
            DPRINT(F(" inputs (DI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINT(register_address);
        DPRINT(F(" and length: "));
        DPRINTLN(read_length);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        register_address < _communicationGetDigitalBufferSize(output)
        // Read length have to be same or smaller as registers size
        && (register_address + read_length) <= _communicationGetDigitalBufferSize(output)
    ) {
        bool byte_buffer[8];

        // Reset bit buffer
        for (uint8_t j = 0; j < 8; j++) {
            byte_buffer[j] = 0;
        }

        uint8_t bit_counter = 0;
        uint8_t byte_counter = 1;
        uint8_t write_byte = 0;

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of data bytes
        // 4-n  => Packet data
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) 0; // Temporary value, will be updated after collecting all

        uint8_t byte_pointer = 4;

        for (uint8_t i = register_address; i < (register_address + read_length) && i < _communicationGetDigitalBufferSize(output); i++) {
            if (output) {
                byte_buffer[bit_counter] = communicationReadDigitalOutput(i) ? 1 : 0;

            } else {
                byte_buffer[bit_counter] = communicationReadDigitalInput(i) ? 1 : 0;
            }

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
                output_content[byte_pointer] = write_byte;

                byte_counter++;
                byte_pointer++;

                // Reset bit buffer
                for (uint8_t j = 0; j < 8; j++) {
                    byte_buffer[j] = 0;
                }
            }
        }

        if (byte_counter == 1 || read_length % 8 != 0) {
            // Converting BIT array to BYTE => decimal number
            write_byte = 0;

            for (uint8_t wr = 0; wr < 8; wr++) {
                write_byte |= byte_buffer[wr] << wr;
            }

            // Write converted decimal number to output buffer
            output_content[byte_pointer] = write_byte;
        }

        // Update data bytes length
        output_content[3] = (char) byte_counter;

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, (byte_counter + 4)) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive digital register reading\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with digital registers content\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, (byte_counter + 4));
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read from undefined digital registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationWriteSingleDigitalOutput(
    const uint8_t packetId,
    uint8_t * payload
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word write_value = (word) payload[3] << 8 | (word) payload[4];

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested writing single value to DO register at address: "));
        DPRINTLN(register_address);
    #endif

    // Check if value is TRUE|FALSE or 1|0
    if (write_value != 0xFF00 && write_value != 0x0000) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] DO register accept only bool value\n"));
        #endif

        // TODO: Send exception

        return;
    }

    if (
        // Write address must be between <0, buffer.size()>
        register_address < _communicationGetDigitalBufferSize(true)
    ) {
        if (communicationReadDigitalOutput(register_address) != (bool) write_value) {
            communicationWriteDigitalOutput(register_address, (bool) write_value);
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION] Value was written into DO register\n"));

        } else {
            DPRINT(F("[COMMUNICATION] Value to write into DO register is same as stored. Write skipped\n"));
        #endif
        }

        // 0 => Packet identifier
        // 1 => High byte of register address
        // 2 => Low byte of register address
        // 3 => High byte of written value
        // 4 => Low byte of written value
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) (write_value >> 8);
        output_content[4] = (char) (write_value & 0xFF);

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 5) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive DO register write result\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with DO register write result\n"));
            }
        #else
            _communicationReplyToPacket(output_content, 5);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to write to undefined DO register address\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationWriteMultipleDigitalOutputs(
    const uint8_t packetId,
    uint8_t * payload
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word write_length = (word) payload[3] << 8 | (word) payload[4];

    uint8_t bytes_count = (uint8_t) payload[5];

    #if DEBUG_SUPPORT
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
                #if DEBUG_SUPPORT
                    DPRINT(F("[COMMUNICATION] Value was written into DO register at address: "));
                    DPRINTLN(write_address);

                } else {
                    DPRINT(F("[COMMUNICATION] Value to write into DO register at address: "));
                    DPRINT(write_address);
                    DPRINT(F(" is same as stored. Write skipped\n"));
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
        output_content[0] = packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) (write_byte >> 8);
        output_content[4] = (char) (write_byte & 0xFF);

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 5) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive multiple DO register write result\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with multiple DO register write result\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, 5);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to write to undefined DO registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------
// ANALOG REGISTERS
// -----------------------------------------------------------------------------

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
                communicationReadAnalogOutput(address, uint8_read_value.number);

            } else {
                communicationReadAnalogInput(address, uint8_read_value.number);
            }

            memcpy(value, uint8_read_value.bytes, 1);
            break;

        case COMMUNICATION_DATA_TYPE_UINT16:
            UINT16_UNION_t uint16_read_value;
            if (output) {
                communicationReadAnalogOutput(address, uint16_read_value.number);

            } else {
                communicationReadAnalogInput(address, uint16_read_value.number);
            }

            memcpy(value, uint16_read_value.bytes, 2);
            break;

        case COMMUNICATION_DATA_TYPE_UINT32:
            UINT32_UNION_t uint32_read_value;
            if (output) {
                communicationReadAnalogOutput(address, uint32_read_value.number);

            } else {
                communicationReadAnalogInput(address, uint32_read_value.number);
            }

            memcpy(value, uint32_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_INT8:
            INT8_UNION_t int8_read_value;
            if (output) {
                communicationReadAnalogOutput(address, int8_read_value.number);

            } else {
                communicationReadAnalogInput(address, int8_read_value.number);
            }

            memcpy(value, int8_read_value.bytes, 1);
            break;

        case COMMUNICATION_DATA_TYPE_INT16:
            INT16_UNION_t int16_read_value;
            if (output) {
                communicationReadAnalogOutput(address, int16_read_value.number);

            } else {
                communicationReadAnalogInput(address, int16_read_value.number);
            }

            memcpy(value, int16_read_value.bytes, 2);
            break;

        case COMMUNICATION_DATA_TYPE_INT32:
            INT32_UNION_t int32_read_value;
            if (output) {
                communicationReadAnalogOutput(address, int32_read_value.number);

            } else {
                communicationReadAnalogInput(address, int32_read_value.number);
            }

            memcpy(value, int32_read_value.bytes, 4);
            break;

        case COMMUNICATION_DATA_TYPE_FLOAT32:
            FLOAT32_UNION_t float_read_value;
            if (output) {
                communicationReadAnalogOutput(address, float_read_value.number);

            } else {
                communicationReadAnalogInput(address, float_read_value.number);
            }

            memcpy(value, float_read_value.bytes, 4);
            break;
    
        default:
            char buffer[4];

            buffer[0] = 0;
            buffer[1] = 0;
            buffer[2] = 0;
            buffer[3] = 0;

            memcpy(value, buffer, 4);
            break;
    }
}

// -----------------------------------------------------------------------------

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
            uint8_write_value.bytes[1] = value[1];
            uint8_write_value.bytes[2] = value[2];
            uint8_write_value.bytes[3] = value[3];

            communicationReadAnalogOutput(address, uint8_stored_value);

            if (uint8_stored_value != uint8_write_value.number) {
                communicationWriteAnalogOutput(address, uint8_write_value.number);
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_UINT16:
            uint16_t uint16_stored_value;
            UINT16_UNION_t uint16_write_value;

            uint16_write_value.bytes[0] = value[0];
            uint16_write_value.bytes[1] = value[1];
            uint16_write_value.bytes[2] = value[2];
            uint16_write_value.bytes[3] = value[3];

            communicationReadAnalogOutput(address, uint16_stored_value);

            if (uint16_stored_value != uint16_write_value.number) {
                communicationWriteAnalogOutput(address, uint16_write_value.number);
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
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
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_INT8:
            int8_t int8_stored_value;
            INT8_UNION_t int8_write_value;

            int8_write_value.bytes[0] = value[0];
            int8_write_value.bytes[1] = value[1];
            int8_write_value.bytes[2] = value[2];
            int8_write_value.bytes[3] = value[3];

            communicationReadAnalogOutput(address, int8_stored_value);

            if (int8_stored_value != int8_write_value.number) {
                communicationWriteAnalogOutput(address, int8_write_value.number);
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
            #endif
            }
            break;

        case COMMUNICATION_DATA_TYPE_INT16:
            int16_t int16_stored_value;
            INT16_UNION_t int16_write_value;

            int16_write_value.bytes[0] = value[0];
            int16_write_value.bytes[1] = value[1];
            int16_write_value.bytes[2] = value[2];
            int16_write_value.bytes[3] = value[3];

            communicationReadAnalogOutput(address, int16_stored_value);

            if (int16_stored_value != int16_write_value.number) {
                communicationWriteAnalogOutput(address, int16_write_value.number);
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
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
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
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
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION] Value was written into AO register\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Value to write into AO register is same as stored. Write skipped\n"));
            #endif
            }
            break;

        default:
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Provided unknown data type for writing into analog register\n"));
            #endif
            break;
    }
}

// -----------------------------------------------------------------------------

/**
 * PAYLOAD:
 * 0 => Packet identifier
 * 1 => High byte of register address
 * 2 => Low byte of register address
 */
void _communicationReportSingleAnalogRegisters(
    const uint8_t packetId,
    uint8_t * payload,
    const bool output
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from single analog"));
        if (output) {
            DPRINT(F(" output (AO) "));
        } else {
            DPRINT(F(" input (AI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINTLN(register_address);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        register_address < _communicationGetAnalogBufferSize(output)
    ) {
        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Register data type
        // 4-7  => Register value
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);

        char read_value[4] = { 0 };

        if (output) {
            _communicationReadAnalogForTransfer(output, _communication_register.analog_outputs[register_address].data_type, register_address, read_value);

            output_content[3] = (char) _communication_register.analog_outputs[register_address].data_type;

        } else {
            _communicationReadAnalogForTransfer(output, _communication_register.analog_inputs[register_address].data_type, register_address, read_value);

            output_content[3] = (char) _communication_register.analog_inputs[register_address].data_type;
        }

        output_content[4] = read_value[0];
        output_content[5] = read_value[1];
        output_content[6] = read_value[2];
        output_content[7] = read_value[3];

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 8) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive analog register reading\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with analog registers content\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, 8);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read from undefined analog registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationReportMultiAnalogRegisters(
    const uint8_t packetId,
    uint8_t * payload,
    const bool output
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word read_length = (word) payload[3] << 8 | (word) payload[4];
    
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from multiple analog"));
        if (output) {
            DPRINT(F(" outputs (AO) "));
        } else {
            DPRINT(F(" inputs (AI) "));
        }
        DPRINT(F("buffer at address: "));
        DPRINT(register_address);
        DPRINT(F(" and length: "));
        DPRINTLN(read_length);
    #endif

    if (
        // Read start address mus be between <0, buffer.size()>
        register_address < _communicationGetAnalogBufferSize(output)
        // Read length have to be same or smaller as registers size
        && (register_address + read_length) <= _communicationGetAnalogBufferSize(output)
    ) {
        uint8_t byte_counter = 0;

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of data bytes
        // 4-n  => Packet data
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) 0; // Temporary value, will be updated after collecting all

        uint8_t byte_pointer = 4;

        char read_value[4];

        for (uint8_t i = register_address; i < (register_address + read_length) && i < _communicationGetAnalogBufferSize(output); i++) {
            if (output) {
                _communicationReadAnalogForTransfer(output, _communication_register.analog_outputs[i].data_type, i, read_value);

            } else {
                _communicationReadAnalogForTransfer(output, _communication_register.analog_inputs[i].data_type, i, read_value);
            }

            output_content[byte_pointer] = read_value[0];
            byte_pointer++;
            output_content[byte_pointer] = read_value[1];
            byte_pointer++;
            output_content[byte_pointer] = read_value[2];
            byte_pointer++;
            output_content[byte_pointer] = read_value[3];
            byte_pointer++;

            byte_counter = byte_counter + 4;
        }

        // Update data bytes length
        output_content[3] = (char) byte_counter;

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, (byte_counter + 4)) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive analog register reading\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with analog registers content\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, (byte_counter + 4));
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read from undefined analog registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationWriteSingleAnalogOutput(
    const uint8_t packetId,
    uint8_t * payload
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested writing single value to AO register at address: "));
        DPRINTLN(register_address);
    #endif

    if (
        // Write address must be between <0, buffer.size()>
        register_address < _communicationGetAnalogBufferSize(true)
    ) {
        char write_value[4];

        write_value[0] = payload[3];
        write_value[1] = payload[4];
        write_value[2] = payload[5];
        write_value[3] = payload[6];

        _communicationWriteAnalogFromTransfer(_communication_register.analog_outputs[register_address].data_type, register_address, write_value);

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3-n  => Written value
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) payload[3];
        output_content[4] = (char) payload[4];
        output_content[5] = (char) payload[5];
        output_content[6] = (char) payload[6];

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 7) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive AO register write result\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with AO register write result\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, 7);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to write to undefined AO register address\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------
/*
void _communicationWriteMultipleAnalogOutputs(
    const uint8_t packetId,
    uint8_t * payload
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word write_length = (word) payload[3] << 8 | (word) payload[4];
    uint8_t bytes_count = (uint8_t) payload[5];

    #if DEBUG_SUPPORT
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
        output_content[0] = packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) (write_byte >> 8);
        output_content[4] = (char) (write_byte & 0xFF);

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 5) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive multiple AO register write result\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with multiple AO register write result\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, 5);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to write to undefined AO registers range\n"));
        #endif

        // TODO: Send exception
    }
}
*/
// -----------------------------------------------------------------------------
// NODE ADDRESSING PROCESS
// -----------------------------------------------------------------------------

/**
 * PAYLOAD:
 * 0 => Packet identifier
 */
void _communicationNodesSearchRequestHandler(
    uint8_t * payload,
    const uint16_t length
) {
    if (communicationHasAssignedAddress() && millis() - _communication_last_node_search_request_time > (COMMUNICATION_ADDRESSING_TIMEOUT * 1.125)) {
        char output_content[PJON_PACKET_MAX_LENGTH];

        // 0    => Packet identifier
        // 1    => Node bus address
        // 2    => Max packet size
        // 3    => Node SN length
        // 4-n  => Node parsed SN
        output_content[0] = (uint8_t) COMMUNICATION_PACKET_SEARCH_NODES;
        output_content[1] = (uint8_t) _communication_bus.device_id();
        output_content[2] = (uint8_t) PJON_PACKET_MAX_LENGTH;
        output_content[3] = (uint8_t) (strlen((char *) NODE_SERIAL_NO) + 1);

        uint8_t byte_pointer = 4;

        for (uint8_t i = 0; i < strlen((char *) NODE_SERIAL_NO); i++) {
            output_content[byte_pointer] = ((char *) NODE_SERIAL_NO)[i];

            byte_pointer++;
        }

        output_content[byte_pointer] = 0; // Be sure to set the null terminator!!!

        // Non blocking packet transfer
        uint16_t result = _communication_bus.send_packet_blocking(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            (byte_pointer + 1)
        );

        if (result == PJON_ACK) {
            _communication_last_node_search_request_time = millis();
        }
    }
}

// -----------------------------------------------------------------------------

/**
 * PAYLOAD:
 * 0 => Packet identifier
 */
void _communicationNewNodesSearchRequestHandler(
    uint8_t * payload,
    const uint16_t length
) {
    if (communicationHasAssignedAddress() == false && millis() - _communication_last_new_node_search_request_time > (COMMUNICATION_ADDRESSING_TIMEOUT * 1.125)) {
        char output_content[PJON_PACKET_MAX_LENGTH];

        // 0    => Packet identifier
        // 1    => Max packet size
        // 2    => Node SN length
        // 3-n  => Node parsed SN
        output_content[0] = (uint8_t) COMMUNICATION_PACKET_SEARCH_NEW_NODES;
        output_content[1] = (uint8_t) PJON_PACKET_MAX_LENGTH;
        output_content[2] = (uint8_t) (strlen((char *) NODE_SERIAL_NO) + 1);

        uint8_t byte_pointer = 3;

        for (uint8_t i = 0; i < strlen((char *) NODE_SERIAL_NO); i++) {
            output_content[byte_pointer] = ((char *) NODE_SERIAL_NO)[i];

            byte_pointer++;
        }

        output_content[byte_pointer] = 0; // Be sure to set the null terminator!!!

        // Non blocking packet transfer
        uint16_t result = _communication_bus.send_packet_blocking(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            (byte_pointer + 1)
        );

        if (result == PJON_ACK) {
            _communication_last_new_node_search_request_time = millis();
        }
    }
}

// -----------------------------------------------------------------------------

/**
 * PAYLOAD:
 * 0    => Packet identifier
 * 1    => Gateway assigned address
 * 2    => Node SN length
 * 3-n  => Node SN
 */
void _communicationAddressConfirmRequestHandler(
    uint8_t * payload,
    const uint16_t length
) {
    // Check for correct received payload length
    if (length < (uint8_t) 4 || length != (uint8_t) (3 + (uint8_t) payload[2])) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Packet length is not correct\n"));
        #endif

        // TODO: reply with exception
        return;
    }

    // Extract address assigned by gateway
    uint8_t address = (uint8_t) payload[1];

    char node_sn[(uint8_t) payload[2]];

    // Extract node serial number from payload
    for (uint8_t i = 0; i < (uint8_t) payload[2]; i++) {
        node_sn[i] = (char) payload[i + 3];
    }

    // Check if received packet if for this node
    if (strcmp((char *) NODE_SERIAL_NO, (char *) node_sn) != 0) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][INFO] Packet is for other node: \""));
            DPRINT(node_sn);
            DPRINTLN(F("\""));
            DPRINT(F("[COMMUNICATION][INFO] Node SN is: \""));
            DPRINT((char *) NODE_SERIAL_NO);
            DPRINTLN(F("\""));
        #endif

        return;
    }

    // Set node address to received from gateway
    _communicationSetAddress(address);
    
    _communication_address_confirmed = true;

    #if DEBUG_SUPPORT
        DPRINTLN();
        DPRINT(F("[COMMUNICATION] ===========================\n"));
        DPRINT(F("[COMMUNICATION] Node address was set to: "));
        DPRINTLN(address);
        DPRINT(F("[COMMUNICATION] ===========================\n"));
        DPRINTLN();

        DPRINT(F("[COMMUNICATION] Confirming node address to master\n"));
    #endif

    char output_content[PJON_PACKET_MAX_LENGTH];

    // 0    => Packet identifier
    // 1    => Gateway assigned address
    // 2    => Node SN length
    // 3-n  => Node SN
    output_content[0] = (uint8_t) COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM;
    output_content[1] = (uint8_t) address;
    output_content[2] = (uint8_t) (strlen((char *) NODE_SERIAL_NO) + 1);

    uint8_t byte_pointer = 3;

    for (uint8_t i = 0; i < strlen((char *) NODE_SERIAL_NO); i++) {
        output_content[byte_pointer] = ((char *) NODE_SERIAL_NO)[i];

        byte_pointer++;
    }

    output_content[byte_pointer] = 0; // Be sure to set the null terminator!!!

    _communicationReplyToPacket(
        output_content,
        (byte_pointer + 1)
    );

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Received node address was successfully confirmed to gateway\n"));
    #endif
}

// -----------------------------------------------------------------------------

void _communicationAddressRequestHandler(
    const uint8_t packetId,
    uint8_t * payload,
    const uint16_t length
) {
    switch (packetId)
    {
        /**
         * Gateway is searching for nodes
         */
        case COMMUNICATION_PACKET_SEARCH_NODES:
            _communicationNodesSearchRequestHandler(payload, length);
            break;

        /**
         * Gateway is searching for not addressed nodes
         */
        case COMMUNICATION_PACKET_SEARCH_NEW_NODES:
            _communicationNewNodesSearchRequestHandler(payload, length);
            break;

        /**
         * Gateway provided node address
         */
        case COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM:
            _communicationAddressConfirmRequestHandler(payload, length);
            break;

        /**
         * Gateway confirmed node address discarding request
         */
        case COMMUNICATION_PACKET_ADDRESS_DISCARD:
            // TODO
            break;
    }
}

// -----------------------------------------------------------------------------
// NODE INITIALIZATION
// -----------------------------------------------------------------------------

void _communicationReportDescriptionRequestHandler(
    const uint8_t packetId,
    const char * sendContent
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    // 0    => Packet identifier
    // 1    => String length
    // 2-n  => String content
    output_content[0] = packetId;
    output_content[1] = (strlen(sendContent) + 1);

    uint8_t byte_pointer = 2;

    for (uint8_t i = 0; i < strlen(sendContent); i++) {
        output_content[byte_pointer] = sendContent[i];

        byte_pointer++;
    }

    output_content[byte_pointer] = 0; // Be sure to set the null terminator!!!

    #if DEBUG_SUPPORT
        // Reply to gateway
        if (_communicationReplyToPacket(output_content, (byte_pointer + 1)) == false) {
            // Node was not able to notify gateway about its address
            DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node description packet: "));
            DPRINT(packetId);
            DPRINT(F(" with content: "));
            DPRINTLN(sendContent);

        } else {
            DPRINT(F("[COMMUNICATION] Replied to gateway with description packet: "));
            DPRINT(packetId);
            DPRINT(F(" with content: "));
            DPRINTLN(sendContent);
        }
    #else
        // Reply to gateway
        _communicationReplyToPacket(output_content, (byte_pointer + 1));
    #endif
}

// -----------------------------------------------------------------------------

void _communicationNodeInitializationRequestHandler(
    const uint8_t packetId
) {
    switch (packetId)
    {
        case COMMUNICATION_PACKET_HW_MODEL:
            _communicationReportDescriptionRequestHandler(packetId, (char *) NODE_NAME);
            return;

        case COMMUNICATION_PACKET_HW_MANUFACTURER:
            _communicationReportDescriptionRequestHandler(packetId, (char *) NODE_MANUFACTURER);
            return;

        case COMMUNICATION_PACKET_HW_VERSION:
            _communicationReportDescriptionRequestHandler(packetId, (char *) NODE_VERSION);
            return;

        case COMMUNICATION_PACKET_FW_MODEL:
            _communicationReportDescriptionRequestHandler(packetId, (char *) FIRMWARE_NAME);
            return;

        case COMMUNICATION_PACKET_FW_MANUFACTURER:
            _communicationReportDescriptionRequestHandler(packetId, (char *) FIRMWARE_MANUFACTURER);
            return;

        case COMMUNICATION_PACKET_FW_VERSION:
            _communicationReportDescriptionRequestHandler(packetId, (char *) FIRMWARE_VERSION);
            return;
    }
}

// -----------------------------------------------------------------------------
// REGISTERS INITIALIZATION
// -----------------------------------------------------------------------------

void _communicationReportRegistersSizes(
    const uint8_t packetId
) {
    char output_content[5];

    // 0 => Packet identifier
    // 1 => DI buffer size
    // 2 => DO buffer size
    // 3 => AI buffer size
    // 4 => AO buffer size
    output_content[0] = (char) packetId;
    output_content[1] = (char) _communicationGetDigitalBufferSize(false);
    output_content[2] = (char) _communicationGetDigitalBufferSize(true);
    output_content[3] = (char) _communicationGetAnalogBufferSize(false);
    output_content[4] = (char) _communicationGetAnalogBufferSize(true);

    #if DEBUG_SUPPORT
        // Reply to gateway
        if (_communicationReplyToPacket(output_content, 5) == false) {
            // Node was not able to notify gateway about its address
            DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node registers sizes\n"));

        } else {
            DPRINT(F("[COMMUNICATION] Replied to gateway with registers sizes\n"));
        }
    #else
        // Reply to gateway
        _communicationReplyToPacket(output_content, 5);
    #endif
}

// -----------------------------------------------------------------------------

/**
 * PAYLOAD:
 * 0 => Packet identifier
 * 1 => High byte of register address
 * 2 => Low byte of register address
 * 3 => High byte of registers length
 * 4 => Low byte of registers length
 */
void _communicationReportAnalogRegisterStructure(
    const uint8_t packetId,
    uint8_t * payload,
    const uint16_t length,
    const bool output
) {
    // Check for correct received payload length
    if (length != (uint8_t) 5) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Packet length is not correct\n"));
        #endif

        // TODO: reply with exception
        return;
    }

    // Register read start address
    word register_address = (word) payload[1] << 8 | (word) payload[2];

    // Number of registers to read
    word read_length = (word) payload[3] << 8 | (word) payload[4];

    if (
        // Read start address mus be between <0, buffer.size()>
        register_address < _communicationGetAnalogBufferSize(output)
        // Read length have to be same or smaller as registers size
        && (register_address + read_length) <= _communicationGetAnalogBufferSize(output)
    ) {
        char output_content[PJON_PACKET_MAX_LENGTH];

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Register length
        // 4-n  => Register data type
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) 0; // Temporary value, will be updated after collecting all

        uint8_t byte_pointer = 4;
        uint8_t byte_counter = 0;

        for (uint8_t i = register_address; i < (register_address + read_length) && i < _communicationGetAnalogBufferSize(output); i++) {
            if (output) {
                output_content[byte_pointer] = _communication_register.analog_outputs[i].data_type;

            } else {
                output_content[byte_pointer] = _communication_register.analog_inputs[i].data_type;
            }

            byte_pointer++;
            byte_counter++;
        }

        // Update data bytes length
        output_content[3] = (char) byte_counter;

        #if DEBUG_SUPPORT
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, byte_pointer) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node analog registers structure\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with analog registers structure\n"));
            }
        #else
            // Reply to gateway
            _communicationReplyToPacket(output_content, byte_pointer);
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read structure for undefined analog registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationRegisterInitializationRequestHandler(
    const uint8_t packetId,
    uint8_t * payload,
    const uint16_t length
) {
    switch (packetId)
    {
        case COMMUNICATION_PACKET_REGISTERS_SIZE:
            _communicationReportRegistersSizes(packetId);
            break;

        case COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE:
            _communicationReportAnalogRegisterStructure(packetId, payload, length, false);
            break;

        case COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE:
            _communicationReportAnalogRegisterStructure(packetId, payload, length, true);
            break;
    }
}

// -----------------------------------------------------------------------------
// COMMUNICATION HANDLERS
// -----------------------------------------------------------------------------

void _communicationReceiverHandler(
    uint8_t * payload,
    const uint16_t length,
    const PJON_Packet_Info &packetInfo
) {
    #if DEBUG_SUPPORT
        DPRINTLN();
        DPRINT(F("[COMMUNICATION] ===============\n"));
        DPRINT(F("[COMMUNICATION] Received packet\n"));
        DPRINT(F("[COMMUNICATION] ===============\n"));
    #endif

    if (length <= 0) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Received packet is without content\n"));
        #endif

        return;
    }

    // Packed ID must be on first byte
    uint8_t packet_id = (uint8_t) payload[0];

    if (packet_id == COMMUNICATION_PACKET_NONE) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Unknown packet\n"));
        #endif

        return;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Received packet: "));
        _communicationDebugLogPacket(packet_id);
    #endif

    uint8_t sender_address = PJON_NOT_ASSIGNED;

    // Get sender address from header
    if (packetInfo.header & PJON_TX_INFO_BIT) {
        sender_address = packetInfo.sender_id;
    }
    
    // Only packets from gateway are accepted
    if (sender_address != COMMUNICATION_BUS_GATEWAY_ADDR) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Received packet from unknown gateway address: "));
            DPRINTLN(sender_address);
        #endif

        return;
    }

    // Reset master lost detection
    _communication_master_lost = false;
    _communication_master_last_request = millis();
        
    // Trying to get node address from gateway
    if (_communicationIsPacketInGroup(packet_id, communication_packets_addresing, COMMUNICATION_PACKET_ADDRESS_MAX)) {
        _communicationAddressRequestHandler(packet_id, payload, length);

    } else if (_communicationIsPacketInGroup(packet_id, communication_packets_node_initialization, COMMUNICATION_PACKET_NODE_INIT_MAX)) {
        _communicationNodeInitializationRequestHandler(packet_id);

    } else if (_communicationIsPacketInGroup(packet_id, communication_packets_registers_initialization, COMMUNICATION_PACKET_REGISTERS_INIT_MAX)) {
        _communicationRegisterInitializationRequestHandler(packet_id, payload, length);

    // Regular gateway messages
    } else {
        switch (packet_id)
        {
            case COMMUNICATION_PACKET_GATEWAY_PING:
                // Nothing to do, gateway is just testing connection
                break;
        
        /**
         * REGISTERS READING
         */

            case COMMUNICATION_PACKET_READ_SINGLE_DI:
                _communicationReportSingleDigitalRegisters(packet_id, payload, false);
                break;

            case COMMUNICATION_PACKET_READ_MULTI_DI:
                _communicationReportMultiDigitalRegisters(packet_id, payload, false);
                break;

            case COMMUNICATION_PACKET_READ_SINGLE_DO:
                _communicationReportSingleDigitalRegisters(packet_id, payload, true);
                break;

            case COMMUNICATION_PACKET_READ_MULTI_DO:
                _communicationReportMultiDigitalRegisters(packet_id, payload, true);
                break;
        
            case COMMUNICATION_PACKET_READ_SINGLE_AI:
                _communicationReportSingleAnalogRegisters(packet_id, payload, false);
                break;

            case COMMUNICATION_PACKET_READ_MULTI_AI:
                _communicationReportMultiAnalogRegisters(packet_id, payload, false);
                break;
        
            case COMMUNICATION_PACKET_READ_SINGLE_AO:
                _communicationReportSingleAnalogRegisters(packet_id, payload, true);
                break;

            case COMMUNICATION_PACKET_READ_MULTI_AO:
                _communicationReportMultiAnalogRegisters(packet_id, payload, true);
                break;
        
        /**
         * REGISTERS WRITING
         */
        
            case COMMUNICATION_PACKET_WRITE_ONE_DO:
                _communicationWriteSingleDigitalOutput(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_WRITE_ONE_AO:
                _communicationWriteSingleAnalogOutput(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_WRITE_MULTI_DO:
                _communicationWriteMultipleDigitalOutputs(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_WRITE_MULTI_AO:
                //_communicationWriteMultipleAnalogOutputs(packet_id, payload);
                break;
        }
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] ========================\n"));
        DPRINT(F("[COMMUNICATION] Handling packet finished\n"));
        DPRINT(F("[COMMUNICATION] ========================\n"));
        DPRINTLN();
    #endif
}

// -----------------------------------------------------------------------------

void _communicationErrorHandler(
    const uint8_t code,
    const uint16_t data,
    void *customPointer
) {
    #if DEBUG_SUPPORT
        if (code == PJON_CONNECTION_LOST) {
            _communication_master_lost = true;

            DPRINT(F("[COMMUNICATION][ERR] Connection lost with gateway\n"));

        } else if (code == PJON_PACKETS_BUFFER_FULL) {
            DPRINT(F("[COMMUNICATION][ERR] Buffer is full\n"));

        } else if (code == PJON_CONTENT_TOO_LONG) {
            DPRINT(F("[COMMUNICATION][ERR] Content is long\n"));

        } else {
            DPRINT(F("[COMMUNICATION][ERR] Unknown error\n"));
        }
    #endif
}

// -----------------------------------------------------------------------------

bool _communicationReplyToPacket(
    char * payload,
    const uint8_t size
) {
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Preparing reply packet: "));
        _communicationDebugLogPacket(payload[0]);
    #endif

    uint16_t result = _communication_bus.reply(
        payload,    // Content
        size        // Content lenght
    );

    if (result == PJON_FAIL) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION] Sending replypacket failed\n"));
        #endif

        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

bool communicationDiscardAddress()
{
    char output_content[6];

    output_content[0] = COMMUNICATION_PACKET_ADDRESS_DISCARD;
    output_content[1] = _communication_bus.device_id();

    if (
        _communication_bus.send_packet_blocking(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            2
        ) == PJON_ACK
    ) {
        _communicationSetAddress(PJON_NOT_ASSIGNED);

        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION] Node address was successfully reseted\n"));
        #endif

        return true;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION][ERR] Node address could not be discarded\n"));
    #endif

    return false;
}

// -----------------------------------------------------------------------------

bool communicationHasAssignedAddress() {
    return _communication_bus.device_id() != PJON_NOT_ASSIGNED;
}

// -----------------------------------------------------------------------------

bool communicationIsMasterLost() {
    return ((millis() - _communication_master_last_request) > COMMUNICATION_MASTER_PING_TIMEOUT || _communication_master_lost);
}

// -----------------------------------------------------------------------------
// DIGITAL INPUTS
// -----------------------------------------------------------------------------

uint8_t communicationRegisterDigitalInput()
{
    return communicationRegisterDigitalInput(false);
}

// -----------------------------------------------------------------------------

uint8_t communicationRegisterDigitalInput(
    bool defaultValue
) {
    _communication_register.digital_inputs.push_back((communication_binary_register_t) {
        defaultValue
    });

    return (_communication_register.digital_inputs.size() - 1);
}

// -----------------------------------------------------------------------------

bool communicationWriteDigitalInput(
    const uint8_t address,
    const bool value
) {
    if (address > _communication_register.digital_inputs.size()) {
        return false;
    }

    _communication_register.digital_inputs[address].value = value;

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalInput(
    const uint8_t address
) {
    if (address > _communication_register.digital_inputs.size()) {
        return false;
    }

    return _communication_register.digital_inputs[address].value;
}

// -----------------------------------------------------------------------------
// DIGITAL OUTPUTS
// -----------------------------------------------------------------------------

uint8_t communicationRegisterDigitalOutput()
{
    return communicationRegisterDigitalOutput(false);
}

// -----------------------------------------------------------------------------

uint8_t communicationRegisterDigitalOutput(
    const bool defaultValue
) {
    _communication_register.digital_outputs.push_back((communication_binary_register_t) {
        defaultValue
    });

    return (_communication_register.digital_outputs.size() - 1);
}

// -----------------------------------------------------------------------------

bool communicationWriteDigitalOutput(
    const uint8_t address,
    const bool value
) {
    if (address > _communication_register.digital_outputs.size()) {
        return false;
    }

    _communication_register.digital_outputs[address].value = value;

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalOutput(
    const uint8_t address
) {
    if (address > _communication_register.digital_outputs.size()) {
        return false;
    }

    return _communication_register.digital_outputs[address].value;
}

// -----------------------------------------------------------------------------
// ANALOG REGISTERS
// -----------------------------------------------------------------------------

void communicationReadAnalogRegister(
    const bool output,
    const uint8_t address,
    void * value,
    const uint8_t size
) {
    if (output) {
        if (
            address < _communication_register.analog_outputs.size()
            && _communication_register.analog_outputs[address].size == size
        ) {
            memcpy(value, _communication_register.analog_outputs[address].value, size);

            return;
        }

    } else {
        if (
            address < _communication_register.analog_inputs.size()
            && _communication_register.analog_inputs[address].size == size
        ) {
            memcpy(value, _communication_register.analog_inputs[address].value, size);

            return;
        }
    }

    char default_value[4] = { 0 };

    memcpy(value, default_value, size);
}

// -----------------------------------------------------------------------------

bool communicationWriteAnalogRegister(
    const bool output,
    const uint8_t address,
    const void * value,
    const uint8_t size
) {
    if (output) {
        if (address > _communication_register.analog_outputs.size()) {
            return false;
        }

        if (_communication_register.analog_outputs[address].size == size) {
            memcpy(_communication_register.analog_outputs[address].value, value, size);
        }

    } else {
        if (address > _communication_register.analog_inputs.size()) {
            return false;
        }

        if (_communication_register.analog_inputs[address].size == size) {
            memcpy(_communication_register.analog_inputs[address].value, value, size);
        }
    }

    return true;
}

// -----------------------------------------------------------------------------
// ANALOG INPUTS
// -----------------------------------------------------------------------------

uint8_t communicationRegisterAnalogInput(
    const uint8_t dataType
) {
    _communication_register.analog_inputs.push_back((communication_analog_register_t) {
        dataType
    });

    uint8_t address = (_communication_register.analog_inputs.size() - 1);

    memset(_communication_register.analog_inputs[address].value, 0, 4);

    if (dataType == COMMUNICATION_DATA_TYPE_UINT8 || dataType == COMMUNICATION_DATA_TYPE_INT8) {
        _communication_register.analog_inputs[address].size = 1;

    } else if (dataType == COMMUNICATION_DATA_TYPE_UINT16 || dataType == COMMUNICATION_DATA_TYPE_INT16) {
        _communication_register.analog_inputs[address].size = 2;

    } else if (dataType == COMMUNICATION_DATA_TYPE_UINT32 || dataType == COMMUNICATION_DATA_TYPE_INT32 || dataType == COMMUNICATION_DATA_TYPE_FLOAT32) {
        _communication_register.analog_inputs[address].size = 4;
    }

    return address;
}

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
bool communicationWriteAnalogInput(const uint8_t address, const uint8_t value) { return communicationWriteAnalogRegister(false, address, &value, 1); }
bool communicationWriteAnalogInput(const uint8_t address, const uint16_t value) { return communicationWriteAnalogRegister(false, address, &value, 2); }
bool communicationWriteAnalogInput(const uint8_t address, const uint32_t value) { return communicationWriteAnalogRegister(false, address, &value, 4); }
bool communicationWriteAnalogInput(const uint8_t address, const int8_t value) { return communicationWriteAnalogRegister(false, address, &value, 1); }
bool communicationWriteAnalogInput(const uint8_t address, const int16_t value) { return communicationWriteAnalogRegister(false, address, &value, 2); }
bool communicationWriteAnalogInput(const uint8_t address, const int32_t value) { return communicationWriteAnalogRegister(false, address, &value, 4); }
bool communicationWriteAnalogInput(const uint8_t address, const float value) { return communicationWriteAnalogRegister(false, address, &value, 4); }

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
void communicationReadAnalogInput(const uint8_t address, uint8_t &value) { communicationReadAnalogRegister(false, address, &value, 1); }
void communicationReadAnalogInput(const uint8_t address, uint16_t &value) { communicationReadAnalogRegister(false, address, &value, 2); }
void communicationReadAnalogInput(const uint8_t address, uint32_t &value) { communicationReadAnalogRegister(false, address, &value, 4); }
void communicationReadAnalogInput(const uint8_t address, int8_t &value) { communicationReadAnalogRegister(false, address, &value, 1); }
void communicationReadAnalogInput(const uint8_t address, int16_t &value) { communicationReadAnalogRegister(false, address, &value, 2); }
void communicationReadAnalogInput(const uint8_t address, int32_t &value) { communicationReadAnalogRegister(false, address, &value, 4); }
void communicationReadAnalogInput(const uint8_t address, float &value) { communicationReadAnalogRegister(false, address, &value, 4); }

// -----------------------------------------------------------------------------
// ANALOG OUTPUTS
// -----------------------------------------------------------------------------

uint8_t communicationRegisterAnalogOutput(
    const uint8_t dataType
) {
    _communication_register.analog_outputs.push_back((communication_analog_register_t) {
        dataType
    });

    uint8_t address = (_communication_register.analog_outputs.size() - 1);

    memset(_communication_register.analog_outputs[address].value, 0, 4);

    if (dataType == COMMUNICATION_DATA_TYPE_UINT8 || dataType == COMMUNICATION_DATA_TYPE_INT8) {
        _communication_register.analog_outputs[address].size = 1;

    } else if (dataType == COMMUNICATION_DATA_TYPE_UINT16 || dataType == COMMUNICATION_DATA_TYPE_INT16) {
        _communication_register.analog_outputs[address].size = 2;

    } else if (dataType == COMMUNICATION_DATA_TYPE_UINT32 || dataType == COMMUNICATION_DATA_TYPE_INT32 || dataType == COMMUNICATION_DATA_TYPE_FLOAT32) {
        _communication_register.analog_outputs[address].size = 4;
    }

    return address;
}

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
bool communicationWriteAnalogOutput(const uint8_t address, const uint8_t value) { return communicationWriteAnalogRegister(true, address, &value, 1); }
bool communicationWriteAnalogOutput(const uint8_t address, const uint16_t value) { return communicationWriteAnalogRegister(true, address, &value, 2); }
bool communicationWriteAnalogOutput(const uint8_t address, const uint32_t value) { return communicationWriteAnalogRegister(true, address, &value, 4); }
bool communicationWriteAnalogOutput(const uint8_t address, const int8_t value) { return communicationWriteAnalogRegister(true, address, &value, 1); }
bool communicationWriteAnalogOutput(const uint8_t address, const int16_t value) { return communicationWriteAnalogRegister(true, address, &value, 2); }
bool communicationWriteAnalogOutput(const uint8_t address, const int32_t value) { return communicationWriteAnalogRegister(true, address, &value, 4); }
bool communicationWriteAnalogOutput(const uint8_t address, const float value) { return communicationWriteAnalogRegister(true, address, &value, 4); }

// -----------------------------------------------------------------------------

// Specialized convenience setters (these do not cost memory because of inlining)
void communicationReadAnalogOutput(const uint8_t address, uint8_t &value) { communicationReadAnalogRegister(true, address, &value, 1); }
void communicationReadAnalogOutput(const uint8_t address, uint16_t &value) { communicationReadAnalogRegister(true, address, &value, 2); }
void communicationReadAnalogOutput(const uint8_t address, uint32_t &value) { communicationReadAnalogRegister(true, address, &value, 4); }
void communicationReadAnalogOutput(const uint8_t address, int8_t &value) { communicationReadAnalogRegister(true, address, &value, 1); }
void communicationReadAnalogOutput(const uint8_t address, int16_t &value) { communicationReadAnalogRegister(true, address, &value, 2); }
void communicationReadAnalogOutput(const uint8_t address, int32_t &value) { communicationReadAnalogRegister(true, address, &value, 4); }
void communicationReadAnalogOutput(const uint8_t address, float &value) { communicationReadAnalogRegister(true, address, &value, 4); }

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void communicationSetup() {
    _communication_serial_bus.begin(SERIAL_BAUDRATE);

    _communication_bus.strategy.set_serial(&_communication_serial_bus);

    // Communication callbacks
    _communication_bus.set_receiver(_communicationReceiverHandler);
    _communication_bus.set_error(_communicationErrorHandler);

    _communication_bus.begin();

    #if COMMUNICATION_DISABLE_ADDRESS_STORING
        uint8_t node_address = PJON_NOT_ASSIGNED;
    #else
        uint8_t node_address = (uint8_t) EEPROM.read(FLASH_ADDRESS_NODE_ADDRESS);

    #endif

    #if DEBUG_SUPPORT
        if (node_address == PJON_NOT_ASSIGNED) {
            DPRINTLN(F("[COMMUNICATION] Unaddressed node"));

        } else {
            DPRINT(F("[COMMUNICATION] Stored node address: "));
            DPRINTLN(node_address);
        }
    #endif

    if (node_address != PJON_NOT_ASSIGNED && node_address > 0 && node_address < 250) {
        _communication_bus.set_id(node_address);
    }
}

// -----------------------------------------------------------------------------

void communicationLoop() {
    // -------------------------------------------------------------------------
    // Bus communication
    // -------------------------------------------------------------------------
    _communication_bus.update();
    _communication_bus.receive();
}
