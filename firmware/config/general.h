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

#ifndef SERIAL_BAUDRATE
#define SERIAL_BAUDRATE                             38400           // Default baudrate
#endif

#ifndef DEBUG_ADD_TIMESTAMP
#define DEBUG_ADD_TIMESTAMP                         1               // Add timestamp to debug messages
                                                                    // (in millis overflowing every 1000 seconds)
#endif

// =============================================================================
// SYSTEM MODULE
// =============================================================================

#define SYSTEM_UPTIME_OVERFLOW                      4294967295      // Uptime overflow value

#ifndef SYSTEM_CONFIGURE_DEVICE_BUTTON
#define SYSTEM_CONFIGURE_DEVICE_BUTTON              INDEX_NONE
#endif

#ifndef SYSTEM_DEVICE_STATE_LED
#define SYSTEM_DEVICE_STATE_LED                     INDEX_NONE
#endif

#ifndef SYSTEM_DEVICE_COMMUNICATION_LED
#define SYSTEM_DEVICE_COMMUNICATION_LED             INDEX_NONE
#endif

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

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
#define COMMUNICATION_MASTER_PING_TIMEOUT           15000           // 
#endif

#ifndef COMMUNICATION_PROTOCOL_VERSION
#define COMMUNICATION_PROTOCOL_VERSION              0x01
#endif

#ifndef COMMUNICATION_DISABLE_ADDRESS_STORING
#define COMMUNICATION_DISABLE_ADDRESS_STORING       1               // Define maximum size of AO registers
#endif

#ifndef COMMUNICATION_MAX_DI_REGISTER_SIZE
#define COMMUNICATION_MAX_DI_REGISTER_SIZE          0               // Define maximum size of DI registers
#endif

#ifndef COMMUNICATION_MAX_DO_REGISTER_SIZE
#define COMMUNICATION_MAX_DO_REGISTER_SIZE          0               // Define maximum size of DO registers
#endif

#ifndef COMMUNICATION_MAX_AI_REGISTER_SIZE
#define COMMUNICATION_MAX_AI_REGISTER_SIZE          0               // Define maximum size of AI registers
#endif

#ifndef COMMUNICATION_MAX_AO_REGISTER_SIZE
#define COMMUNICATION_MAX_AO_REGISTER_SIZE          0               // Define maximum size of AO registers
#endif

#ifndef COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE
#define COMMUNICATION_MAX_DEVICE_SETTINGS_SIZE      0               // Define maximum size of device settings
#endif

#ifndef COMMUNICATION_MAX_DEVICE_ATTRIBUTES_SIZE
#define COMMUNICATION_MAX_DEVICE_ATTRIBUTES_SIZE    0               // Define maximum size of device attributes
#endif

#ifndef COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE
#define COMMUNICATION_MAX_REGISTERS_SETTINGS_SIZE   0               // Define maximum size of registers settings
#endif

#ifndef COMMUNICATION_NOTIFY_STATE_DELAY
#define COMMUNICATION_NOTIFY_STATE_DELAY            5000            // Delay before master is notified after boot up
#endif

#ifndef COMMUNICATION_DESCRIPTION_SUPPORT
#define COMMUNICATION_DESCRIPTION_SUPPORT           1               // 
#endif

#ifndef COMMUNICATION_SETTINGS_SUPPORT
#define COMMUNICATION_SETTINGS_SUPPORT              1               // 
#endif

#ifndef COMMUNICATION_PUB_SUB_PUB_SUPPORT
#define COMMUNICATION_PUB_SUB_PUB_SUPPORT           1               // Enable or disable publish register support
#endif

#ifndef COMMUNICATION_PUB_SUB_SUB_SUPPORT
#define COMMUNICATION_PUB_SUB_SUB_SUPPORT           1               // Enable or disable subscription to messages
#endif

#ifndef COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS
#define COMMUNICATION_PUB_SUB_MAX_SUBSCRIPTIONS     0               // Define maximum supported subscriptions
#endif

#ifndef COMMUNICATION_PUB_SUB_MAX_CONDITIONS
#define COMMUNICATION_PUB_SUB_MAX_CONDITIONS        1               // Define maximum supported conditions per subscription
#endif

#ifndef COMMUNICATION_PUB_SUB_MAX_ACTIONS
#define COMMUNICATION_PUB_SUB_MAX_ACTIONS           1               // Define maximum supported actions per subscription
#endif

#ifndef COMMUNICATION_REGISTER_KEY_LENGTH
#define COMMUNICATION_REGISTER_KEY_LENGTH           9               // 
#endif

#ifndef COMMUNICATION_USE_CRC
#define COMMUNICATION_USE_CRC                       1               // Add CRC checksum to end of packet
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
#define RELAY_BOOT_MODE                             RELAY_BOOT_OFF
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
