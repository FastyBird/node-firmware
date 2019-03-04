/*

DATA TYPES & CONSTANTS DEFINITION

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// COMMUNICATION PACKET
// =============================================================================

// Node addressing
#define COMMUNICATION_PACKET_SEARCH_NODES               0x01
#define COMMUNICATION_PACKET_SEARCH_NEW_NODES           0x02
#define COMMUNICATION_PACKET_NODE_ADDRESS_CONFIRM       0x03
#define COMMUNICATION_PACKET_ADDRESS_DISCARD            0x04

#define COMMUNICATION_PACKET_ADDRESS_MAX                4

// Node initialization
#define COMMUNICATION_PACKET_HW_MODEL                   0x11
#define COMMUNICATION_PACKET_HW_MANUFACTURER            0x12
#define COMMUNICATION_PACKET_HW_VERSION                 0x13
#define COMMUNICATION_PACKET_FW_MODEL                   0x14
#define COMMUNICATION_PACKET_FW_MANUFACTURER            0x15
#define COMMUNICATION_PACKET_FW_VERSION                 0x16

#define COMMUNICATION_PACKET_NODE_INIT_MAX              6

// Node registers initialization
#define COMMUNICATION_PACKET_REGISTERS_SIZE             0x21
#define COMMUNICATION_PACKET_DI_REGISTERS_STRUCTURE     0x22
#define COMMUNICATION_PACKET_DO_REGISTERS_STRUCTURE     0x23
#define COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE     0x24
#define COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE     0x25

#define COMMUNICATION_PACKET_REGISTERS_INIT_MAX         5

// Registers reading
#define COMMUNICATION_PACKET_READ_SINGLE_DI             0x31   // Master requested DI one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_DI              0x32   // Master requested DI multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_DO             0x33   // Master requested DO one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_DO              0x34   // Master requested DO multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_AI             0x35   // Master requested AI one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_AI              0x36   // Master requested AI multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_AO             0x37   // Master requested AO one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_AO              0x38   // Master requested AO multiple regiters reading

#define COMMUNICATION_PACKET_REGISTERS_REDING_MAX       8

// Registers writing
#define COMMUNICATION_PACKET_WRITE_ONE_DO               0x41
#define COMMUNICATION_PACKET_WRITE_ONE_AO               0x42
#define COMMUNICATION_PACKET_WRITE_MULTI_DO             0x43
#define COMMUNICATION_PACKET_WRITE_MULTI_AO             0x44

#define COMMUNICATION_PACKET_REGISTERS_WRITING_MAX      4

// Node misc communication
#define COMMUNICATION_PACKET_NONE                       0xFF
#define COMMUNICATION_PACKET_GATEWAY_PING               0x51   // Master is testing node if is alive

#define COMMUNICATION_PACKET_MISC_MAX                   2

// =============================================================================
// REGISTERS VALUES DATATYPES
// =============================================================================

#define COMMUNICATION_DATA_TYPE_UNKNOWN                 0xFF
#define COMMUNICATION_DATA_TYPE_BOOLEAN                 0x01
#define COMMUNICATION_DATA_TYPE_UINT8                   0x02
#define COMMUNICATION_DATA_TYPE_UINT16                  0x03
#define COMMUNICATION_DATA_TYPE_UINT32                  0x04
#define COMMUNICATION_DATA_TYPE_INT8                    0x05
#define COMMUNICATION_DATA_TYPE_INT16                   0x06
#define COMMUNICATION_DATA_TYPE_INT32                   0x07
#define COMMUNICATION_DATA_TYPE_FLOAT32                 0x08

// =============================================================================
// LED
// =============================================================================

#define LED_MODE_BUS                                    0       // LED will blink according to the BUS status
#define LED_MODE_FINDME                                 1       // LED will be ON
#define LED_MODE_FINDME_BUS                             2       // A mixture between BUS and FINDME
#define LED_MODE_ON                                     3       // LED always ON
#define LED_MODE_OFF                                    4       // LED always OFF

// =============================================================================
// BUTTONS EVENTS
// =============================================================================

#define BUTTON_EVENT_NONE                               0
#define BUTTON_EVENT_PRESSED                            1
#define BUTTON_EVENT_CLICK                              2
#define BUTTON_EVENT_DBLCLICK                           3
#define BUTTON_EVENT_LNGCLICK                           4
#define BUTTON_EVENT_LNGLNGCLICK                        5
#define BUTTON_EVENT_TRIPLECLICK                        6

// =============================================================================
// RELAY
// =============================================================================

#define RELAY_BOOT_OFF                                  0
#define RELAY_BOOT_ON                                   1
#define RELAY_BOOT_SAME                                 2
#define RELAY_BOOT_TOGGLE                               3

#define RELAY_TYPE_NORMAL                               0
#define RELAY_TYPE_INVERSE                              1
#define RELAY_TYPE_LATCHED                              2
#define RELAY_TYPE_LATCHED_INVERSE                      3

#define RELAY_SYNC_ANY                                  0
#define RELAY_SYNC_NONE_OR_ONE                          1
#define RELAY_SYNC_ONE                                  2
#define RELAY_SYNC_SAME                                 3

#define RELAY_PULSE_NONE                                0
#define RELAY_PULSE_OFF                                 1
#define RELAY_PULSE_ON                                  2

#define RELAY_PROVIDER_NONE                             0
#define RELAY_PROVIDER_RELAY                            1
