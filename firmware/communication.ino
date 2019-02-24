/*

COMMUNICATION MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <PJON.h>
#include <Vector.h>

PJON<ThroughSerial> _communication_bus(PJON_NOT_ASSIGNED);

SoftwareSerial _communication_serial_bus(COMMUNICATION_BUS_TX_PIN, COMMUNICATION_BUS_RX_PIN);

typedef struct {
    byte address;
    bool value;
} communication_binary_register_t;

typedef struct {
    byte address;
    word value;
} communication_analog_register_t;

struct communication_register_t {
    Vector<communication_binary_register_t> digital_inputs;
    Vector<communication_binary_register_t> digital_outputs;

    Vector<communication_analog_register_t> analog_inputs;
    Vector<communication_analog_register_t> analog_outputs;
};

communication_register_t _communication_register;

uint32_t _communication_rid;
uint32_t _communication_last_rid_request_time;
uint32_t _communication_master_lost = 0;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

#if DEBUG_SUPPORT
    void _communicationDebugLogPacket(
        byte packetId
    ) {
        DPRINT(packetId);
        DPRINT(F(" => ("));

        if (packetId < COMMUNICATION_PACKETS_MAX) {
            char buffer[50];

            strcpy_P(buffer, (char *) pgm_read_word(&communication_packet_string[packetId]));
            DPRINT(buffer);

        } else {
            DPRINTLN(F("unknown"));
        }

        DPRINTLN(F(")"));
    }

// -----------------------------------------------------------------------------

    void _communicationDebugLogNodeAddressAcquire(
        byte index
    ) {
        DPRINT(index);
        DPRINT(F(" => ("));

        if (index < COMMUNICATION_ACQUIRE_ADDRESS_MAX) {
            char buffer[50];

            strcpy_P(buffer, (char *) pgm_read_word(&communication_address_acquire_string[index]));
            DPRINT(buffer);

        } else {
            DPRINTLN(F("unknown"));
        }

        DPRINTLN(F(")"));
    }

// -----------------------------------------------------------------------------

    void _communicationDebugLogNodeDescription(
        byte description
    ) {
        DPRINT(description);
        DPRINT(F(" => ("));

        if (description < COMMUNICATION_DESCRIBES_MAX) {
            char buffer[50];

            strcpy_P(buffer, (char *) pgm_read_word(&communication_describe_string[description]));
            DPRINT(buffer);

        } else {
            DPRINTLN(F("unknown"));
        }

        DPRINTLN(F(")"));
    }
#endif

// -----------------------------------------------------------------------------

/**
 * Get DI or DO buffer size
 */
byte _communicationGetDigitalBufferSize(
    bool output
) {
    if (output) {
        return _communication_register.digital_outputs.size();
    }

    return _communication_register.digital_inputs.size();
}

// -----------------------------------------------------------------------------

/**
 * Get value from DI or DO buffer
 */
bool _communicationGetDigitalBufferValue(
    bool output,
    byte index
) {
    if (output) {
        if (index < _communication_register.digital_outputs.size()) {
            return _communication_register.digital_outputs[index].value;

        } else {
            return false;
        }
    }

    if (index < _communication_register.digital_inputs.size()) {
        return _communication_register.digital_inputs[index].value;
    }

    return false;
}

// -----------------------------------------------------------------------------

/**
 * Get AI or AO buffer size
 */
byte _communicationGetAnalogBufferSize(
    bool output
) {
    if (output) {
        return _communication_register.analog_outputs.size();
    }

    return _communication_register.analog_inputs.size();
}

// -----------------------------------------------------------------------------

/**
 * Get value from AI or AO buffer
 */
word _communicationGetAnalogBufferValue(
    bool output,
    byte index
) {
    if (output) {
        if (index < _communication_register.analog_outputs.size()) {
            return _communication_register.analog_outputs[index].value;

        } else {
            return (word) 0;
        }
    }

    if (index < _communication_register.analog_inputs.size()) {
        return _communication_register.analog_inputs[index].value;
    }

    return (word) 0;
}

