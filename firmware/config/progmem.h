/*

FIRMWARE CONSTANTS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if DEBUG_SUPPORT

// =============================================================================
// PACKETS HUMAN READABLE TRANSLATIONS
// =============================================================================

    const char communication_packet_search_nodes[] PROGMEM              = "COMMUNICATION_PACKET_SEARCH_NODES";
    const char communication_packet_node_address_confirm[] PROGMEM      = "COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM";
    const char communication_packet_address_discard[] PROGMEM           = "COMMUNICATION_PACKET_ADDRESS_DISCARD";

    PROGMEM const char * const communication_packets_addresing_string[] = {
        communication_packet_search_nodes, communication_packet_node_address_confirm, communication_packet_address_discard
    };

    const char communication_packet_hw_model[] PROGMEM                  = "COMMUNICATION_PACKET_HW_MODEL";
    const char communication_packet_hw_manufacturer[] PROGMEM           = "COMMUNICATION_PACKET_HW_MANUFACTURER";
    const char communication_packet_hw_version[] PROGMEM                = "COMMUNICATION_PACKET_HW_VERSION";
    const char communication_packet_fw_model[] PROGMEM                  = "COMMUNICATION_PACKET_FW_MODEL";
    const char communication_packet_fw_manufacturer[] PROGMEM           = "COMMUNICATION_PACKET_FW_MANUFACTURER";
    const char communication_packet_fw_version[] PROGMEM                = "COMMUNICATION_PACKET_FW_VERSION";

    PROGMEM const char * const communication_packets_node_initialization_string[] = {
        communication_packet_hw_model, communication_packet_hw_manufacturer, communication_packet_hw_version,
        communication_packet_fw_model, communication_packet_fw_manufacturer, communication_packet_fw_version
    };

    const char communication_packet_registers_size[] PROGMEM            = "COMMUNICATION_PACKET_REGISTERS_SIZE";
    const char communication_packet_di_registers_structure[] PROGMEM    = "COMMUNICATION_PACKET_DI_REGISTERS_STRUCTURE";
    const char communication_packet_do_registers_structure[] PROGMEM    = "COMMUNICATION_PACKET_DO_REGISTERS_STRUCTURE";
    const char communication_packet_ai_registers_structure[] PROGMEM    = "COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE";
    const char communication_packet_ao_registers_structure[] PROGMEM    = "COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE";

    PROGMEM const char * const communication_packets_registers_initialization_string[] = {
        communication_packet_registers_size,
        communication_packet_di_registers_structure, communication_packet_do_registers_structure,
        communication_packet_ai_registers_structure, communication_packet_ao_registers_structure
    };

    const char communication_packet_read_single_di[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_DI";
    const char communication_packet_read_multi_di[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_DI";
    const char communication_packet_read_single_do[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_DO";
    const char communication_packet_read_multi_do[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_DO";
    const char communication_packet_read_ai[] PROGMEM                   = "COMMUNICATION_PACKET_READ_AI";
    const char communication_packet_read_ao[] PROGMEM                   = "COMMUNICATION_PACKET_READ_AO";

    PROGMEM const char * const communication_packets_registers_reading_string[] = {
        communication_packet_read_single_di, communication_packet_read_multi_di,
        communication_packet_read_single_do, communication_packet_read_multi_do,
        communication_packet_read_ai, communication_packet_read_ao
    };

    const char communication_packet_write_one_do[] PROGMEM              = "COMMUNICATION_PACKET_WRITE_ONE_DO";
    const char communication_packet_write_one_ao[] PROGMEM              = "COMMUNICATION_PACKET_WRITE_ONE_AO";
    const char communication_packet_write_multi_do[] PROGMEM            = "COMMUNICATION_PACKET_WRITE_MULTI_DO";

    PROGMEM const char * const communication_packets_registers_writing_string[] = {
        communication_packet_write_one_do, communication_packet_write_one_ao, communication_packet_write_multi_do
    };

    const char communication_packet_none[] PROGMEM                      = "COMMUNICATION_PACKET_NONE";
    const char communication_packet_ping[] PROGMEM                      = "COMMUNICATION_PACKET_GATEWAY_PING";

    PROGMEM const char * const communication_packets_misc_string[] = {
        communication_packet_none, communication_packet_ping
    };

#endif

// =============================================================================
// PACKETS GROUPS
// =============================================================================

PROGMEM const int communication_packets_addresing[3] = {
    COMMUNICATION_PACKET_SEARCH_NODES, COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM, COMMUNICATION_PACKET_ADDRESS_DISCARD
};

PROGMEM const int communication_packets_node_initialization[6] = {
    COMMUNICATION_PACKET_HW_MODEL, COMMUNICATION_PACKET_HW_MANUFACTURER, COMMUNICATION_PACKET_HW_VERSION,
    COMMUNICATION_PACKET_FW_MODEL, COMMUNICATION_PACKET_FW_MANUFACTURER, COMMUNICATION_PACKET_FW_VERSION
};

PROGMEM const int communication_packets_registers_initialization[5] = {
    COMMUNICATION_PACKET_REGISTERS_SIZE,
    COMMUNICATION_PACKET_DI_REGISTERS_STRUCTURE, COMMUNICATION_PACKET_DO_REGISTERS_STRUCTURE,
    COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE, COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE
};

PROGMEM const int communication_packets_registers_reading[6] = {
    COMMUNICATION_PACKET_READ_SINGLE_DI, COMMUNICATION_PACKET_READ_MULTI_DI,
    COMMUNICATION_PACKET_READ_SINGLE_DO, COMMUNICATION_PACKET_READ_MULTI_DO,
    COMMUNICATION_PACKET_READ_AI, COMMUNICATION_PACKET_READ_AO
};

PROGMEM const int communication_packets_registers_writing[3] = {
    COMMUNICATION_PACKET_WRITE_ONE_DO, COMMUNICATION_PACKET_WRITE_ONE_AO, COMMUNICATION_PACKET_WRITE_MULTI_DO
};

PROGMEM const int communication_packets_misc[2] = {
    COMMUNICATION_PACKET_NONE, COMMUNICATION_PACKET_GATEWAY_PING
};
