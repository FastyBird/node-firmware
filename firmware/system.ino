/*

SYSTEM MODULE

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// Heartbeat
bool _system_send_heartbeat = false;

unsigned long _system_last_heartbeat = 0;

// Ping&Pong
unsigned long _system_last_pong_reply = 0;
unsigned long _system_last_ping = 0;

bool _system_send_ping = false;

// -----------------------------------------------------------------------------
// MODULE PRIVATE
// -----------------------------------------------------------------------------

void _systemHeartbeat() {
/*
    if (communicationHasMasterAddress() == false) {
        DPRINT(F("[SYSTEM] Node is not connected to master, heartbeat could not be sent\n"));

        return;
    }

    if (nodeIsInitialized() == false) {
        DPRINT(F("[SYSTEM] Node is not initialized, heartbeat could not be sent\n"));

        return;
    }

    DPRINT(F("[SYSTEM] Node heartbeat\n"));

    char output_buffer[PJON_PACKET_MAX_LENGTH];
    sprintf(output_buffer, "{\"P\":%d,\"U\":%d}", COMMUNICATION_PACKET_HEARTBEAT, getUptime());

    // Send packet to master
    if (communicationSendPacket(COMMUNICATION_PACKET_HEARTBEAT, output_buffer) == false) {
        _system_send_heartbeat = true;
    }
*/
}

// -----------------------------------------------------------------------------

void _systemPing() {
/*
    if (communicationHasMasterAddress() == false) {
        DPRINT(F("[SYSTEM] Node is not connected to master, ping could not be sent\n"));

        return;
    }

    if (nodeIsInitialized() == false) {
        DPRINT(F("[SYSTEM] Node is not initialized, ping could not be sent\n"));

        return;
    }

    DPRINT(F("[SYSTEM] Node PING to master\n"));

    char output_buffer[PJON_PACKET_MAX_LENGTH];
    sprintf(output_buffer, "{\"P\":%d}", COMMUNICATION_PACKET_PING);

    // Send packet to master
    if (communicationSendPacket(COMMUNICATION_PACKET_PING, output_buffer)) {
        _system_last_pong_reply = 0;

    } else {
        _system_send_ping = true;
    }
*/
}

// -----------------------------------------------------------------------------

void _systemCheckPingPongResponse() {
    if (_system_last_ping > 0) {
        if (millis() - _system_last_ping > SYSTEM_MAX_PONG_DELAY && _system_last_pong_reply == 0) {
            nodeResetInitialization();

            DPRINT(F("[SYSTEM] Lost connection with master\n"));
            DPRINT(F("[SYSTEM] Last PING: "));
            DPRINTLN(_system_last_ping);
            DPRINT(F("[SYSTEM] Last PONG: "));
            DPRINTLN(_system_last_pong_reply);
        }
    }
}

// -----------------------------------------------------------------------------
// MODULE API
// -----------------------------------------------------------------------------

void systemResetCounters() {
    _system_last_ping = millis();
    _system_last_pong_reply = millis();

    _system_last_heartbeat = millis();
}

// -----------------------------------------------------------------------------

void systemPongReceived() {
    _system_last_pong_reply = millis();
}

// -----------------------------------------------------------------------------
// MODULE CORE
// -----------------------------------------------------------------------------

void systemSetup() {
}

// -----------------------------------------------------------------------------

void systemLoop() {
    if (communicationConnected() && nodeIsInitialized()) {
        // ---------------------------------------------------------------------
        // Heartbeat
        // ---------------------------------------------------------------------
        if (_system_send_heartbeat == true || (_system_last_heartbeat == 0) || ((millis() - _system_last_heartbeat) > SYSTEM_HEARTBEAT_INTERVAL)) {
            if (_system_last_heartbeat != 0) {
                _systemHeartbeat();
            }

            _system_send_heartbeat = false;
            _system_last_heartbeat = millis();
        }

        // ---------------------------------------------------------------------
        // Ping&Pong
        // ---------------------------------------------------------------------
        if (_system_send_ping == true || (_system_last_ping == 0) || ((millis() - _system_last_ping) > SYSTEM_PING_PONG_INTERVAL)) {
            if (_system_last_ping != 0) {
                _systemPing();

                relayStatus(0, !relayStatus(0));
            }

            _system_send_ping = false;
            _system_last_ping = millis();
        }

        //_systemCheckPingPongResponse();
    }
}