// -----------------------------------------------------------------------------

void _communicationReportBasicStructure(
    byte packetId
) {
    char output_content[11];

    // 0    => Packet identifier
    // 1    => Subaction identifier
    // 2    => Supported SN info
    // 3    => Supported HW model
    // 4    => Supported HW manufacturer
    // 5    => Supported HW version
    // 6    => Supported FW model
    // 7    => Supported FW manufacturer
    // 8    => Supported FW version
    // 9    => Supported value registers
    // 10   => Max packet size
    output_content[0] = (char) COMMUNICATION_PACKET_WHO_ARE_YOU;
    output_content[1] = (char) COMMUNICATION_DESCRIBE_NODE;
    output_content[2] = 0xFF;
    output_content[3] = 0xFF;
    output_content[4] = 0xFF;
    output_content[5] = 0xFF;
    output_content[6] = 0xFF;
    output_content[7] = 0xFF;
    output_content[8] = 0xFF;
    output_content[9] = 0xFF;
    output_content[10] = PJON_PACKET_MAX_LENGTH;

    #if DEBUG_SUPPORT
        // Reply to gateway
        _communicationReplyToPacket(output_content, 11);
    #else
        // Reply to gateway
        if (_communicationReplyToPacket(output_content, 11) == false) {
            // Node was not able to notify gateway about its address
            DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node basic structure\n"));

        } else {
            DPRINT(F("[COMMUNICATION] Replied to gateway with basic sctructure info\n"));
        }
    #endif
}

// -----------------------------------------------------------------------------

void _communicationReportDescription(
    byte packetId,
    byte describeType,
    char * sendContent
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    // 0    => Packet identifier
    // 1    => Subaction identifier
    // 2    => String length
    // 3-n  => String content
    output_content[0] = (char) COMMUNICATION_PACKET_WHO_ARE_YOU;
    output_content[1] = (char) describeType;
    output_content[2] = (strlen(sendContent) + 1);

    byte bytes_counter = 3;

    for (byte i = 0; i < strlen(sendContent); i++) {
        output_content[bytes_counter] = sendContent[i];

        bytes_counter++;
    }

    output_content[bytes_counter] = 0; // Be sure to set the null terminator!!!

    #if DEBUG_SUPPORT
        // Reply to gateway
        _communicationReplyToPacket(output_content, (bytes_counter + 1));
    #else
        // Reply to gateway
        if (_communicationReplyToPacket(output_content, (bytes_counter + 1)) == false) {
            // Node was not able to notify gateway about its address
            DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node description: "));
            DPRINT(describeType);
            DPRINT(F(" with content: "));
            DPRINTLN(sendContent);

        } else {
            DPRINT(F("[COMMUNICATION] Replied to gateway with description: "));
            DPRINT(describeType);
            DPRINT(F(" with content: "));
            DPRINTLN(sendContent);
        }
    #endif
}

// -----------------------------------------------------------------------------

void _communicationReportRegistersSizes(
    byte packetId
) {
    char output_content[6];

    // 0 => Packet identifier
    // 1 => Subaction identifier
    // 2 => DI buffer size
    // 3 => DO buffer size
    // 4 => AI buffer size
    // 5 => AO buffer size
    output_content[0] = (char) packetId;
    output_content[1] = (char) COMMUNICATION_DESCRIBE_REGISTERS_SIZE;
    output_content[2] = (char) _communicationGetDigitalBufferSize(false);
    output_content[3] = (char) _communicationGetDigitalBufferSize(true);
    output_content[4] = (char) _communicationGetAnalogBufferSize(false);
    output_content[5] = (char) _communicationGetAnalogBufferSize(true);

    #if DEBUG_SUPPORT
        // Reply to gateway
        _communicationReplyToPacket(output_content, 6);
    #else
        // Reply to gateway
        if (_communicationReplyToPacket(output_content, 6) == false) {
            // Node was not able to notify gateway about its address
            DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node registers sizes\n"));

        } else {
            DPRINT(F("[COMMUNICATION] Replied to gateway with registers sizes\n"));
        }
    #endif
}

