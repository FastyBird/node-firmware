/*

DATA TYPES & CONSTANTS DEFINITION

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// COMMUNICATION PACKET
// =============================================================================

#define COMMUNICATION_PACKET_NONE               0
#define COMMUNICATION_PACKET_ACQUIRE_ADDRESS    1
#define COMMUNICATION_PACKET_GATEWAY_PING       2   // Master is testing node if is alive
#define COMMUNICATION_PACKET_WHO_ARE_YOU        3   // Master want to know slave details
#define COMMUNICATION_PACKET_READ_DI            4   // Master requested DI regiter reading
#define COMMUNICATION_PACKET_READ_DO            5   // Master requested DO regiter reading
#define COMMUNICATION_PACKET_READ_AI            6   // Master requested AI regiter reading
#define COMMUNICATION_PACKET_READ_AO            7   // Master requested AO regiter reading
#define COMMUNICATION_PACKET_WRITE_ONE_DO       8
#define COMMUNICATION_PACKET_WRITE_ONE_AO       9
#define COMMUNICATION_PACKET_WRITE_MULTI_DO     10
#define COMMUNICATION_PACKET_WRITE_MULTI_AO     11

#define COMMUNICATION_PACKETS_MAX               12

// =============================================================================
// NODE ADDRESS ACQUISION
// =============================================================================

#define COMMUNICATION_ACQUIRE_ADDRESS_NONE      0
#define COMMUNICATION_ACQUIRE_ADDRESS_REQUEST   1
#define COMMUNICATION_ACQUIRE_ADDRESS_CONFIRM   2
#define COMMUNICATION_ACQUIRE_ADDRESS_REFRESH   3
#define COMMUNICATION_ACQUIRE_ADDRESS_NEGATE    4
#define COMMUNICATION_ACQUIRE_ADDRESS_LIST      5

#define COMMUNICATION_ACQUIRE_ADDRESS_MAX       6

// =============================================================================
// NODE DESCRIBE ITSELF
// =============================================================================

#define COMMUNICATION_DESCRIBE_NONE             0
#define COMMUNICATION_DESCRIBE_NODE             1   // Send node basic structure to master
#define COMMUNICATION_DESCRIBE_SN               2   // Send node SN to master
#define COMMUNICATION_DESCRIBE_HW_MODEL         3   // Send hardware model info to master
#define COMMUNICATION_DESCRIBE_HW_MANUFACTURER  4   // Send hardware manufacturer info to master
#define COMMUNICATION_DESCRIBE_HW_VERSION       5   // Send hardware version info to master
#define COMMUNICATION_DESCRIBE_FW_MODEL         6   // Send firmware model info to master
#define COMMUNICATION_DESCRIBE_FW_MANUFACTURER  7   // Send firmware manufacturer info to master
#define COMMUNICATION_DESCRIBE_FW_VERSION       8   // Send firmware version info to master
#define COMMUNICATION_DESCRIBE_REGISTERS_SIZE   9

#define COMMUNICATION_DESCRIBES_MAX             10

// =============================================================================
// LED
// =============================================================================

#define LED_MODE_BUS                            0       // LED will blink according to the BUS status
#define LED_MODE_FINDME                         1       // LED will be ON
#define LED_MODE_FINDME_BUS                     2       // A mixture between BUS and FINDME
#define LED_MODE_ON                             3       // LED always ON
#define LED_MODE_OFF                            4       // LED always OFF

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
