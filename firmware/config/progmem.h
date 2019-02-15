//--------------------------------------------------------------------------------
// PROGMEM definitions
//--------------------------------------------------------------------------------

const char communication_packet_none[] PROGMEM              = "COMMUNICATION_PACKET_NONE";
const char communication_packet_unknown[] PROGMEM           = "COMMUNICATION_PACKET_UNKNOWN";
const char communication_packet_error[] PROGMEM             = "COMMUNICATION_PACKET_ERROR";
const char communication_packet_accepted[] PROGMEM          = "COMMUNICATION_PACKET_ACCEPTED";
const char communication_packet_master_lookup[] PROGMEM     = "COMMUNICATION_PACKET_MASTER_LOOKUP";
const char communication_packet_heartbeat[] PROGMEM         = "COMMUNICATION_PACKET_HEARTBEAT";
const char communication_packet_ping[] PROGMEM              = "COMMUNICATION_PACKET_PING";
const char communication_packet_pong[] PROGMEM              = "COMMUNICATION_PACKET_PONG";
const char communication_packet_data[] PROGMEM              = "COMMUNICATION_PACKET_DATA";
const char communication_packet_init_start[] PROGMEM        = "COMMUNICATION_PACKET_INIT_START";
const char communication_packet_init_end[] PROGMEM          = "COMMUNICATION_PACKET_INIT_END";
const char communication_packet_init_restart[] PROGMEM      = "COMMUNICATION_PACKET_INIT_RESTART";
const char communication_packet_hw_info[] PROGMEM           = "COMMUNICATION_PACKET_HARDWARE_INFO";
const char communication_packet_fw_info[] PROGMEM           = "COMMUNICATION_PACKET_FIRMWARE_INFO";
const char communication_packet_settings_schema[] PROGMEM   = "COMMUNICATION_PACKET_SETTINGS_SCHEMA";
const char communication_packet_channels_schema[] PROGMEM   = "COMMUNICATION_PACKET_CHANNELS_SCHEMA";
const char communication_packet_who_are_you[] PROGMEM       = "COMMUNICATION_PACKET_WHO_ARE_YOU";

PROGMEM const char* const communication_packet_string[] = {
    communication_packet_none, communication_packet_unknown, communication_packet_error, communication_packet_accepted,
    communication_packet_master_lookup,
    communication_packet_heartbeat, communication_packet_ping, communication_packet_pong, communication_packet_data,
    communication_packet_init_start, communication_packet_init_end, communication_packet_init_restart,
    communication_packet_hw_info, communication_packet_fw_info,
    communication_packet_settings_schema, communication_packet_channels_schema,
    communication_packet_who_are_you
};