// -----------------------------------------------------------------------------
// DIGITAL REGISTERS
// -----------------------------------------------------------------------------

void _communicationReportDigitalRegisters(
    byte packetId,
    uint8_t * payload,
    bool output
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word read_length = (word) payload[3] << 8 | (word) payload[4];
    
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from digital"));
        if (output) {
            DPRINT(F(" output (DO) "));
        } else {
            DPRINT(F(" input (DI) "));
        }
        DPRINT(F(" buffer address: "));
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
        for (byte j = 0; j < 8; j++) {
            byte_buffer[j] = 0;
        }

        byte bit_counter = 0;
        byte byte_counter = 1;
        byte write_byte = 0;

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of data bytes
        // 4-n  => Packet data
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) 0; // Temporary value, will be updated after collecting all

        byte data_write_index = 4;

        for (byte i = register_address; i < (register_address + read_length) && i < _communicationGetDigitalBufferSize(output); i++) {
            byte_buffer[bit_counter] = _communicationGetDigitalBufferValue(output, i) ? 1 : 0;

            bit_counter++;

            // bit counter reached size of one byte
            if (bit_counter >= 8) {
                bit_counter = 0;

                // Converting BIT array to BYTE => decimal number
                write_byte = 0;

                for (byte wr = 0; wr < 8; wr++) {
                    write_byte |= byte_buffer[wr] << wr;
                }

                // Write converted decimal number to output buffer
                output_content[data_write_index] = write_byte;

                byte_counter++;
                data_write_index++;

                // Reset bit buffer
                for (byte j = 0; j < 8; j++) {
                    byte_buffer[j] = 0;
                }
            }
        }

        if (byte_counter == 1 || read_length % 8 != 0) {
            // Converting BIT array to BYTE => decimal number
            write_byte = 0;

            for (byte wr = 0; wr < 8; wr++) {
                write_byte |= byte_buffer[wr] << wr;
            }

            // Write converted decimal number to output buffer
            output_content[data_write_index] = write_byte;
        }

        // Update data bytes length
        output_content[3] = (char) byte_counter;

        #if DEBUG_SUPPORT
            // Reply to gateway
            _communicationReplyToPacket(output_content, (byte_counter + 4));
        #else
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, (byte_counter + 4)) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive digital register reading\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with digital registers content\n"));
            }
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read from undefined digital registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationWriteDigitalOutput(
    byte packetId,
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
        if (_communication_register.digital_outputs[register_address].value != (bool) write_value) {
            _communication_register.digital_outputs[register_address].value = (bool) write_value;
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
            _communicationReplyToPacket(output_content, 5);
        #else
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 5) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive DO register write result\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with DO register write result\n"));
            }
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to write to undefined DO register address\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

