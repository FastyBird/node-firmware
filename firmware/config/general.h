/*

FIRMWARE GENERAL SETTINGS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// DEBUG
// =============================================================================

#ifndef DEBUG_SUPPORT
#define DEBUG_SUPPORT                           1                   // Enable serial debug log
#endif

#ifndef DEBUG_COMMUNICATION_SUPPORT
#define DEBUG_COMMUNICATION_SUPPORT             1                   // Enable communication serial debug log
#endif

#ifndef SERIAL_BAUDRATE
#define SERIAL_BAUDRATE                         115200              // Default baudrate
#endif

#ifndef DEBUG_ADD_TIMESTAMP
#define DEBUG_ADD_TIMESTAMP                     1                   // Add timestamp to debug messages
                                                                    // (in millis overflowing every 1000 seconds)
#endif

// =============================================================================
// SYSTEM MODULE
// =============================================================================

#define SYSTEM_UPTIME_OVERFLOW                  4294967295          // Uptime overflow value

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

#ifndef COMMUNICATION_BUS_EN_PIN
#define COMMUNICATION_BUS_EN_PIN                GPIO_NONE
#endif

#ifndef COMMUNICATION_BUS_TX_PIN
#define COMMUNICATION_BUS_TX_PIN                GPIO_NONE
#endif

#ifndef COMMUNICATION_BUS_RX_PIN
#define COMMUNICATION_BUS_RX_PIN                GPIO_NONE
#endif

#ifndef COMMUNICATION_BUS_GATEWAY_ADDR
#define COMMUNICATION_BUS_GATEWAY_ADDR          254                 // Gateway network address definition
#endif

#ifndef COMMUNICATION_ADDRESSING_TIMEOUT
#define COMMUNICATION_ADDRESSING_TIMEOUT        4000                // Master COMMUNICATION_ACQUIRE_ADDRESS_REQUEST and COMMUNICATION_ACQUIRE_ADDRESS_NEGATE timeout
#endif

#ifndef COMMUNICATION_MASTER_PING_TIMEOUT
#define COMMUNICATION_MASTER_PING_TIMEOUT       4000                // 
#endif

#ifndef COMMUNICATION_MAX_DI_REGISTER_SIZE
#define COMMUNICATION_MAX_DI_REGISTER_SIZE      0                   // Define maximum size of DI registers
#endif

#ifndef COMMUNICATION_MAX_DO_REGISTER_SIZE
#define COMMUNICATION_MAX_DO_REGISTER_SIZE      0                   // Define maximum size of DO registers
#endif

#ifndef COMMUNICATION_MAX_AI_REGISTER_SIZE
#define COMMUNICATION_MAX_AI_REGISTER_SIZE      0                   // Define maximum size of AI registers
#endif

#ifndef COMMUNICATION_MAX_AO_REGISTER_SIZE
#define COMMUNICATION_MAX_AO_REGISTER_SIZE      0                   // Define maximum size of AO registers
#endif

#ifndef COMMUNICATION_DISABLE_ADDRESS_STORING
#define COMMUNICATION_DISABLE_ADDRESS_STORING   0                   // Define maximum size of AO registers
#endif

// =============================================================================
// BUTTON MODULE|EXPANDER BUTTON MODULE
// =============================================================================

#ifndef BUTTON_SUPPORT
#define BUTTON_SUPPORT                          0
#endif

#ifndef BUTTON_EXPANDER_SUPPORT
#define BUTTON_EXPANDER_SUPPORT                 0
#endif

#ifndef BUTTON_DEBOUNCE_DELAY
#define BUTTON_DEBOUNCE_DELAY                   50                  // Debounce delay (ms)
#endif

#ifndef BUTTON_DBLCLICK_DELAY
#define BUTTON_DBLCLICK_DELAY                   500                 // Time in ms to wait for a second (or third...) click
#endif

#ifndef BUTTON_LNGCLICK_DELAY
#define BUTTON_LNGCLICK_DELAY                   1000                // Time in ms holding the button down to get a long click
#endif

#ifndef BUTTON_LNGLNGCLICK_DELAY
#define BUTTON_LNGLNGCLICK_DELAY                5000                // Time in ms holding the button down to get a long-long click
#endif

// =============================================================================
// RELAY MODULE
// =============================================================================

// Default boot mode: 0 means OFF, 1 ON and 2 whatever was before
#ifndef RELAY_BOOT_MODE
#define RELAY_BOOT_MODE                         RELAY_BOOT_OFF
#endif

// 0 means ANY, 1 zero or one and 2 one and only one
#ifndef RELAY_SYNC
#define RELAY_SYNC                              RELAY_SYNC_ANY
#endif

// Default pulse mode: 0 means no pulses, 1 means normally off, 2 normally on
#ifndef RELAY_PULSE_MODE
#define RELAY_PULSE_MODE                        RELAY_PULSE_NONE
#endif

// Default pulse time in seconds
#ifndef RELAY_PULSE_TIME
#define RELAY_PULSE_TIME                        1.0
#endif

// Relay requests flood protection window - in seconds
#ifndef RELAY_FLOOD_WINDOW
#define RELAY_FLOOD_WINDOW                      3
#endif

// Allowed actual relay changes inside requests flood protection window
#ifndef RELAY_FLOOD_CHANGES
#define RELAY_FLOOD_CHANGES                     5
#endif

// Pulse with in milliseconds for a latched relay
#ifndef RELAY_LATCHING_PULSE
#define RELAY_LATCHING_PULSE                    10
#endif

// Do not save relay state after these many milliseconds
#ifndef RELAY_SAVE_DELAY
#define RELAY_SAVE_DELAY                        1000
#endif
