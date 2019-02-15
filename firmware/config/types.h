/*

DATA TYPES & CONSTANTS DEFINITION

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// COMMUNICATION PACKET
// =============================================================================

#define COMMUNICATION_PACKET_NONE               0
#define COMMUNICATION_PACKET_MASTER_LOOKUP      1   // Master is searching for slaves
#define COMMUNICATION_PACKET_WHO_ARE_YOU        2   // Master want to know slave details
#define COMMUNICATION_PACKET_READ_DI            3   // Master requested DI regiter reading
#define COMMUNICATION_PACKET_READ_DO            4   // Master requested DO regiter reading
#define COMMUNICATION_PACKET_READ_AI            5   // Master requested AI regiter reading
#define COMMUNICATION_PACKET_READ_AO            6   // Master requested AO regiter reading
#define COMMUNICATION_PACKET_WRITE_ONE_DO       7
#define COMMUNICATION_PACKET_WRITE_ONE_AO       8
#define COMMUNICATION_PACKET_WRITE_MULTI_DO     9
#define COMMUNICATION_PACKET_WRITE_MULTI_AO     10

#define COMMUNICATION_PACKETS_MAX               11

// =============================================================================
// NODE DESCRIBE ITSELF
// =============================================================================

#define COMMUNICATION_DESCRIBE_NODE             1   // Send node SN to master
#define COMMUNICATION_DESCRIBE_HARDWARE         2   // Send hardware info to master
#define COMMUNICATION_DESCRIBE_FIRMWARE         3   // Send firmware info to master
#define COMMUNICATION_DESCRIBE_REGISTERS_SIZE   4
#define COMMUNICATION_DESCRIBE_DI_SIZE          5   // Send DI registers length
#define COMMUNICATION_DESCRIBE_DO_SIZE          6   // Send DO registers length
#define COMMUNICATION_DESCRIBE_AI_SIZE          7   // Send AI registers length
#define COMMUNICATION_DESCRIBE_AO_SIZE          8   // Send AO registers length

#define COMMUNICATION_DESCRIBES_MAX             9

// =============================================================================
// NODE INITIALIZATION STEPS
// =============================================================================

#define NODE_INIT_INIT                          0
#define NODE_INIT_HARDWARE                      1
#define NODE_INIT_FIRMWARE                      2
#define NODE_INIT_SETTINGS                      3
#define NODE_INIT_INPUTS                        4
#define NODE_INIT_OUTPUTS                       5
#define NODE_INIT_BUTTONS                       6
#define NODE_INIT_SWITCHES                      7
#define NODE_INIT_FINISHED                      8
#define NODE_INIT_WAIT                          9

// =============================================================================
// NODE CHANNELS TYPES
// =============================================================================

#define NODE_CHANNEL_TYPE_BUTTON                1
#define NODE_CHANNEL_TYPE_INPUT                 2
#define NODE_CHANNEL_TYPE_OUTPUT                3
#define NODE_CHANNEL_TYPE_SWITCH                4

// =============================================================================
// NODE CHANNELS FORMATS
// =============================================================================

#define NODE_CHANNEL_FORMAT_ANALOG              1       // Input or Output value is FLOAT
#define NODE_CHANNEL_FORMAT_DIGITAL             2       // Input or Output value is BOOLEAN
#define NODE_CHANNEL_FORMAT_NUMERIC             3       // Input or Output value is INTEGER
#define NODE_CHANNEL_FORMAT_EVENT               4       // Input or Output value is CHAR

// =============================================================================
// BUTTONS EVENTS
// =============================================================================

#define BUTTON_EVENT_NONE                       0
#define BUTTON_EVENT_PRESSED                    1
#define BUTTON_EVENT_CLICK                      2
#define BUTTON_EVENT_DBLCLICK                   3
#define BUTTON_EVENT_LNGCLICK                   4
#define BUTTON_EVENT_LNGLNGCLICK                5
#define BUTTON_EVENT_TRIPLECLICK                6

// =============================================================================
// LED
// =============================================================================

#define LED_MODE_BUS                            0       // LED will blink according to the BUS status
#define LED_MODE_FINDME                         1       // LED will be ON
#define LED_MODE_FINDME_BUS                     2       // A mixture between BUS and FINDME
#define LED_MODE_ON                             3       // LED always ON
#define LED_MODE_OFF                            4       // LED always OFF

// =============================================================================
// RELAY
// =============================================================================

#define RELAY_BOOT_OFF                          0
#define RELAY_BOOT_ON                           1
#define RELAY_BOOT_SAME                         2
#define RELAY_BOOT_TOGGLE                       3

#define RELAY_TYPE_NORMAL                       0
#define RELAY_TYPE_INVERSE                      1
#define RELAY_TYPE_LATCHED                      2
#define RELAY_TYPE_LATCHED_INVERSE              3

#define RELAY_SYNC_ANY                          0
#define RELAY_SYNC_NONE_OR_ONE                  1
#define RELAY_SYNC_ONE                          2
#define RELAY_SYNC_SAME                         3

#define RELAY_PULSE_NONE                        0
#define RELAY_PULSE_OFF                         1
#define RELAY_PULSE_ON                          2

#define RELAY_PROVIDER_NONE                     0
#define RELAY_PROVIDER_RELAY                    1