void _communicationWriteMultipleDigitalOutput(
    byte packetId,
    uint8_t * payload
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word write_length = (word) payload[3] << 8 | (word) payload[4];
    byte bytes_count = (byte) payload[5];

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
        byte write_byte = 1;
        byte data_byte;

        byte write_address = register_address;

        while (
            write_address < (register_address + write_length)
            && write_address < _communicationGetDigitalBufferSize(true)
            && write_byte <= bytes_count
        ) {
            data_byte = (byte) payload[5 + write_byte];
            bool write_value = false;

            for (byte i = 7; i >= 0; i--) {
                write_value = (data_byte >> i) & 0x01;

                if (_communication_register.digital_outputs[write_address].value != write_value) {
                    _communication_register.digital_outputs[write_address].value = write_value;
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
            _communicationReplyToPacket(output_content, 5);
        #else
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, 5) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive multiple DO register write result\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with multiple DO register write result\n"));
            }
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

void _communicationReportAnalogRegisters(
    byte packetId,
    uint8_t * payload,
    bool output
) {
    char output_content[PJON_PACKET_MAX_LENGTH];

    word register_address = (word) payload[1] << 8 | (word) payload[2];
    word read_length = (word) payload[3] << 8 | (word) payload[4];
    
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Requested reading from digital"));
        if (output) {
            DPRINT(F(" output (AO) "));
        } else {
            DPRINT(F(" input (AI) "));
        }
        DPRINT(F(" buffer address: "));
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
        byte byte_counter = 1;

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of data bytes
        // 4-n  => Packet data
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) 0; // Temporary value, will be updated after collecting all

        for (byte i = register_address; i < (register_address + read_length) && i < _communicationGetAnalogBufferSize(output); i++) {
            output_content[byte_counter] = (char) (_communicationGetAnalogBufferValue(output, i) >> 8);
            output_content[byte_counter + 1] = (char) (_communicationGetAnalogBufferValue(output, i) & 0xFF);

            byte_counter++;
            byte_counter++;
        }

        // Update data bytes length
        output_content[3] = (char) byte_counter;

        #if DEBUG_SUPPORT
            // Reply to gateway
            _communicationReplyToPacket(output_content, (byte_counter + 4));
        #else
            // Reply to gateway
            if (_communicationReplyToPacket(output_content, (byte_counter + 4)) == false) {
                // Node was not able to notify gateway about its address
                DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive analog register reading\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Replied to gateway with analog registers content\n"));
            }
        #endif

    } else {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Gateway is trying to read from undefined analog registers range\n"));
        #endif

        // TODO: Send exception
    }
}

// -----------------------------------------------------------------------------

/**
 * Generate a new node RID
 */
void _communicationGenerateRid() {
    _communication_rid = (
        (uint32_t) (PJON_RANDOM(2147483646)) ^
        (uint32_t) (PJON_ANALOG_READ(A0)) ^
        (uint32_t) (millis())
    ) ^ _communication_rid ^ _communication_last_rid_request_time;
}

// -----------------------------------------------------------------------------

/**
 * Send request to gateway for assigning node address
 */
void _communicationAcquireAddress()
{
    _communicationGenerateRid();

    char output_content[5];

    output_content[0] = COMMUNICATION_PACKET_ACQUIRE_ADDRESS;
    output_content[1] = COMMUNICATION_ACQUIRE_ADDRESS_REQUEST;
    output_content[2] = (uint8_t) ((uint32_t) (_communication_rid) >> 24);
    output_content[3] = (uint8_t) ((uint32_t) (_communication_rid) >> 16);
    output_content[4] = (uint8_t) ((uint32_t) (_communication_rid) >>  8);
    output_content[5] = (uint8_t) ((uint32_t) (_communication_rid));

    if (
        _communication_bus.send_packet_blocking(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            6
        ) == PJON_ACK
    ) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION] Sent request to gateway for address\n"));
        #endif

        return;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION][ERR] Acquisition node address fail\n"));
    #endif
}

// -----------------------------------------------------------------------------

