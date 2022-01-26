/*

FIRMWARE GENERAL SETTINGS

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

// =============================================================================
// DEBUG
// =============================================================================

#ifndef DEBUG_SUPPORT
#define DEBUG_SUPPORT                               1               // Enable serial debug log
#endif

#ifndef DEBUG_COMMUNICATION_SUPPORT
#define DEBUG_COMMUNICATION_SUPPORT                 1               // Enable communication serial debug log
#endif

#ifndef DEBUG_ADD_TIMESTAMP
#define DEBUG_ADD_TIMESTAMP                         1               // Add timestamp to debug messages
                                                                    // (in millis overflowing every 1000 seconds)
#endif

// =============================================================================
// SYSTEM MODULE
// =============================================================================

#define SYSTEM_UPTIME_OVERFLOW                      4294967295      // Uptime overflow value

#ifndef SYSTEM_CONFIGURE_DEVICE_BUTTON_INDEX
#define SYSTEM_CONFIGURE_DEVICE_BUTTON_INDEX        INDEX_NONE
#endif

#ifndef SYSTEM_DEVICE_STATE_LED_INDEX
#define SYSTEM_DEVICE_STATE_LED_INDEX               INDEX_NONE
#endif

#ifndef SYSTEM_DEVICE_COMMUNICATION_LED_INDEX
#define SYSTEM_DEVICE_COMMUNICATION_LED_INDEX       INDEX_NONE
#endif

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

#ifndef COMMUNICATION_SERIAL_BAUDRATE
#define COMMUNICATION_SERIAL_BAUDRATE               38400           // Default baudrate
#endif

#ifndef COMMUNICATION_BUS_HARDWARE_SERIAL
#define COMMUNICATION_BUS_HARDWARE_SERIAL           0
#endif

#ifndef COMMUNICATION_BUS_EN_PIN
#define COMMUNICATION_BUS_EN_PIN                    GPIO_NONE
#endif

#ifndef COMMUNICATION_BUS_TX_PIN
#define COMMUNICATION_BUS_TX_PIN                    GPIO_NONE
#endif

#ifndef COMMUNICATION_BUS_RX_PIN
#define COMMUNICATION_BUS_RX_PIN                    GPIO_NONE
#endif

#ifndef COMMUNICATION_BUS_MASTER_ADDR
#define COMMUNICATION_BUS_MASTER_ADDR               254             // Master network address definition
#endif

#ifndef COMMUNICATION_MASTER_PING_TIMEOUT
#define COMMUNICATION_MASTER_PING_TIMEOUT           15000
#endif

#ifndef COMMUNICATION_PROTOCOL_VERSION
#define COMMUNICATION_PROTOCOL_VERSION              0x01
#endif

#ifndef COMMUNICATION_DISABLE_ADDRESS_STORING
#define COMMUNICATION_DISABLE_ADDRESS_STORING       1
#endif

#ifndef COMMUNICATION_NOTIFY_STATE_DELAY
#define COMMUNICATION_NOTIFY_STATE_DELAY            5000            // Delay before master is notified after boot up
#endif

#ifndef COMMUNICATION_USE_CRC
#define COMMUNICATION_USE_CRC                       1               // Add CRC checksum to end of packet
#endif

#ifndef COMMUNICATION_ATTR_REGISTER_ADDR_ADDRESS
#define COMMUNICATION_ATTR_REGISTER_ADDR_ADDRESS    0               // Attribute register address where is stored device address
#endif

#ifndef COMMUNICATION_ATTR_REGISTER_PML_ADDRESS
#define COMMUNICATION_ATTR_REGISTER_PML_ADDRESS     1               // Attribute register address where is stored packet max length
#endif

#ifndef COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS
#define COMMUNICATION_ATTR_REGISTER_STATE_ADDRESS   2               // Attribute register address where is stored device state
#endif

// =============================================================================
// REGISTER MODULE
// =============================================================================

#ifndef REGISTER_MAX_INPUT_REGISTERS_SIZE
#define REGISTER_MAX_INPUT_REGISTERS_SIZE           0               // Define maximum size of input registers
#endif

#ifndef REGISTER_MAX_OUTPUT_REGISTERS_SIZE
#define REGISTER_MAX_OUTPUT_REGISTERS_SIZE          0               // Define maximum size of output registers
#endif

#ifndef REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE
#define REGISTER_MAX_ATTRIBUTE_REGISTERS_SIZE       0               // Define maximum size of attribute registers
#endif

// =============================================================================
// BUTTON MODULE
// =============================================================================

#ifndef BUTTON_MAX_ITEMS
#define BUTTON_MAX_ITEMS                            0               // Define maximum size of buttons items
#endif

// =============================================================================
// EXPANDER BUTTON MODULE
// =============================================================================

#ifndef BUTTON_EXPANDER_SUPPORT
#define BUTTON_EXPANDER_SUPPORT                     0
#endif

// =============================================================================
// RELAY MODULE
// =============================================================================

// Default boot mode: 0 means OFF, 1 ON and 2 whatever was before
#ifndef RELAY_BOOT_MODE
#define RELAY_BOOT_MODE                             RELAY_BOOT_SAME
#endif

// 0 means ANY, 1 zero or one and 2 one and only one
#ifndef RELAY_SYNC
#define RELAY_SYNC                                  RELAY_SYNC_ANY
#endif

// Default pulse mode: 0 means no pulses, 1 means normally off, 2 normally on
#ifndef RELAY_PULSE_MODE
#define RELAY_PULSE_MODE                            RELAY_PULSE_NONE
#endif

// Default pulse time in seconds
#ifndef RELAY_PULSE_TIME
#define RELAY_PULSE_TIME                            1.0
#endif

// Relay requests flood protection window - in seconds
#ifndef RELAY_FLOOD_WINDOW
#define RELAY_FLOOD_WINDOW                          3
#endif

// Allowed actual relay changes inside requests flood protection window
#ifndef RELAY_FLOOD_CHANGES
#define RELAY_FLOOD_CHANGES                         5
#endif

// Pulse with in milliseconds for a latched relay
#ifndef RELAY_LATCHING_PULSE
#define RELAY_LATCHING_PULSE                        10
#endif

// Do not save relay state after these many milliseconds
#ifndef RELAY_SAVE_DELAY
#define RELAY_SAVE_DELAY                            1000
#endif

#ifndef RELAY_MAX_ITEMS
#define RELAY_MAX_ITEMS                             0               // Define maximum size of relay items
#endif
