/*

DATA TYPES & CONSTANTS DEFINITION

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// FLASH MEMORY ADDRESSING
// =============================================================================

#define FLASH_ADDRESS_NODE_ADDRESS                      0x01
#define FLASH_ADDRESS_RELAY_01                          0x10
#define FLASH_ADDRESS_RELAY_02                          0x11
#define FLASH_ADDRESS_RELAY_03                          0x12
#define FLASH_ADDRESS_RELAY_04                          0x14
#define FLASH_ADDRESS_RELAY_05                          0x15
#define FLASH_ADDRESS_RELAY_06                          0x16
#define FLASH_ADDRESS_RELAY_07                          0x17
#define FLASH_ADDRESS_RELAY_08                          0x18

// =============================================================================
// COMMUNICATION PACKET
// =============================================================================

// Node searching
#define COMMUNICATION_PACKET_SEARCH_NODES               0x01
#define COMMUNICATION_PACKET_SEARCH_NODES_CONFIRM       0x02
#define COMMUNICATION_PACKET_ADDRESS                    0x03
#define COMMUNICATION_PACKET_ADDRESS_CONFIRM            0x04
#define COMMUNICATION_PACKET_ADDRESS_DISCARD            0x05

#define COMMUNICATION_PACKET_ADDRESS_MAX                5

// Node initialization
#define COMMUNICATION_PACKET_HW_MODEL                   0x11
#define COMMUNICATION_PACKET_HW_MANUFACTURER            0x12
#define COMMUNICATION_PACKET_HW_VERSION                 0x13
#define COMMUNICATION_PACKET_FW_MODEL                   0x14
#define COMMUNICATION_PACKET_FW_MANUFACTURER            0x15
#define COMMUNICATION_PACKET_FW_VERSION                 0x16
#define COMMUNICATION_PACKET_REGISTERS_SIZE             0x17
#define COMMUNICATION_PACKET_AI_REGISTERS_STRUCTURE     0x18
#define COMMUNICATION_PACKET_AO_REGISTERS_STRUCTURE     0x19

#define COMMUNICATION_PACKET_NODE_INIT_MAX              9

// Registers reading
#define COMMUNICATION_PACKET_READ_SINGLE_DI             0x21   // Master requested DI one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_DI              0x22   // Master requested DI multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_DO             0x23   // Master requested DO one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_DO              0x24   // Master requested DO multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_AI             0x25   // Master requested AI one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_AI              0x26   // Master requested AI multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_AO             0x27   // Master requested AO one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_AO              0x28   // Master requested AO multiple regiters reading
#define COMMUNICATION_PACKET_READ_SINGLE_EV             0x29   // Master requested EV one regiter reading
#define COMMUNICATION_PACKET_READ_MULTI_EV              0x2A   // Master requested EV multiple regiters reading

#define COMMUNICATION_PACKET_REGISTERS_REDING_MAX       10

// Registers writing
#define COMMUNICATION_PACKET_WRITE_ONE_DO               0x31
#define COMMUNICATION_PACKET_WRITE_ONE_AO               0x32
#define COMMUNICATION_PACKET_WRITE_MULTI_DO             0x33
#define COMMUNICATION_PACKET_WRITE_MULTI_AO             0x34

#define COMMUNICATION_PACKET_REGISTERS_WRITING_MAX      4

// Node misc communication
#define COMMUNICATION_PACKET_PING                       0x41
#define COMMUNICATION_PACKET_PONG                       0x42
#define COMMUNICATION_PACKET_HELLO                      0x43
#define COMMUNICATION_PACKET_NONE                       0xFF

#define COMMUNICATION_PACKET_MISC_MAX                   4

// =============================================================================
// REGISTERS VALUES DATATYPES
// =============================================================================

#define COMMUNICATION_DATA_TYPE_UNKNOWN                 0xFF
#define COMMUNICATION_DATA_TYPE_UINT8                   0x01
#define COMMUNICATION_DATA_TYPE_UINT16                  0x02
#define COMMUNICATION_DATA_TYPE_UINT32                  0x03
#define COMMUNICATION_DATA_TYPE_INT8                    0x04
#define COMMUNICATION_DATA_TYPE_INT16                   0x05
#define COMMUNICATION_DATA_TYPE_INT32                   0x06
#define COMMUNICATION_DATA_TYPE_FLOAT32                 0x07

// =============================================================================
// BUTTON EVENT MAPPING
// =============================================================================

#define COMMUNICATION_BUTTON_EVENT_NONE                 0
#define COMMUNICATION_BUTTON_EVENT_PRESSED              1
#define COMMUNICATION_BUTTON_EVENT_CLICK                2
#define COMMUNICATION_BUTTON_EVENT_DBLCLICK             3
#define COMMUNICATION_BUTTON_EVENT_TRIPLECLICK          4
#define COMMUNICATION_BUTTON_EVENT_LNGCLICK             5
#define COMMUNICATION_BUTTON_EVENT_LNGLNGCLICK          6

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
#define BUTTON_EVENT_RELEASED                           2
#define BUTTON_EVENT_CLICK                              3
#define BUTTON_EVENT_DBLCLICK                           4
#define BUTTON_EVENT_TRIPLECLICK                        5
#define BUTTON_EVENT_LNGCLICK                           6
#define BUTTON_EVENT_LNGLNGCLICK                        7

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