void _communicationAddressRequestHandler(
    uint8_t * payload
) {
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Gateway replied to address negotiation: "));
        _communicationDebugLogNodeAddressAcquire((byte) payload[1]);
    #endif

    uint8_t rid[4] = {
        (uint8_t) ((uint32_t) (_communication_rid) >> 24),
        (uint8_t) ((uint32_t) (_communication_rid) >> 16),
        (uint8_t) ((uint32_t) (_communication_rid) >>  8),
        (uint8_t) ((uint32_t) (_communication_rid))
    };

    char output_content[PJON_PACKET_MAX_LENGTH];

    // Basic reply packet structure
    output_content[0] = (uint8_t) COMMUNICATION_PACKET_ACQUIRE_ADDRESS;
    output_content[1] = (uint8_t) COMMUNICATION_ACQUIRE_ADDRESS_NONE;
    output_content[2] = rid[0];
    output_content[3] = rid[1];
    output_content[4] = rid[2];
    output_content[5] = rid[3];

    uint8_t received_rid[4];

    received_rid[0] = (uint8_t) payload[2];
    received_rid[1] = (uint8_t) payload[3];
    received_rid[2] = (uint8_t) payload[4];
    received_rid[3] = (uint8_t) payload[5];

    switch ((byte) payload[1])
    {
        /**
         * Gateway confirmed address acquire request
         * & replied with assigned node address
         */
        case COMMUNICATION_ACQUIRE_ADDRESS_REQUEST:
            // This packet is broadcasted to all nodes so node has to check RID if is same
            if (PJONTools::bus_id_equality(received_rid, rid)) {
                output_content[1] = (uint8_t) COMMUNICATION_ACQUIRE_ADDRESS_CONFIRM;
                output_content[6] = (uint8_t) payload[6];

                // Set node address defined by gateway
                _communication_bus.set_id((uint8_t) payload[6]);

                #if DEBUG_SUPPORT
                    DPRINTLN();
                    DPRINT(F("[COMMUNICATION] ===========================\n"));
                    DPRINT(F("[COMMUNICATION] Node address was set to: "));
                    DPRINTLN((uint8_t) payload[6]);
                    DPRINT(F("[COMMUNICATION] ===========================\n"));
                    DPRINTLN();

                    DPRINT(F("[COMMUNICATION] Confirming node address to master\n"));
                #endif

                if (
                    _communication_bus.send_packet_blocking(
                        COMMUNICATION_BUS_GATEWAY_ADDR,
                        output_content,
                        7
                    ) != PJON_ACK
                ) {
                    _communication_bus.set_id(PJON_NOT_ASSIGNED);
                #if DEBUG_SUPPORT
                    DPRINT(F("[COMMUNICATION][ERR] Received node address could not be confirmed to gateway\n"));

                } else {
                    DPRINT(F("[COMMUNICATION] Received node address was successfully confirmed to gateway\n"));
                #endif
                }
            }
            break;

        /**
         * Gateway was not able to finish address negotiation
         * Node has to re-request for new address
         */
        case COMMUNICATION_ACQUIRE_ADDRESS_NEGATE:
            if (PJONTools::bus_id_equality(received_rid, rid)) {
                _communicationAcquireAddress();
            }
            break;

        /**
         * Gateway is reinitializing its nodes collection
         */
        case COMMUNICATION_ACQUIRE_ADDRESS_LIST:
            if (communicationMasterAddress() != PJON_NOT_ASSIGNED) {
                if (millis() - _communication_last_rid_request_time > (COMMUNICATION_ADDRESSING_TIMEOUT * 1.125)) {
                    _communication_last_rid_request_time = millis();

                    output_content[1] = (uint8_t) COMMUNICATION_ACQUIRE_ADDRESS_REFRESH;
                    output_content[6] = (uint8_t) communicationMasterAddress();

                    _communication_bus.send(
                        COMMUNICATION_BUS_GATEWAY_ADDR,
                        output_content,
                        7
                    );
                }

            } else if (millis() - _communication_last_rid_request_time > (COMMUNICATION_ADDRESSING_TIMEOUT * 1.125)) {
                _communication_last_rid_request_time = millis();

                _communicationAcquireAddress();
            }
            break;

        #if DEBUG_SUPPORT
            case COMMUNICATION_ACQUIRE_ADDRESS_REFRESH:
                DPRINT(F("[COMMUNICATION][WARN] Refresh action is only for node to gateway direction\n"));
                break;

            case COMMUNICATION_ACQUIRE_ADDRESS_CONFIRM:
                DPRINT(F("[COMMUNICATION][WARN] Confirm action is only for node to gateway direction\n"));
                break;
        #endif
    }
}

