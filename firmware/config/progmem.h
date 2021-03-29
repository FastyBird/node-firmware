/*

FIRMWARE CONSTANTS

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#if DEBUG_COMMUNICATION_SUPPORT

// =============================================================================
// PACKETS HUMAN READABLE TRANSLATIONS
// =============================================================================

    const char communication_packet_pair_device[] PROGMEM               = "COMMUNICATION_PACKET_PAIR_DEVICE";

    const char communication_packet_read_single_register[] PROGMEM      = "COMMUNICATION_PACKET_READ_SINGLE_REGISTER";
    const char communication_packet_read_multiple_registers[] PROGMEM   = "COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS";

    const char communication_packet_write_single_register[] PROGMEM     = "COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER";
    const char communication_packet_write_multiple_registers[] PROGMEM  = "COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS";

    const char communication_packet_report_single_register[] PROGMEM    = "COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER";

    const char communication_packet_read_one_configuration[] PROGMEM    = "COMMUNICATION_PACKET_READ_ONE_CONFIGURATION";
    const char communication_packet_write_one_configuration[] PROGMEM   = "COMMUNICATION_PACKET_WRITE_ONE_CONFIGURATION";
    const char communication_packet_report_one_configuration[] PROGMEM  = "COMMUNICATION_PACKET_REPORT_ONE_CONFIGURATION";

    const char communication_packet_ping[] PROGMEM                      = "COMMUNICATION_PACKET_PING";
    const char communication_packet_pong[] PROGMEM                      = "COMMUNICATION_PACKET_PONG";
    const char communication_packet_hello[] PROGMEM                     = "COMMUNICATION_PACKET_HELLO";

    const char communication_packet_get_state[] PROGMEM                 = "COMMUNICATION_PACKET_GET_STATE";
    const char communication_packet_set_state[] PROGMEM                 = "COMMUNICATION_PACKET_SET_STATE";
    const char communication_packet_report_state[] PROGMEM              = "COMMUNICATION_PACKET_REPORT_STATE";

    const char communication_packet_pubsub_broadcast[] PROGMEM          = "COMMUNICATION_PACKET_PUBSUB_BROADCAST";
    const char communication_packet_pubsub_subscribe[] PROGMEM          = "COMMUNICATION_PACKET_PUBSUB_SUBSCRIBE";
    const char communication_packet_pubsub_unsubscribe[] PROGMEM        = "COMMUNICATION_PACKET_PUBSUB_UNSUBSCRIBE";

    const char communication_packet_exception[] PROGMEM                 = "COMMUNICATION_PACKET_EXCEPTION";

    PROGMEM const char * const communication_packets_string[COMMUNICATION_PACKET_MAX] = {
        communication_packet_pair_device,
        communication_packet_read_single_register, communication_packet_read_multiple_registers,
        communication_packet_write_single_register, communication_packet_write_multiple_registers,
        communication_packet_report_single_register,
        communication_packet_read_one_configuration, communication_packet_write_one_configuration, communication_packet_report_one_configuration,
        communication_packet_ping, communication_packet_pong, communication_packet_hello,
        communication_packet_get_state, communication_packet_set_state, communication_packet_report_state,
        communication_packet_pubsub_broadcast, communication_packet_pubsub_subscribe, communication_packet_pubsub_unsubscribe,
        communication_packet_exception
    };


// =============================================================================
// PACKETS GROUPS
// =============================================================================

    PROGMEM const uint8_t communication_packets[COMMUNICATION_PACKET_MAX] = {
        COMMUNICATION_PACKET_PAIR_DEVICE,
        COMMUNICATION_PACKET_READ_SINGLE_REGISTER, COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS,
        COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER, COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS,
        COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER,
        COMMUNICATION_PACKET_READ_ONE_CONFIGURATION, COMMUNICATION_PACKET_WRITE_ONE_CONFIGURATION, COMMUNICATION_PACKET_REPORT_ONE_CONFIGURATION,
        COMMUNICATION_PACKET_PING, COMMUNICATION_PACKET_PONG, COMMUNICATION_PACKET_HELLO,
        COMMUNICATION_PACKET_GET_STATE, COMMUNICATION_PACKET_SET_STATE, COMMUNICATION_PACKET_REPORT_STATE,
        COMMUNICATION_PACKET_PUBSUB_BROADCAST, COMMUNICATION_PACKET_PUBSUB_SUBSCRIBE, COMMUNICATION_PACKET_PUBSUB_UNSUBSCRIBE,
        COMMUNICATION_PACKET_EXCEPTION
    };

#endif
