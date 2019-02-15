/*

NODE MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include <Vector.h>

typedef struct {
    byte packet;
    bool initialized;
} node_initialize_packet_t;

Vector<node_initialize_packet_t> _node_initialization;

bool _node_waiting_for_response = false;
byte _node_initialization_step = NODE_INIT_INIT;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

int _nodefindInitializationPacketInfoIndex(
    byte packetId
) {
    for (byte i = 0; i < _node_initialization.size(); i++) {
        if (_node_initialization[i].packet == packetId) {
            return i;
        }
    }

    return -1;  
}

// -----------------------------------------------------------------------------

void _nodePushBackInitilizationPacket(
    byte packetId,
    bool state
) {
    int index = _nodefindInitializationPacketInfoIndex(packetId);

    if (index != -1) {
        _node_initialization[index].initialized = state;

    } else {
        _node_initialization.push_back({packetId, state});  
    }
}

// -----------------------------------------------------------------------------

bool _nodeIsPacketInitialized(
    byte packetId
) {
    int index = _nodefindInitializationPacketInfoIndex(packetId);

    if (index != -1 && _node_initialization[index].initialized == true) {
        return true;
    }

    return false;
}

// -----------------------------------------------------------------------------

void _nodeResetInitializationPackets() {
/*
    _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_INIT_START, false);

    _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_HARDWARE_INFO, false);
    _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_FIRMWARE_INFO, false);

    _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_SETTINGS_SCHEMA, false);
    _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_CHANNELS_SCHEMA, false);

    _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_INIT_END, false);
*/
}

// -----------------------------------------------------------------------------

/**
 * FLOW:
 *
 * 1. Check if node is initialized
 * 2. If is, then skip initialization process and mark node as initialized
 * 3. If not, send all initialization packets every NODE_INIT_DELAY
 */
void _nodeInitialize() {
    // Check if node is fully initialized to master
    if (nodeIsInitialized()) {
        DPRINT(F("[NODE] All init packets were sent\n"));

        systemResetCounters();

        return;

    // Check if initialization process is ongoing
    } else if (_node_initialization_step != NODE_INIT_INIT && _node_initialization_step != NODE_INIT_WAIT) {
        _nodeSendInitializationPacket();

        return;

    // Check if node is waiting for finishing initialization
    } else if (_node_initialization_step == NODE_INIT_WAIT) {
        return;
    }

    // Check reconnect interval
    static unsigned long last = 0;

    if (millis() - last < NODE_INIT_DELAY) {
        return;
    }

    last = millis();

    if (communicationHasMasterAddress() == false) {
        DPRINT(F("[NODE] Unknown master address\n"));

        return;
    }

    _nodeResetInitializationPackets();

    _nodeSendInitializationPacket();
}

// -----------------------------------------------------------------------------

