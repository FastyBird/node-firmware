/*

COMMUNICATION MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include <SoftwareSerial.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <PJON.h>
#include <Vector.h>

PJON<ThroughSerial> bus(COMMUNICATION_BUS_NODE_ADDR);

SoftwareSerial serialBus(COMMUNICATION_BUS_TX_PIN, COMMUNICATION_BUS_RX_PIN);

typedef struct {
    byte address;
    bool value;
} communication_binary_register_t;

typedef struct {
    byte address;
    float value;
} communication_analog_register_t;

typedef struct {
    Vector<communication_binary_register_t> digitalInputs;
    Vector<communication_binary_register_t> digitalOutputs;

    Vector<communication_analog_register_t> analogInputs;
    Vector<communication_analog_register_t> analogOutputs;
} communication_register_t;

communication_register_t _communication_register;

// Communication
byte _communication_master_id = COMMUNICATION_BUS_UNKNOWN_ADDR;

bool _communication_node_waiting_for_master_reply = false;

// -----------------------------------------------------------------------------

bool communicationRegisterBinaryInput(
    byte address,
    bool defaultValue
) {
    for (byte i = 0; i < _communication_register.digitalInputs.size(); i++) {
        // Check if address is free
        if (_communication_register.digitalInputs[i].address == address) {
            return false;
        }
    }

    _communication_register.digitalInputs.push_back((communication_binary_register_t) {
        address,
        defaultValue
    });

    return true;
}

// -----------------------------------------------------------------------------

bool communicationRegisterBinaryInput(
    byte address
) {
    return communicationRegisterBinaryInput(address, false);
}

// -----------------------------------------------------------------------------

bool communicationRegisterBinaryOutput(
    byte address,
    bool defaultValue
) {
    for (byte i = 0; i < _communication_register.digitalInputs.size(); i++) {
        // Check if address is free
        if (_communication_register.digitalOutputs[i].address == address) {
            return false;
        }
    }

    _communication_register.digitalOutputs.push_back((communication_binary_register_t) {
        address,
        defaultValue
    });

    return true;
}

// -----------------------------------------------------------------------------

bool communicationRegisterBinaryOutput(
    byte address
) {
    return communicationRegisterBinaryOutput(address, false);
}





// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

void _communicationTranslatePacket(
    byte packetId
) {
    if (packetId < COMMUNICATION_PACKETS_MAX) {
        int len = strlen_P(communication_packet_none);

        char buffer[40];

        DPRINT(packetId);
        DPRINT(F(" => ("));

        strcpy_P(buffer, (char *) pgm_read_word(&communication_packet_string[packetId]));
        DPRINT(buffer);

        DPRINTLN(F(")"));

    } else {
        DPRINTLN(F("unknown"));
    }
}

// -----------------------------------------------------------------------------

void _communicationReceiverHandler(
    uint8_t * payload,
    uint16_t length,
    const PJON_Packet_Info &packetInfo
) {
    DPRINTLN();
    DPRINT(F("[COMMUNICATION] ===============\n"));
    DPRINT(F("[COMMUNICATION] Received packet\n"));
    DPRINT(F("[COMMUNICATION] ===============\n"));

    // Init variables
    byte sender_id = 0;
    byte packet_id = COMMUNICATION_PACKET_NONE;
    byte receiver_id = 0;
    bool is_broadcast = false;

    // Prepare output buffer for new packet
    char output_buffer[PJON_PACKET_MAX_LENGTH];

    // Check if packet is broadcast packet or direct packet
    if (packetInfo.header & PJON_MODE_BIT) {
        receiver_id = packetInfo.receiver_id;

        if (receiver_id == 0) {
            is_broadcast = true;
        }
    }

    // Get sender address from header
    if (packetInfo.header & PJON_TX_INFO_BIT) {
        sender_id = packetInfo.sender_id;
        packet_id = packetInfo.id;
    }

    if (packet_id == COMMUNICATION_PACKET_NONE) {
        DPRINT(F("[COMMUNICATION][ERR] Unknown packet"));

        return;
    }

    DPRINT(F("[COMMUNICATION] Received packet: "));
    _communicationTranslatePacket(packet_id);
    
    switch (packet_id)
    {
        case COMMUNICATION_PACKET_MASTER_LOOKUP:
            // Master is broadcasting to all nodes
            if (is_broadcast) {
                // Master addres is not set yet
                if (_communication_master_id == COMMUNICATION_BUS_UNKNOWN_ADDR) {
                    // Master address was in header
                    if (sender_id != COMMUNICATION_BUS_UNKNOWN_ADDR) {
                        _communication_master_id = sender_id;

                        DPRINTLN();
                        DPRINT(F("[COMMUNICATION] ==============================\n"));
                        DPRINT(F("[COMMUNICATION] Master address was set to: "));
                        DPRINTLN(_communication_master_id);
                        DPRINT(F("[COMMUNICATION] ==============================\n"));
                        DPRINTLN();

                        char output_content[1] = { (char) COMMUNICATION_BUS_NODE_ADDR };

                        // Reply to master
                        if (communicationReplyToPacket(packet_id, output_content) == false) {
                            // Node was not able to notify master about its address
                            DPRINT(F("[COMMUNICATION][ERR] Master could not receive node address\n"));
                        }

                    // Master address was missing
                    } else {
                        DPRINT(F("[COMMUNICATION][ERR] Master address was not found in header\n"));
                    }
                }

            // Master is sending direct packet to node
            } else {
                DPRINT(F("[COMMUNICATION][ERR] Not supported\n"));
            }
            break;
    
        case COMMUNICATION_PACKET_WHO_ARE_YOU:
            // Add info about what is in reply

            DPRINT(F("[COMMUNICATION] Master requested description:"));
            DPRINTLN((byte) payload[0]);

            switch ((byte) payload[0])
            {
                // Send node SN to master
                case COMMUNICATION_DESCRIBE_NODE:
                    char output_content[5] = NODE_SERIAL_NO;
                    break;

                case COMMUNICATION_DESCRIBE_HARDWARE:
                    DPRINT(F("[COMMUNICATION][ERR] Not supported\n"));
                    return;

                case COMMUNICATION_DESCRIBE_FIRMWARE:
                    DPRINT(F("[COMMUNICATION][ERR] Not supported\n"));
                    return;

                // Send register sizes to master
                case COMMUNICATION_DESCRIBE_REGISTERS_SIZE:
                    char output_content[5] = {
                        (char) COMMUNICATION_PACKET_WHO_ARE_YOU,
                        (char) _communication_register.digitalInputs.size(),
                        (char) _communication_register.digitalOutputs.size(),
                        (char) _communication_register.analogInputs.size(),
                        (char) _communication_register.analogOutputs.size()
                    };
                    break;
            }

            // Reply to master
            if (communicationReplyToPacket(packet_id, output_content) == false) {
                // Node was not able to notify master about its address
                DPRINT(F("[COMMUNICATION][ERR] Master could not receive node description\n"));
            }
            break;
    
        case COMMUNICATION_PACKET_READ_DI:
            break;
    
        case COMMUNICATION_PACKET_READ_DO:
            break;
    
        case COMMUNICATION_PACKET_READ_AI:
            break;
    
        case COMMUNICATION_PACKET_READ_AO:
            break;
    
        case COMMUNICATION_PACKET_WRITE_ONE_DO:
            break;
    
        case COMMUNICATION_PACKET_WRITE_ONE_AO:
            break;
    
        case COMMUNICATION_PACKET_WRITE_MULTI_DO:
            break;
    
        case COMMUNICATION_PACKET_WRITE_MULTI_AO:
            break;
    }

    DPRINT(F("[COMMUNICATION] ========================\n"));
    DPRINT(F("[COMMUNICATION] Handling packet finished\n"));
    DPRINT(F("[COMMUNICATION] ========================\n"));
    DPRINTLN();

    return;



/*
    StaticJsonBuffer<PJON_PACKET_MAX_LENGTH> json_buffer;

    JsonObject& parsed_packet = json_buffer.parseObject((char *) payload);

    if (!parsed_packet.success()) {
        DPRINT(F("[COMMUNICATION] Error parsing payload data, invalid JSON\n"));
        DPRINT(F("[COMMUNICATION] Received paylodad with length: "));
        DPRINT(length);
        DPRINT(F(" & content: "));
        // DPRINTLN(formatted_packet_buffer);
        DPRINTLN();

        return;

    } else {
        DPRINT(F("[COMMUNICATION] Successfully parsed JSON\n"));
        DPRINT(F("[COMMUNICATION] Received paylodad with length: "));
        DPRINT(length);
        DPRINT(F(" & content: "));
        // DPRINTLN(formatted_packet_buffer);
        DPRINTLN();
    }

    // Packed has to have attribute "P" which define packet type
    if (parsed_packet.containsKey("P") == false)  {
        DPRINT(F("[COMMUNICATION] Missing packet identifier: \"P\" in payload\n"));

        return;
    }

    // Extract packet identifier
    byte packet_id = parsed_packet["P"].as<byte>();

    // Prepare output buffer for new packet
    char output_buffer[PJON_PACKET_MAX_LENGTH];

    // Sent packet was accepted by master
    if (packet_id == COMMUNICATION_PACKET_ACCEPTED) {
        DPRINT(F("[COMMUNICATION] Recieved acceptation packet\n"));

        // Acceptation packet has to have accepted packet identifier
        if (parsed_packet.containsKey("A") == false)  {
            DPRINT(F("[COMMUNICATION] Recieved acceptation packet with invalid structure\n"));

            return;
        }
        
        // Extract accepted packet identifier
        byte accepted_packet = parsed_packet["A"].as<byte>();

        DPRINT(F("[COMMUNICATION] Received acceptation for packet: "));
        _communicationTranslatePacket(accepted_packet);

        // Master is sending reply for identification request
        if (accepted_packet == COMMUNICATION_PACKET_MASTER_LOOKUP) {
            // If sender device id is included
            if (sender_id != COMMUNICATION_BUS_UNKNOWN_ADDR) {
                _communication_master_id = sender_id;

                DPRINTLN();
                DPRINT(F("[COMMUNICATION] ==============================\n"));
                DPRINT(F("[COMMUNICATION] Master address was set to: "));
                DPRINTLN(_communication_master_id);
                DPRINT(F("[COMMUNICATION] ==============================\n"));
                DPRINTLN();

            } else {
                DPRINT(F("[COMMUNICATION] Master address ID not found in header\n"));
            }

            _communication_node_waiting_for_master_reply = false;

        // Node packet
        } else {
            nodePacketAccepted(accepted_packet, sender_id, parsed_packet);
        }

    // Master has replyed to node PING
    } else if (packet_id == COMMUNICATION_PACKET_PONG) {
        DPRINT(F("[COMMUNICATION] Master replied to PING with PONG\n"));

        systemPongReceived();

    // Master is sending PING request. Node have to respond with PONG
    } else if (packet_id == COMMUNICATION_PACKET_PING) {
        sprintf(output_buffer, "{\"P\":%d, \"A\":%d}", COMMUNICATION_PACKET_ACCEPTED, COMMUNICATION_PACKET_PONG);

        communicationReplyToPacket(packet_id, output_buffer);

    // Master replied with error
    } else if (packet_id == COMMUNICATION_PACKET_ERROR) {

    // Master requested initialization sequence
    } else if (packet_id == COMMUNICATION_PACKET_INIT_RESTART) {
        if (sender_id != COMMUNICATION_BUS_UNKNOWN_ADDR && sender_id == _communication_master_id) {
            nodeResetInitialization();

            sprintf(output_buffer, "{\"P\":%d, \"A\":%d}", COMMUNICATION_PACKET_ACCEPTED, COMMUNICATION_PACKET_INIT_RESTART);

            communicationReplyToPacket(packet_id, output_buffer);

        } else {
            DPRINT(F("[COMMUNICATION] Received reinitialization request from unknown master\n"));
        }

    // Master is probably restarted and do not know about node
    } else if (packet_id == COMMUNICATION_PACKET_WHO_ARE_YOU) {
        if (sender_id != COMMUNICATION_BUS_UNKNOWN_ADDR && sender_id == _communication_master_id) {
            nodeResetInitialization();

            sprintf(output_buffer, "{\"P\":%d, \"A\":%d}", COMMUNICATION_PACKET_ACCEPTED, COMMUNICATION_PACKET_WHO_ARE_YOU);

            communicationReplyToPacket(packet_id, output_buffer);

        } else {
            DPRINT(F("[COMMUNICATION] Received master do not know request from unknown master\n"));
        }

    // Requested output update
    } else if (packet_id == COMMUNICATION_PACKET_DATA) {
        if (sender_id != COMMUNICATION_BUS_UNKNOWN_ADDR && sender_id == _communication_master_id) {
            #if RELAY_SUPPORT && RELAY_PROVIDER != RELAY_PROVIDER_NONE
                if (parsed_packet["T"].as<byte>() == NODE_CHANNEL_TYPE_SWITCH) {
                    relayStatus(parsed_packet["C"].as<byte>(), parsed_packet["V"].as<bool>());
                }
            #endif

            sprintf(output_buffer, "{\"P\":%d, \"A\":%d}", COMMUNICATION_PACKET_ACCEPTED, COMMUNICATION_PACKET_DATA);

            communicationReplyToPacket(packet_id, output_buffer);

        } else {
            DPRINT(F("[COMMUNICATION] Received data request from unknown master\n"));
        }
    }

    DPRINT(F("[COMMUNICATION] ========================\n"));
    DPRINT(F("[COMMUNICATION] Handling packet finished\n"));
    DPRINT(F("[COMMUNICATION] ========================\n"));
    DPRINTLN();
*/
}