// -----------------------------------------------------------------------------

void _communicationDescribeRequestHandler(
    byte packetId,
    uint8_t * payload
) {
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Gateway requested description:"));
        _communicationDebugLogNodeDescription((byte) payload[1]);
    #endif

    switch ((byte) payload[1])
    {
        case COMMUNICATION_DESCRIBE_NODE:
            _communicationReportBasicStructure(packetId);
            break;

        // Send node SN to gateway
        case COMMUNICATION_DESCRIBE_SN:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) NODE_SERIAL_NO);
            break;

        case COMMUNICATION_DESCRIBE_HW_MODEL:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) NODE_NAME);
            return;

        case COMMUNICATION_DESCRIBE_HW_MANUFACTURER:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) NODE_MANUFACTURER);
            return;

        case COMMUNICATION_DESCRIBE_HW_VERSION:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) NODE_VERSION);
            return;

        case COMMUNICATION_DESCRIBE_FW_MODEL:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) FIRMWARE_NAME);
            return;

        case COMMUNICATION_DESCRIBE_FW_MANUFACTURER:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) FIRMWARE_MANUFACTURER);
            return;

        case COMMUNICATION_DESCRIBE_FW_VERSION:
            _communicationReportDescription(packetId, (byte) payload[1], (char *) FIRMWARE_VERSION);
            return;

        // Send register sizes to gateway
        case COMMUNICATION_DESCRIBE_REGISTERS_SIZE:
            _communicationReportRegistersSizes(packetId);
            break;
    }
}

// -----------------------------------------------------------------------------

