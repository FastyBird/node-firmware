/*

FIRMWARE CONSTANTS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if DEBUG_SUPPORT

// =============================================================================
// PACKETS HUMAN READABLE TRANSLATIONS
// =============================================================================

    const char communication_packet_search_new_nodes[] PROGMEM          = "COMMUNICATION_PACKET_SEARCH_NEW_NODES";
    const char communication_packet_node_address_confirm[] PROGMEM      = "COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM";
    const char communication_packet_address_discard[] PROGMEM           = "COMMUNICATION_PACKET_ADDRESS_DISCARD";

    PROGMEM const char * const communication_packets_addresing_string[] = {
        communication_packet_search_new_nodes,communication_packet_node_address_confirm, communication_packet_address_discard
    };

    const char communication_packet_hw_model[] PROGMEM                  = "COMMUNICATION_PACKET_HW_MODEL";
    const char communication_packet_hw_manufacturer[] PROGMEM           = "COMMUNICATION_PACKET_HW_MANUFACTURER";
    const char communication_packet_hw_version[] PROGMEM                = "COMMUNICATION_PACKET_HW_VERSION";
    const char communication_packet_fw_model[] PROGMEM                  = "COMMUNICATION_PACKET_FW_MODEL";
    const char communication_packet_fw_manufacturer[] PROGMEM           = "COMMUNICATION_PACKET_FW_MANUFACTURER";
    const char communication_packet_fw_version[] PROGMEM                = "COMMUNICATION_PACKET_FW_VERSION";
    const char communication_packet_registers_size[] PROGMEM            = "COMMUNICATION_PACKET_REGISTERS_SIZE";
    const char communication_packet_ai_registers_structure[] PROGMEM    = "COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE";
    const char communication_packet_ao_registers_structure[] PROGMEM    = "COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE";

    PROGMEM const char * const communication_packets_node_initialization_string[] = {
        communication_packet_hw_model, communication_packet_hw_manufacturer, communication_packet_hw_version,
        communication_packet_fw_model, communication_packet_fw_manufacturer, communication_packet_fw_version,
        communication_packet_registers_size,
        communication_packet_ai_registers_structure, communication_packet_ao_registers_structure
    };

    const char communication_packet_read_single_di[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_DI";
    const char communication_packet_read_multi_di[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_DI";
    const char communication_packet_read_single_do[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_DO";
    const char communication_packet_read_multi_do[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_DO";
    const char communication_packet_read_single_ai[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_AI";
    const char communication_packet_read_multi_ai[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_AI";
    const char communication_packet_read_single_ao[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_AO";
    const char communication_packet_read_multi_ao[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_AO";
    const char communication_packet_read_single_ev[] PROGMEM            = "COMMUNICATION_PACKET_READ_SINGLE_EV";
    const char communication_packet_read_multi_ev[] PROGMEM             = "COMMUNICATION_PACKET_READ_MULTI_EV";

    PROGMEM const char * const communication_packets_registers_reading_string[] = {
        communication_packet_read_single_di, communication_packet_read_multi_di,
        communication_packet_read_single_do, communication_packet_read_multi_do,
        communication_packet_read_single_ai, communication_packet_read_multi_ai,
        communication_packet_read_single_ao, communication_packet_read_multi_ao,
        communication_packet_read_single_ev, communication_packet_read_multi_ev
    };

    const char communication_packet_write_one_do[] PROGMEM              = "COMMUNICATION_PACKET_WRITE_ONE_DO";
    const char communication_packet_write_one_ao[] PROGMEM              = "COMMUNICATION_PACKET_WRITE_ONE_AO";
    const char communication_packet_write_multi_do[] PROGMEM            = "COMMUNICATION_PACKET_WRITE_MULTI_DO";
    const char communication_packet_write_multi_ao[] PROGMEM            = "COMMUNICATION_PACKET_WRITE_MULTI_AO";

    PROGMEM const char * const communication_packets_registers_writing_string[] = {
        communication_packet_write_one_do, communication_packet_write_one_ao,
        communication_packet_write_multi_do, communication_packet_write_multi_ao
    };

    const char communication_packet_none[] PROGMEM                      = "COMMUNICATION_PACKET_NONE";

    PROGMEM const char * const communication_packets_misc_string[] = {
        communication_packet_none
    };

#endif

// =============================================================================
// PACKETS GROUPS
// =============================================================================

PROGMEM const int communication_packets_addresing[3] = {
    COMMUNICATION_PACKET_SEARCH_NEW_NODES, COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM, COMMUNICATION_PACKET_ADDRESS_DISCARD
};

PROGMEM const int communication_packets_node_initialization[9] = {
    COMMUNICATION_PACKET_HW_MODEL, COMMUNICATION_PACKET_HW_MANUFACTURER, COMMUNICATION_PACKET_HW_VERSION,
    COMMUNICATION_PACKET_FW_MODEL, COMMUNICATION_PACKET_FW_MANUFACTURER, COMMUNICATION_PACKET_FW_VERSION,
    COMMUNICATION_PACKET_REGISTERS_SIZE,
    COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE, COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE
};

PROGMEM const int communication_packets_registers_reading[10] = {
    COMMUNICATION_PACKET_READ_SINGLE_DI, COMMUNICATION_PACKET_READ_MULTI_DI,
    COMMUNICATION_PACKET_READ_SINGLE_DO, COMMUNICATION_PACKET_READ_MULTI_DO,
    COMMUNICATION_PACKET_READ_SINGLE_AI, COMMUNICATION_PACKET_READ_MULTI_AI,
    COMMUNICATION_PACKET_READ_SINGLE_AO, COMMUNICATION_PACKET_READ_MULTI_AO,
    COMMUNICATION_PACKET_READ_SINGLE_EV, COMMUNICATION_PACKET_READ_MULTI_EV
};

PROGMEM const int communication_packets_registers_writing[4] = {
    COMMUNICATION_PACKET_WRITE_ONE_DO, COMMUNICATION_PACKET_WRITE_ONE_AO,
    COMMUNICATION_PACKET_WRITE_MULTI_DO, COMMUNICATION_PACKET_WRITE_MULTI_AO
};

PROGMEM const int communication_packets_misc[1] = {
    COMMUNICATION_PACKET_NONE
};
