/*

FIRMWARE CONSTANTS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#if DEBUG_SUPPORT

// =============================================================================
// PACKETS HUMAN READABLE TRANSLATIONS
// =============================================================================

    const char communication_packet_none[] PROGMEM                  = "N/A";
    const char communication_packet_acquire_address[] PROGMEM       = "COMMUNICATION_PACKET_ACQUIRE_ADDRESS";
    const char communication_packet_gateway_ping[] PROGMEM          = "COMMUNICATION_PACKET_GATEWAY_PING";
    const char communication_packet_who_are_you[] PROGMEM           = "COMMUNICATION_PACKET_WHO_ARE_YOU";
    const char communication_packet_read_di[] PROGMEM               = "COMMUNICATION_PACKET_READ_DI";
    const char communication_packet_read_do[] PROGMEM               = "COMMUNICATION_PACKET_READ_DO";
    const char communication_packet_read_ai[] PROGMEM               = "COMMUNICATION_PACKET_READ_AI";
    const char communication_packet_read_ao[] PROGMEM               = "COMMUNICATION_PACKET_READ_AO";
    const char communication_packet_write_one_do[] PROGMEM          = "COMMUNICATION_PACKET_WRITE_ONE_DO";
    const char communication_packet_write_one_ao[] PROGMEM          = "COMMUNICATION_PACKET_WRITE_ONE_AO";
    const char communication_packet_write_multi_do[] PROGMEM        = "COMMUNICATION_PACKET_WRITE_MULTI_DO";
    const char communication_packet_write_multi_ao[] PROGMEM        = "COMMUNICATION_PACKET_WRITE_MULTI_AO";

    PROGMEM const char * const communication_packet_string[] = {
        communication_packet_none, communication_packet_acquire_address, communication_packet_gateway_ping, communication_packet_who_are_you,
        communication_packet_read_di, communication_packet_read_do, communication_packet_read_ai, communication_packet_read_ao,
        communication_packet_write_one_do, communication_packet_write_one_ao, communication_packet_write_multi_do, communication_packet_write_multi_ao
    };

// =============================================================================
// NODE ADDRESS ACQUIRE HUMAN READABLE TRANSLATIONS
// =============================================================================

    const char communication_address_acquire_none[] PROGMEM         = "N/A";
    const char communication_address_acquire_request[] PROGMEM      = "COMMUNICATION_ACQUIRE_ADDRESS_REQUEST";
    const char communication_address_acquire_confirm[] PROGMEM      = "COMMUNICATION_ACQUIRE_ADDRESS_CONFIRM";
    const char communication_address_acquire_refresh[] PROGMEM      = "COMMUNICATION_ACQUIRE_ADDRESS_REFRESH";
    const char communication_address_acquire_negate[] PROGMEM       = "COMMUNICATION_ACQUIRE_ADDRESS_NEGATE";
    const char communication_address_acquire_list[] PROGMEM         = "COMMUNICATION_ACQUIRE_ADDRESS_LIST";

    PROGMEM const char * const communication_address_acquire_string[] = {
        communication_address_acquire_none, communication_address_acquire_request, communication_address_acquire_confirm,
        communication_address_acquire_refresh, communication_address_acquire_negate, communication_address_acquire_list
    };

// =============================================================================
// NODE DESCRIPTION HUMAN READABLE TRANSLATIONS
// =============================================================================

    const char communication_describe_none[] PROGMEM                = "N/A";
    const char communication_describe_node[] PROGMEM                = "COMMUNICATION_DESCRIBE_NODE";
    const char communication_describe_sn[] PROGMEM                  = "COMMUNICATION_DESCRIBE_SN";
    const char communication_describe_hw_model[] PROGMEM            = "COMMUNICATION_DESCRIBE_HW_MODEL";
    const char communication_describe_hw_manufacturer[] PROGMEM     = "COMMUNICATION_DESCRIBE_HW_MANUFACTURER";
    const char communication_describe_hw_version[] PROGMEM          = "COMMUNICATION_DESCRIBE_HW_VERSION";
    const char communication_describe_fw_model[] PROGMEM            = "COMMUNICATION_DESCRIBE_FW_MODEL";
    const char communication_describe_fw_manufacturer[] PROGMEM     = "COMMUNICATION_DESCRIBE_FW_MANUFACTURER";
    const char communication_describe_fw_version[] PROGMEM          = "COMMUNICATION_DESCRIBE_FW_VERSION";
    const char communication_describe_registers[] PROGMEM           = "COMMUNICATION_DESCRIBE_REGISTERS_SIZE";

    PROGMEM const char * const communication_describe_string[] = {
        communication_describe_none, communication_describe_node, communication_describe_sn,
        communication_describe_hw_model, communication_describe_hw_manufacturer, communication_describe_hw_version,
        communication_describe_fw_model, communication_describe_fw_manufacturer, communication_describe_fw_version,
        communication_describe_registers
    };

#endif