// -----------------------------------------------------------------------------

void _communicationErrorHandler(
    uint8_t code,
    uint16_t data,
    void *customPointer
) {
    if (code == PJON_CONNECTION_LOST) {
        DPRINT(F("[COMMUNICATION] Error. Connection lost with gateway\n"));

    } else if (code == PJON_PACKETS_BUFFER_FULL) {
        DPRINT(F("[COMMUNICATION] Error. Buffer is full\n"));

    } else if (code == PJON_CONTENT_TOO_LONG) {
        DPRINT(F("[COMMUNICATION] Error. Content is long\n"));
    } else {
        DPRINT(F("[COMMUNICATION] Unknown error\n"));
    }
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

bool communicationSendPacket(
    byte packet,
    char * payload
) {
    if (_communication_master_id == COMMUNICATION_BUS_UNKNOWN_ADDR) {
        DPRINT(F("[COMMUNICATION] Error. Master address is unknown\n"));

        return false;
    }

    communicationSendPacket(_communication_master_id, packet, payload);
}

// -----------------------------------------------------------------------------

bool communicationSendPacket(
    byte recipient,
    byte packet,
    char * payload
) {
return true;
    DPRINT(F("[COMMUNICATION] Preparing packet: "));
    _communicationTranslatePacket(packet);

    uint16_t result = bus.send_packet(
        recipient,          // Master address
        payload,            // Content
        strlen(payload),    // Content length
        bus.config,         // Header
        packet              // Packet identifier
    );
return true;
    if (result != PJON_ACK) {
        if (result == PJON_BUSY ) {
            DPRINT(F("[COMMUNICATION] Sending packet failed, bus is busy\n"));

        } else if (result == PJON_FAIL) {
            DPRINT(F("[COMMUNICATION] Sending packet failed\n"));

        } else {
            DPRINT(F("[COMMUNICATION] Sending packet failed, unknonw error\n"));
        }

        return false;
    }

    DPRINT(F("[COMMUNICATION] Packet was successfully sent\n"));

    return true;
}

// -----------------------------------------------------------------------------

bool communicationReplyToPacket(
    byte packet,
    char * payload
) {
    DPRINT(F("[COMMUNICATION] Preparing reply packet: "));
    _communicationTranslatePacket(packet);

    bus.reply(payload, 4);

    return true;
}

// -----------------------------------------------------------------------------

void communicationSearchForMaster() {
    // Check reconnect interval
    static unsigned long last = 0;

    if (millis() - last < COMMUNICATION_MASTER_SEARCH_DELAY) {
        return;
    }

    last = millis();

    DPRINTLN();
    DPRINT(F("[COMMUNICATION] ====================\n"));
    DPRINT(F("[COMMUNICATION] Searching for MASTER\n"));
    DPRINT(F("[COMMUNICATION] ====================\n"));
    DPRINTLN();

    char output_buffer[PJON_PACKET_MAX_LENGTH];
    sprintf(output_buffer, "{\"P\":%d}", COMMUNICATION_PACKET_MASTER_LOOKUP);

    // Send search packet to all nodes in network
    if (communicationSendPacket(PJON_MASTER_ID, COMMUNICATION_PACKET_MASTER_LOOKUP, output_buffer)) {
        _communication_node_waiting_for_master_reply = true;
    }
}

// -----------------------------------------------------------------------------

bool communicationHasMasterAddress() {
    return _communication_master_id != 0;
}

// -----------------------------------------------------------------------------

byte communicationMasterAddress() {
    return _communication_master_id;
}

// -----------------------------------------------------------------------------

void communicationResetMasterAddress() {
    _communication_master_id = COMMUNICATION_BUS_UNKNOWN_ADDR;
}

// -----------------------------------------------------------------------------

bool communicationConnected() {
    return communicationHasMasterAddress();
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void communicationSetup() {
    Serial.begin(SERIAL_BAUDRATE);

    serialBus.begin(SERIAL_BAUDRATE);

    bus.set_packet_id(true);

    bus.strategy.set_serial(&serialBus);

    // Set asynchronous acknowledgement response, avoid sync
    //bus.set_synchronous_acknowledge(false);
    
    // Communication callbacks
    bus.set_receiver(_communicationReceiverHandler);
    bus.set_error(_communicationErrorHandler);

    bus.begin();
}

// -----------------------------------------------------------------------------

void communicationLoop() {
    // -------------------------------------------------------------------------
    // Autodiscover
    // -------------------------------------------------------------------------
    if (communicationHasMasterAddress() == false) {
        // communicationSearchForMaster();
    }

    // -------------------------------------------------------------------------
    // Bus communication
    // -------------------------------------------------------------------------

    bus.update();
    //bus.receive(TS_TIME_IN + TS_BYTE_TIME_OUT);
    bus.receive();
}