void _communicationReceiverHandler(
    uint8_t * payload,
    uint16_t length,
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
    byte packet_id = (byte) payload[0];

    if (packet_id == COMMUNICATION_PACKET_NONE || packet_id >= COMMUNICATION_PACKETS_MAX) {
        #if DEBUG_SUPPORT
            DPRINT(F("[COMMUNICATION][ERR] Unknown packet\n"));
        #endif

        return;
    }

    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Received packet: "));
        _communicationDebugLogPacket(packet_id);
    #endif

    // Trying to get node address from gateway
    if (packet_id == COMMUNICATION_PACKET_ACQUIRE_ADDRESS) {
        _communicationAddressRequestHandler(payload);

    // Regular gateway messages
    } else {
        byte sender_address = PJON_NOT_ASSIGNED;

        // Get sender address from header
        if (packetInfo.header & PJON_TX_INFO_BIT) {
            sender_address = packetInfo.sender_id;
        }

        if (sender_address != COMMUNICATION_BUS_GATEWAY_ADDR) {
            #if DEBUG_SUPPORT
                DPRINT(F("[COMMUNICATION][ERR] Received packet from unknown gateway address: "));
                DPRINTLN(sender_address);
            #endif

            return;
        }

        switch (packet_id)
        {
            case COMMUNICATION_PACKET_GATEWAY_PING:
                // Nothing to do, gateway is just testing connection
                break;
        
            case COMMUNICATION_PACKET_WHO_ARE_YOU:
                _communicationDescribeRequestHandler(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_READ_DI:
                _communicationReportDigitalRegisters(packet_id, payload, false);
                break;

            case COMMUNICATION_PACKET_READ_DO:
                _communicationReportDigitalRegisters(packet_id, payload, true);
                break;
        
            case COMMUNICATION_PACKET_READ_AI:
                _communicationReportAnalogRegisters(packet_id, payload, false);
                break;
        
            case COMMUNICATION_PACKET_READ_AO:
                _communicationReportAnalogRegisters(packet_id, payload, true);
                break;
        
            case COMMUNICATION_PACKET_WRITE_ONE_DO:
                _communicationWriteDigitalOutput(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_WRITE_ONE_AO:
                break;
        
            case COMMUNICATION_PACKET_WRITE_MULTI_DO:
                _communicationWriteMultipleDigitalOutput(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_WRITE_MULTI_AO:
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
    uint8_t code,
    uint16_t data,
    void *customPointer
) {
    #if DEBUG_SUPPORT
        if (code == PJON_CONNECTION_LOST) {
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
    byte size
) {
    #if DEBUG_SUPPORT
        DPRINT(F("[COMMUNICATION] Preparing reply packet: "));
        _communicationDebugLogPacket(payload[0]);
    #endif

    _communication_bus.reply(
        payload,    // Content
        size        // Content lenght
    );
/*
    TODO: Should be fixed?
    ======================

    #if DEBUG_SUPPORT
        if (result != PJON_ACK) {
            if (result == PJON_BUSY ) {
                DPRINT(F("[COMMUNICATION] Sending replypacket failed, bus is busy\n"));

            } else if (result == PJON_FAIL) {
                DPRINT(F("[COMMUNICATION] Sending replypacket failed\n"));

            } else {
                DPRINT(F("[COMMUNICATION] Sending replypacket failed, unknonw error\n"));
            }

            return false;
        }

        DPRINT(F("[COMMUNICATION] Replypacket was successfully sent\n"));
    #endif
*/
    return true;
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

bool communicationDiscardAddress()
{
    char output_content[7];

    output_content[0] = COMMUNICATION_PACKET_ACQUIRE_ADDRESS;
    output_content[1] = (uint8_t) COMMUNICATION_ACQUIRE_ADDRESS_NEGATE;
    output_content[2] = (uint8_t) ((uint32_t) (_communication_rid) >> 24);
    output_content[3] = (uint8_t) ((uint32_t) (_communication_rid) >> 16);
    output_content[4] = (uint8_t) ((uint32_t) (_communication_rid) >>  8);
    output_content[5] = (uint8_t) ((uint32_t) (_communication_rid));
    output_content[6] = communicationMasterAddress();

    if (
        _communication_bus.send_packet_blocking(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            6
        ) == PJON_ACK
    ) {
        _communication_bus.set_id(PJON_NOT_ASSIGNED);

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

bool communicationHasMasterAddress() {
    return _communication_bus.device_id() != PJON_NOT_ASSIGNED;
}

// -----------------------------------------------------------------------------

byte communicationMasterAddress() {
    return _communication_bus.device_id();
}

// -----------------------------------------------------------------------------

void communicationResetMasterAddress() {
    _communication_bus.set_id(PJON_NOT_ASSIGNED);
}

// -----------------------------------------------------------------------------

bool communicationConnected() {
    return communicationHasMasterAddress();
}

// -----------------------------------------------------------------------------
// DIGITAL INPUTS
// -----------------------------------------------------------------------------

bool communicationRegisterDigitalInput(
    byte address
) {
    return communicationRegisterDigitalInput(address, false);
}

// -----------------------------------------------------------------------------

bool communicationRegisterDigitalInput(
    byte address,
    bool defaultValue
) {
    for (byte i = 0; i < _communication_register.digital_inputs.size(); i++) {
        // Check if address is free
        if (_communication_register.digital_inputs[i].address == address) {
            return false;
        }
    }

    _communication_register.digital_inputs.push_back((communication_binary_register_t) {
        address,
        defaultValue
    });

    return true;
}

// -----------------------------------------------------------------------------

bool communicationWriteDigitalInput(
    byte address,
    bool value
) {
    for (byte i = 0; i < _communication_register.digital_inputs.size(); i++) {
        // Check if address is registered
        if (_communication_register.digital_inputs[i].address == address) {
            _communication_register.digital_inputs[i].value = value;

            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalInput(
    byte address
) {
    for (byte i = 0; i < _communication_register.digital_inputs.size(); i++) {
        // Check if address is registered
        if (_communication_register.digital_inputs[i].address == address) {
            return (bool) _communication_register.digital_inputs[i].value;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------
// DIGITAL OUTPUTS
// -----------------------------------------------------------------------------

bool communicationRegisterDigitalOutput(
    byte address
) {
    return communicationRegisterDigitalOutput(address, false);
}

// -----------------------------------------------------------------------------

bool communicationRegisterDigitalOutput(
    byte address,
    bool defaultValue
) {
    for (byte i = 0; i < _communication_register.digital_outputs.size(); i++) {
        // Check if address is free
        if (_communication_register.digital_outputs[i].address == address) {
            return false;
        }
    }

    _communication_register.digital_outputs.push_back((communication_binary_register_t) {
        address,
        defaultValue
    });

    return true;
}

// -----------------------------------------------------------------------------

bool communicationWriteDigitalOutput(
    byte address,
    bool value
) {
    for (byte i = 0; i < _communication_register.digital_outputs.size(); i++) {
        // Check if address is free
        if (_communication_register.digital_outputs[i].address == address) {
            _communication_register.digital_outputs[i].value = value;

            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalOutput(
    byte address
) {
    for (byte i = 0; i < _communication_register.digital_outputs.size(); i++) {
        // Check if address is registered
        if (_communication_register.digital_outputs[i].address == address) {
            return (bool) _communication_register.digital_outputs[i].value;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------
// ANALOG INPUTS
// -----------------------------------------------------------------------------

bool communicationRegisterAnalogInput(
    byte address
) {
    return communicationRegisterAnalogInput(address, 0);
}

// -----------------------------------------------------------------------------

bool communicationRegisterAnalogInput(
    byte address,
    word defaultValue
) {
    for (byte i = 0; i < _communication_register.analog_inputs.size(); i++) {
        // Check if address is free
        if (_communication_register.analog_inputs[i].address == address) {
            return false;
        }
    }

    _communication_register.analog_inputs.push_back((communication_analog_register_t) {
        address,
        defaultValue
    });

    return true;
}

// -----------------------------------------------------------------------------

bool communicationWriteAnalogInput(
    byte address,
    word value
) {
    for (byte i = 0; i < _communication_register.analog_inputs.size(); i++) {
        // Check if address is registered
        if (_communication_register.analog_inputs[i].address == address) {
            _communication_register.analog_inputs[i].value = value;

            return true;
        }
    }

    return false;
}

// -----------------------------------------------------------------------------

word communicationReadAnalogInput(
    byte address
) {
    for (byte i = 0; i < _communication_register.analog_inputs.size(); i++) {
        // Check if address is registered
        if (_communication_register.analog_inputs[i].address == address) {
            return (word) _communication_register.analog_inputs[i].value;
        }
    }

    return (word) 0;
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void communicationSetup() {
    Serial.begin(SERIAL_BAUDRATE);

    _communication_serial_bus.begin(SERIAL_BAUDRATE);

    _communication_bus.strategy.set_serial(&_communication_serial_bus);

    // Communication callbacks
    _communication_bus.set_receiver(_communicationReceiverHandler);
    _communication_bus.set_error(_communicationErrorHandler);

    _communication_bus.begin();
}

// -----------------------------------------------------------------------------

void communicationLoop() {
    static unsigned long _communication_last_address_acquire = 0;

    if (
        communicationMasterAddress() == PJON_NOT_ASSIGNED
        && (_communication_last_address_acquire == 0 || millis() - _communication_last_address_acquire > COMMUNICATION_GATEWAY_SEARCH_DELAY)
    ) {
        _communication_last_address_acquire = millis();

        _communicationAcquireAddress();
    }

    // -------------------------------------------------------------------------
    // Bus communication
    // -------------------------------------------------------------------------
    _communication_bus.update();
    _communication_bus.receive();
}
