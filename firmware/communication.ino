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

struct communication_binary_register_t {
    const char * name;
    uint8_t data_type;
    bool value;
};

typedef struct {
    const char * name;
    uint8_t data_type;
    word value;
} communication_analog_register_t;

struct communication_register_t {
    Vector<communication_binary_register_t> digital_inputs;
    Vector<communication_binary_register_t> digital_outputs;

    Vector<communication_analog_register_t> analog_inputs;
    Vector<communication_analog_register_t> analog_outputs;
};

communication_register_t _communication_register;

uint32_t _communication_last_node_search_request_time;
uint32_t _communication_master_lost = 0;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

bool _communicationIsPacketInGroup(
    uint8_t packetId,
    const int * group,
    uint8_t length
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
    uint8_t packetId,
    const int * group,
    uint8_t length
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
        uint8_t packetId
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
    bool output
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
    uint8_t index
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
// DIGITAL REGISTERS
// -----------------------------------------------------------------------------

void _communicationReportDigitalRegisters(
    uint8_t packetId,
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

        uint8_t data_write_index = 4;

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
                output_content[data_write_index] = write_byte;

                byte_counter++;
                data_write_index++;

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

void _communicationWriteSingleDigitalOutput(
    uint8_t packetId,
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

void _communicationWriteMultipleDigitalOutputs(
    uint8_t packetId,
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

        while (
            write_address < (register_address + write_length)
            && write_address < _communicationGetDigitalBufferSize(true)
            && write_byte <= bytes_count
        ) {
            data_byte = (uint8_t) payload[5 + write_byte];
            bool write_value = false;

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
    uint8_t packetId,
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
        uint8_t byte_counter = 1;

        // 0    => Packet identifier
        // 1    => High byte of register address
        // 2    => Low byte of register address
        // 3    => Count of data bytes
        // 4-n  => Packet data
        output_content[0] = (char) packetId;
        output_content[1] = (char) (register_address >> 8);
        output_content[2] = (char) (register_address & 0xFF);
        output_content[3] = (char) 0; // Temporary value, will be updated after collecting all

        for (uint8_t i = register_address; i < (register_address + read_length) && i < _communicationGetAnalogBufferSize(output); i++) {
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
// NODE ADDRESSING PROCESS
// -----------------------------------------------------------------------------

/**
 * PAYLOAD:
 * 0 => Packet identifier
 */
void _communicationNodesSearchRequestHandler(
    uint8_t * payload
) {
    if (millis() - _communication_last_node_search_request_time > (COMMUNICATION_ADDRESSING_TIMEOUT * 1.125)) {
        _communication_last_node_search_request_time = millis();

        char output_content[PJON_PACKET_MAX_LENGTH];

        // 0    => Packet identifier
        // 1    => Node bus address
        // 2    => Max packet size
        // 3    => Node SN length
        // 4-n  => Node parsed SN
        output_content[0] = (uint8_t) COMMUNICATION_PACKET_SEARCH_NODES;
        output_content[1] = (uint8_t) communicationNodeAddress();
        output_content[2] = (uint8_t) PJON_PACKET_MAX_LENGTH;
        output_content[3] = (uint8_t) (strlen((char *) NODE_SERIAL_NO) + 1);

        uint8_t byte_pointer = 4;

        for (uint8_t i = 0; i < strlen((char *) NODE_SERIAL_NO); i++) {
            output_content[byte_pointer] = ((char *) NODE_SERIAL_NO)[i];

            byte_pointer++;
        }

        output_content[byte_pointer] = 0; // Be sure to set the null terminator!!!

        // Non blocking packet transfer
        _communication_bus.send(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            (byte_pointer + 1)
        );
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
    uint8_t * payload
) {
    // Extract address assigned by gateway
    uint8_t address = (uint8_t) payload[1];

    char node_sn[(uint8_t) payload[2]];

    // Extract node serial number from payload
    for (uint8_t i = 0; i < (uint8_t) payload[2]; i++) {
        node_sn[i] = (char) payload[i + 3];
    }

    // Check if received packet if for this node
    if (strcmp((char *) NODE_SERIAL_NO, (char *) node_sn) != 0) {
        return;
    }

    // Set node address to received from gateway
    _communication_bus.set_id(address);
    
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
    uint8_t packetId,
    uint8_t * payload
) {
    switch (packetId)
    {
        /**
         * Gateway is searching for nodes
         */
        case COMMUNICATION_PACKET_SEARCH_NODES:
            _communicationNodesSearchRequestHandler(payload);
            break;

        /**
         * Gateway provided node address
         */
        case COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM:
            _communicationAddressConfirmRequestHandler(payload);
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
    uint8_t packetId,
    char * sendContent
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
        _communicationReplyToPacket(output_content, (byte_pointer + 1));
    #else
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
    #endif
}

// -----------------------------------------------------------------------------

void _communicationNodeInitializationRequestHandler(
    uint8_t packetId,
    uint8_t * payload
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
    uint8_t packetId
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
        _communicationReplyToPacket(output_content, 5);
    #else
        // Reply to gateway
        if (_communicationReplyToPacket(output_content, 5) == false) {
            // Node was not able to notify gateway about its address
            DPRINT(F("[COMMUNICATION][ERR] Gateway could not receive node registers sizes\n"));

        } else {
            DPRINT(F("[COMMUNICATION] Replied to gateway with registers sizes\n"));
        }
    #endif
}

// -----------------------------------------------------------------------------

void _communicationRegisterInitializationRequestHandler(
    uint8_t packetId,
    uint8_t * payload
) {
    switch (packetId)
    {
        case COMMUNICATION_PACKET_REGISTERS_SIZE:
            _communicationReportRegistersSizes(packetId);
            break;

        case COMMUNICATION_PACKET_DI_REGISTERS_STRUCTURE:
            break;

        case COMMUNICATION_PACKET_DO_REGISTERS_STRUCTURE:
            break;

        case COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE:
            break;

        case COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE:
            break;
    }
}

// -----------------------------------------------------------------------------
// COMMUNICATION HANDLERS
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
    _communication_master_lost = 0;
        
    // Trying to get node address from gateway
    if (_communicationIsPacketInGroup(packet_id, communication_packets_addresing, COMMUNICATION_PACKET_ADDRESS_MAX)) {
        _communicationAddressRequestHandler(packet_id, payload);

    } else if (_communicationIsPacketInGroup(packet_id, communication_packets_node_initialization, COMMUNICATION_PACKET_NODE_INIT_MAX)) {
        _communicationNodeInitializationRequestHandler(packet_id, payload);

    } else if (_communicationIsPacketInGroup(packet_id, communication_packets_registers_initialization, COMMUNICATION_PACKET_REGISTERS_INIT_MAX)) {
        _communicationRegisterInitializationRequestHandler(packet_id, payload);

    // Regular gateway messages
    } else {
        switch (packet_id)
        {
            case COMMUNICATION_PACKET_GATEWAY_PING:
                // Nothing to do, gateway is just testing connection
                break;
        
            case COMMUNICATION_PACKET_READ_SINGLE_DI:
                break;

            case COMMUNICATION_PACKET_READ_MULTI_DI:
                _communicationReportDigitalRegisters(packet_id, payload, false);
                break;

            case COMMUNICATION_PACKET_READ_SINGLE_DO:
                break;

            case COMMUNICATION_PACKET_READ_MULTI_DO:
                _communicationReportDigitalRegisters(packet_id, payload, true);
                break;
        
            case COMMUNICATION_PACKET_READ_AI:
                _communicationReportAnalogRegisters(packet_id, payload, false);
                break;
        
            case COMMUNICATION_PACKET_READ_AO:
                _communicationReportAnalogRegisters(packet_id, payload, true);
                break;
        
            case COMMUNICATION_PACKET_WRITE_ONE_DO:
                _communicationWriteSingleDigitalOutput(packet_id, payload);
                break;
        
            case COMMUNICATION_PACKET_WRITE_ONE_AO:
                break;
        
            case COMMUNICATION_PACKET_WRITE_MULTI_DO:
                _communicationWriteMultipleDigitalOutputs(packet_id, payload);
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
            _communication_master_lost = millis();

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
    uint8_t size
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
    output_content[1] = communicationNodeAddress();

    if (
        _communication_bus.send_packet_blocking(
            COMMUNICATION_BUS_GATEWAY_ADDR,
            output_content,
            2
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

bool communicationHasAssignedAddress() {
    return _communication_bus.device_id() != PJON_NOT_ASSIGNED;
}

// -----------------------------------------------------------------------------

uint8_t communicationNodeAddress() {
    return _communication_bus.device_id();
}

// -----------------------------------------------------------------------------

void communicationResetNodeAddress() {
    _communication_bus.set_id(PJON_NOT_ASSIGNED);
}

// -----------------------------------------------------------------------------

bool communicationConnected() {
    return communicationHasAssignedAddress();
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
        "",
        COMMUNICATION_DATA_TYPE_BOOLEAN,
        defaultValue
    });

    return (_communication_register.digital_inputs.size() - 1);
}

// -----------------------------------------------------------------------------

bool communicationWriteDigitalInput(
    uint8_t address,
    bool value
) {
    if (address > _communication_register.digital_inputs.size()) {
        return false;
    }

    _communication_register.digital_inputs[address].value = value;

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalInput(
    uint8_t address
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
    bool defaultValue
) {
    _communication_register.digital_outputs.push_back((communication_binary_register_t) {
        "",
        COMMUNICATION_DATA_TYPE_BOOLEAN,
        defaultValue
    });

    return (_communication_register.digital_outputs.size() - 1);
}

// -----------------------------------------------------------------------------

bool communicationWriteDigitalOutput(
    uint8_t address,
    bool value
) {
    if (address > _communication_register.digital_outputs.size()) {
        return false;
    }

    _communication_register.digital_outputs[address].value = value;

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReadDigitalOutput(
    uint8_t address
) {
    if (address > _communication_register.digital_outputs.size()) {
        return false;
    }

    return _communication_register.digital_outputs[address].value;
}

// -----------------------------------------------------------------------------
// ANALOG INPUTS
// -----------------------------------------------------------------------------

uint8_t communicationRegisterAnalogInput(
    uint8_t dataType
) {
    return communicationRegisterAnalogInput(0, dataType);
}

// -----------------------------------------------------------------------------

uint8_t communicationRegisterAnalogInput(
    word defaultValue,
    uint8_t dataType
) {
    _communication_register.analog_inputs.push_back((communication_analog_register_t) {
        "",
        dataType,
        defaultValue
    });

    return (_communication_register.analog_inputs.size() - 1);
}

// -----------------------------------------------------------------------------

bool communicationWriteAnalogInput(
    uint8_t address,
    word value
) {
    if (address > _communication_register.analog_inputs.size()) {
        return false;
    }

    _communication_register.analog_inputs[address].value = value;

    return true;
}

// -----------------------------------------------------------------------------

word communicationReadAnalogInput(
    uint8_t address
) {
    if (address > _communication_register.analog_inputs.size()) {
        return (word) 0;
    }

    return (word) _communication_register.analog_inputs[address].value;
}

// -----------------------------------------------------------------------------
// ANALOG OUTPUTS
// -----------------------------------------------------------------------------

// TODO

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
}

// -----------------------------------------------------------------------------

void communicationLoop() {
    // -------------------------------------------------------------------------
    // Bus communication
    // -------------------------------------------------------------------------
    _communication_bus.update();
    _communication_bus.receive();
}