void _nodeSendInitializationPacket() {
    // Packet was sent, waiting for response
    if (_node_waiting_for_response == true) {
        return;
    }
/*
    char output_buffer[PJON_PACKET_MAX_LENGTH];

    switch (_node_initialization_step) {
        case NODE_INIT_INIT:
            DPRINTLN();
            DPRINT(F("[NODE] ======================================\n"));
            DPRINT(F("[NODE] Starting node to master initialization\n"));
            DPRINT(F("[NODE] ======================================\n"));
            DPRINTLN();

            // Send node basic info
            // P    = Packet id             => COMMUNICATION_PACKET_HARDWARE_INFO
            // N    = Node serial number    => NODE_SERIAL_NO                       // Automatically generated per node
            sprintf(
                output_buffer,
                "{\"P\":%d,\"N\":\"%s\"}",
                COMMUNICATION_PACKET_INIT_START,
                NODE_SERIAL_NO
            );

            if (communicationSendPacket(COMMUNICATION_PACKET_INIT_START, output_buffer)) {
                _node_initialization_step = NODE_INIT_HARDWARE;

                _node_waiting_for_response = true;
            }
            break;

        case NODE_INIT_HARDWARE:
            // Send node basic info
            // P    = Packet id         => COMMUNICATION_PACKET_HARDWARE_INFO
            // M    = Manufacturer      => NODE_MANUFACTURER
            // T    = Model/Type        => NODE_NAME
            // V    = Version           => NODE_VERSION
            sprintf(
                output_buffer,
                "{\"P\":%d,\"M\":\"%s\",\"T\":\"%s\",\"V\":\"%s\"}",
                COMMUNICATION_PACKET_HARDWARE_INFO,
                NODE_MANUFACTURER,
                NODE_NAME,
                NODE_VERSION
            );

            if (communicationSendPacket(COMMUNICATION_PACKET_HARDWARE_INFO, output_buffer)) {
                _node_initialization_step = NODE_INIT_FIRMWARE;

                _node_waiting_for_response = true;
            }
            break;

        case NODE_INIT_FIRMWARE:
            // Send node basic info
            // P    = Packet id         => COMMUNICATION_PACKET_FIRMWARE_INFO
            // M    = Manufacturer      => FIRMWARE_MANUFACTURER
            // T    = Model/Type        => FIRMWARE_NAME
            // V    = Version           => FIRMWARE_VERSION
            sprintf(
                output_buffer,
                "{\"P\":%d,\"M\":\"%s\",\"T\":\"%s\",\"V\":\"%s\"}",
                COMMUNICATION_PACKET_FIRMWARE_INFO,
                FIRMWARE_MANUFACTURER,
                FIRMWARE_NAME,
                FIRMWARE_VERSION
            );

            if (communicationSendPacket(COMMUNICATION_PACKET_FIRMWARE_INFO, output_buffer)) {
                _node_initialization_step = NODE_INIT_SETTINGS;

                _node_waiting_for_response = true;
            }
            break;

        case NODE_INIT_SETTINGS:
            // TODO: refactor it for settings    
            _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_SETTINGS_SCHEMA, true);

            _node_initialization_step = NODE_INIT_INPUTS;
            break;

        case NODE_INIT_INPUTS:
            _node_initialization_step = NODE_INIT_OUTPUTS;
            break;

        case NODE_INIT_OUTPUTS:
            _node_initialization_step = NODE_INIT_BUTTONS;
            break;

        case NODE_INIT_BUTTONS:
            #if BUTTON_SUPPORT
                if (buttonCount() > 0) {
                    // Send node inputs details
                    // P    = Packet id                 => COMMUNICATION_PACKET_CHANNELS_SCHEMA
                    // T    = Channel type              => BUTTON
                    // C    = Number of channels        => buttonCount()
                    // I    = Channel is readable       => TRUE             // Value could be read from channel
                    // O    = Channel is writtable      => TRUE             // Channel could be set to value
                    // F    = Channel data type format  => BOOL             // Channel value data type
                    sprintf(
                        output_buffer,
                        "{\"P\":%d,\"T\":\"%d\",\"C\":\"%d\",\"I\":\"T\",\"O\":\"F\",\"F\":\"%d\"}",
                        COMMUNICATION_PACKET_CHANNELS_SCHEMA,
                        NODE_CHANNEL_TYPE_BUTTON,
                        buttonCount(),
                        NODE_CHANNEL_FORMAT_EVENT
                    );

                    if (communicationSendPacket(COMMUNICATION_PACKET_CHANNELS_SCHEMA, output_buffer)) {
                        _node_initialization_step = NODE_INIT_SWITCHES;

                        _node_waiting_for_response = true;
                    }

                } else {
                    _node_initialization_step = NODE_INIT_SWITCHES;
                }
            #else
                _node_initialization_step = NODE_INIT_SWITCHES;
            #endif        
            break;

        case NODE_INIT_SWITCHES:
            #if RELAY_SUPPORT && RELAY_PROVIDER != RELAY_PROVIDER_NONE
                if (relayCount() > 0) {
                    // Send node inputs details
                    // P    = Packet id                 => COMMUNICATION_PACKET_CHANNELS_SCHEMA
                    // T    = Channel type              => SWITCH
                    // C    = Number of channels        => relayCount()
                    // I    = Channel is readable       => TRUE             // Value could be read from channel
                    // O    = Channel is writtable      => TRUE             // Channel could be set to value
                    // F    = Channel data type format  => BOOL             // Channel value data type
                    sprintf(
                        output_buffer,
                        "{\"P\":%d,\"T\":\"%d\",\"C\":\"%d\",\"I\":\"T\",\"O\":\"T\",\"F\":\"%d\"}",
                        COMMUNICATION_PACKET_CHANNELS_SCHEMA,
                        NODE_CHANNEL_TYPE_SWITCH,
                        relayCount(),
                        NODE_CHANNEL_FORMAT_DIGITAL
                    );

                    if (communicationSendPacket(COMMUNICATION_PACKET_CHANNELS_SCHEMA, output_buffer)) {
                        _node_initialization_step = NODE_INIT_FINISHED;

                        _node_waiting_for_response = true;
                    }

                } else {
                    _node_initialization_step = NODE_INIT_FINISHED;
                }
            #else
                _node_initialization_step = NODE_INIT_FINISHED;
            #endif
            break;

        case NODE_INIT_FINISHED:
            _nodePushBackInitilizationPacket(COMMUNICATION_PACKET_CHANNELS_SCHEMA, true);

            // Send node basic info
            // P    = Packet id     => COMMUNICATION_PACKET_INIT_END
            sprintf(
                output_buffer,
                "{\"P\":%d}",
                COMMUNICATION_PACKET_INIT_END
            );

            if (communicationSendPacket(COMMUNICATION_PACKET_INIT_END, output_buffer)) {
                _node_initialization_step = NODE_INIT_WAIT;

                _node_waiting_for_response = true;
            }
            break;
    }
*/
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

void nodePacketAccepted(
    byte packetId,
    byte senderId,
    JsonObject& payload
) {
/*
    // Check if node is waiting for response
    if (_node_waiting_for_response == false) {
        if (
            packetId == COMMUNICATION_PACKET_INIT_START
            || packetId == COMMUNICATION_PACKET_INIT_END
            || packetId == COMMUNICATION_PACKET_HARDWARE_INFO
            || packetId == COMMUNICATION_PACKET_FIRMWARE_INFO
            || packetId == COMMUNICATION_PACKET_SETTINGS_SCHEMA
            || packetId == COMMUNICATION_PACKET_CHANNELS_SCHEMA
        ) {
            DPRINT(F("[NODE] Not waiting for acceptation packet\n"));
        }

        return;
    }

    DPRINT(F("[NODE] Packet: "));
    DPRINT(packetId);
    DPRINT(F(" was accepted\n"));

    switch (packetId) {
        // Master has accepted node configuration details
        case COMMUNICATION_PACKET_INIT_START:
        case COMMUNICATION_PACKET_INIT_END:
        case COMMUNICATION_PACKET_HARDWARE_INFO:
        case COMMUNICATION_PACKET_FIRMWARE_INFO:
        case COMMUNICATION_PACKET_SETTINGS_SCHEMA:
        case COMMUNICATION_PACKET_CHANNELS_SCHEMA:
            if (senderId == COMMUNICATION_BUS_UNKNOWN_ADDR || senderId != communicationMasterAddress()) {
                DPRINT(F("[NODE] Received packet is from unknown master\n"));

                break;
            }

            // Initialization start accepted
            if (packetId == COMMUNICATION_PACKET_INIT_START && _node_initialization_step == NODE_INIT_HARDWARE) {
                _nodePushBackInitilizationPacket(packetId, true);

                _node_waiting_for_response = false;

            // Initialization end accepted
            } else if (packetId == COMMUNICATION_PACKET_INIT_END && _node_initialization_step == NODE_INIT_WAIT) {
                _nodePushBackInitilizationPacket(packetId, true);

                _node_waiting_for_response = false;

                DPRINTLN();
                DPRINT(F("[NODE] ===========================================\n"));
                DPRINT(F("[NODE] Node was successfully initialized to master\n"));
                DPRINT(F("[NODE] ===========================================\n"));
                DPRINTLN();
                DPRINTLN();

                systemPongReceived();

            } else if (packetId == COMMUNICATION_PACKET_HARDWARE_INFO && _node_initialization_step == NODE_INIT_FIRMWARE) {
                _nodePushBackInitilizationPacket(packetId, true);

                _node_waiting_for_response = false;

            } else if (packetId == COMMUNICATION_PACKET_FIRMWARE_INFO && _node_initialization_step == NODE_INIT_SETTINGS) {
                _nodePushBackInitilizationPacket(packetId, true);

                _node_waiting_for_response = false;

            // Initialization buttons channels schema accepted
            } else if (
                packetId == COMMUNICATION_PACKET_CHANNELS_SCHEMA
                && (
                    _node_initialization_step == NODE_INIT_OUTPUTS
                    || _node_initialization_step == NODE_INIT_BUTTONS
                    || _node_initialization_step == NODE_INIT_SWITCHES
                    || _node_initialization_step == NODE_INIT_FINISHED
                )
            ) {
                _node_waiting_for_response = false;

            } else {
                DPRINT(F("[NODE] Received initializaton acceptation packet for unknown state\n"));
                DPRINT(F("[NODE] Node actual state is: "));
                DPRINTLN(_node_initialization_step);
            }
            break;

        default:
            DPRINT(F("[NODE] Received acceptation packet is uknown\n"));
            break;
    }
*/
}

// -----------------------------------------------------------------------------

void nodeResetInitialization() {
    communicationResetMasterAddress();

    _node_initialization_step = NODE_INIT_INIT;
    _node_waiting_for_response = false;

    _nodeResetInitializationPackets();
}

// -----------------------------------------------------------------------------

/**
 * Check if node has sent all init packets and master has accepted them
 */
bool nodeIsInitialized() {
/*
    if (
        // Check for init packet
        _nodeIsPacketInitialized(COMMUNICATION_PACKET_INIT_START)
        // Check for hardware info packet
        && _nodeIsPacketInitialized(COMMUNICATION_PACKET_HARDWARE_INFO)
        // Check for firmware info packet
        && _nodeIsPacketInitialized(COMMUNICATION_PACKET_FIRMWARE_INFO)
        // Check for settings schema packet
        && _nodeIsPacketInitialized(COMMUNICATION_PACKET_SETTINGS_SCHEMA)
        // Check for channels schema packet
        && _nodeIsPacketInitialized(COMMUNICATION_PACKET_CHANNELS_SCHEMA)
        // Check for init finished packet
        && _nodeIsPacketInitialized(COMMUNICATION_PACKET_INIT_END)
    ) {
        return true;
    }
*/
    return false;
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void nodeSetup() {
}

// -----------------------------------------------------------------------------

void nodeLoop() {
    // -------------------------------------------------------------------------
    // Initialization
    // -------------------------------------------------------------------------
    if (communicationConnected() && nodeIsInitialized() == false) {
        //_nodeInitialize();
    }
}
