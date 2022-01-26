/*

DATA TYPES & CONSTANTS DEFINITION

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

// =============================================================================
// Hardware default values
// =============================================================================

#define GPIO_NONE                                                   0x99
#define INDEX_NONE                                                  0xFF

// =============================================================================
// FLASH MEMORY ADDRESSING
// =============================================================================

#define FLASH_ADDRESS_DEVICE_ADDRESS                                0x01
#define FLASH_ADDRESS_DEVICE_STATE                                  0x02

#define FLASH_ADDRESS_RELAY_01                                      0x10
#define FLASH_ADDRESS_RELAY_02                                      0x11
#define FLASH_ADDRESS_RELAY_03                                      0x12
#define FLASH_ADDRESS_RELAY_04                                      0x13
#define FLASH_ADDRESS_RELAY_05                                      0x14
#define FLASH_ADDRESS_RELAY_06                                      0x15
#define FLASH_ADDRESS_RELAY_07                                      0x16
#define FLASH_ADDRESS_RELAY_08                                      0x17
#define FLASH_ADDRESS_RELAY_09                                      0x18
#define FLASH_ADDRESS_RELAY_10                                      0x19
#define FLASH_ADDRESS_RELAY_11                                      0x1A
#define FLASH_ADDRESS_RELAY_12                                      0x1B
#define FLASH_ADDRESS_RELAY_13                                      0x1C
#define FLASH_ADDRESS_RELAY_14                                      0x1D
#define FLASH_ADDRESS_RELAY_15                                      0x1E
#define FLASH_ADDRESS_RELAY_16                                      0x1F

// =============================================================================
// DEVICE STATES
// =============================================================================

#define DEVICE_STATE_RUNNING                                        0x01
#define DEVICE_STATE_STOPPED                                        0x02
#define DEVICE_STATE_PAIRING                                        0x03
#define DEVICE_STATE_ERROR                                          0x0A
#define DEVICE_STATE_STOPPED_BY_OPERATOR                            0x0B

// =============================================================================
// COMMUNICATION
// =============================================================================

#define COMMUNICATION_PACKET_TERMINATOR                             0x00
#define COMMUNICATION_PACKET_DATA_SPACE                             0x20

#define COMMUNICATION_PACKET_PING                                   0x01
#define COMMUNICATION_PACKET_PONG                                   0x02
#define COMMUNICATION_PACKET_EXCEPTION                              0x03
#define COMMUNICATION_PACKET_DISCOVER                               0x04

#define COMMUNICATION_PACKET_READ_SINGLE_REGISTER_VALUES            0x21
#define COMMUNICATION_PACKET_READ_MULTIPLE_REGISTERS_VALUES         0x22
#define COMMUNICATION_PACKET_WRITE_SINGLE_REGISTER_VALUE            0x23
#define COMMUNICATION_PACKET_WRITE_MULTIPLE_REGISTERS_VALUES        0x24
#define COMMUNICATION_PACKET_READ_SINGLE_REGISTER_STRUCTURE         0x25
#define COMMUNICATION_PACKET_READ_MULTIPLE_REGISTER_STRUCTURE       0x26
#define COMMUNICATION_PACKET_REPORT_SINGLE_REGISTER_VALUE           0x27

// =============================================================================
// REGISTER
// =============================================================================

#define REGISTER_TYPE_INPUT                                         0x01
#define REGISTER_TYPE_OUTPUT                                        0x02
#define REGISTER_TYPE_ATTRIBUTE                                     0x03

// =============================================================================
// REGISTER VALUES CONSTANTS
// =============================================================================

#define REGISTER_BOOLEAN_VALUE_TRUE                                 0xFF00
#define REGISTER_BOOLEAN_VALUE_FALSE                                0x0000

// =============================================================================
// REGISTER DATATYPES
// =============================================================================

#define REGISTER_DATA_TYPE_UNKNOWN                                  0xFF
#define REGISTER_DATA_TYPE_UINT8                                    0x01
#define REGISTER_DATA_TYPE_UINT16                                   0x02
#define REGISTER_DATA_TYPE_UINT32                                   0x03
#define REGISTER_DATA_TYPE_INT8                                     0x04
#define REGISTER_DATA_TYPE_INT16                                    0x05
#define REGISTER_DATA_TYPE_INT32                                    0x06
#define REGISTER_DATA_TYPE_FLOAT32                                  0x07
#define REGISTER_DATA_TYPE_BOOLEAN                                  0x08
#define REGISTER_DATA_TYPE_TIME                                     0x09
#define REGISTER_DATA_TYPE_DATE                                     0x0A
#define REGISTER_DATA_TYPE_DATETIME                                 0x0B
#define REGISTER_DATA_TYPE_STRING                                   0x0C
#define REGISTER_DATA_TYPE_BUTTON                                   0x0D
#define REGISTER_DATA_TYPE_SWITCH                                   0x0E

// =============================================================================
// LED
// =============================================================================

#define LED_MODE_FINDME                                             1       // LED will blink
#define LED_MODE_ON                                                 2       // LED always ON
#define LED_MODE_OFF                                                3       // LED always OFF

// =============================================================================
// BUTTON
// =============================================================================

#define BUTTON_EVENT_NONE                                           0
#define BUTTON_EVENT_PRESSED                                        1
#define BUTTON_EVENT_RELEASED                                       2
#define BUTTON_EVENT_CLICK                                          3
#define BUTTON_EVENT_DBLCLICK                                       4
#define BUTTON_EVENT_TRIPLECLICK                                    5
#define BUTTON_EVENT_LNGCLICK                                       6
#define BUTTON_EVENT_LNGLNGCLICK                                    7

#define BUTTON_DEBOUNCE_DELAY                                       50      // Debounce delay (ms)
#define BUTTON_DBLCLICK_DELAY                                       350     // Time in ms to wait for a second (or third...) click
#define BUTTON_LNGCLICK_DELAY                                       900     // Time in ms holding the button down to get a long click
#define BUTTON_LNGLNGCLICK_DELAY                                    2500    // Time in ms holding the button down to get a long-long click

// =============================================================================
// RELAY
// =============================================================================

#define RELAY_TURN_OFF                                              0
#define RELAY_TURN_ON                                               1
#define RELAY_TURN_TOGGLE                                           2

#define RELAY_BOOT_OFF                                              0
#define RELAY_BOOT_ON                                               1
#define RELAY_BOOT_SAME                                             2
#define RELAY_BOOT_TOGGLE                                           3

#define RELAY_TYPE_NORMAL                                           0
#define RELAY_TYPE_INVERSE                                          1
#define RELAY_TYPE_LATCHED                                          2
#define RELAY_TYPE_LATCHED_INVERSE                                  3

#define RELAY_SYNC_ANY                                              0
#define RELAY_SYNC_NONE_OR_ONE                                      1
#define RELAY_SYNC_ONE                                              2
#define RELAY_SYNC_SAME                                             3

#define RELAY_PULSE_NONE                                            0
#define RELAY_PULSE_OFF                                             1
#define RELAY_PULSE_ON                                              2

#define RELAY_PROVIDER_NONE                                         0
#define RELAY_PROVIDER_RELAY                                        1

#define RELAY_DELAY_ON                                              0
#define RELAY_DELAY_OFF                                             0
