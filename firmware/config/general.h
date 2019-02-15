/*

FIRMWARE GENERAL SETTINGS

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// DEBUG
// =============================================================================

#ifndef DEBUG_SUPPORT
#define DEBUG_SUPPORT                       1                   // Enable serial debug log
#endif

#ifndef SERIAL_BAUDRATE
#define SERIAL_BAUDRATE                     115200              // Default baudrate
#endif

#ifndef DEBUG_ADD_TIMESTAMP
#define DEBUG_ADD_TIMESTAMP                 1                   // Add timestamp to debug messages
                                                                // (in millis overflowing every 1000 seconds)
#endif

// =============================================================================
// SYSTEM MODULE
// =============================================================================

#define SYSTEM_MAX_PONG_DELAY               10000
#define SYSTEM_PING_PONG_INTERVAL           30000               // Interval between ping messages (in ms)

#define SYSTEM_HEARTBEAT_INTERVAL           300000              // Interval between heartbeat messages (in ms)
#define SYSTEM_UPTIME_OVERFLOW              4294967295          // Uptime overflow value

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

#ifndef COMMUNICATION_BUS_EN_PIN
#define COMMUNICATION_BUS_EN_PIN            9
#endif

#ifndef COMMUNICATION_BUS_TX_PIN
#define COMMUNICATION_BUS_TX_PIN            3
#endif

#ifndef COMMUNICATION_BUS_RX_PIN
#define COMMUNICATION_BUS_RX_PIN            2
#endif

#ifndef COMMUNICATION_BUS_NODE_ADDR
#define COMMUNICATION_BUS_NODE_ADDR         4
#endif

#ifndef COMMUNICATION_BUS_UNKNOWN_ADDR
#define COMMUNICATION_BUS_UNKNOWN_ADDR      0
#endif

#ifndef COMMUNICATION_MASTER_SEARCH_DELAY
#define COMMUNICATION_MASTER_SEARCH_DELAY   2500
#endif

// =============================================================================
// NODE MODULE
// =============================================================================

#ifndef NODE_INIT_DELAY
#define NODE_INIT_DELAY                     5000
#endif

// =============================================================================
// BUTTON MODULE
// =============================================================================

#ifndef BUTTON_SUPPORT
#define BUTTON_SUPPORT                      0
#endif

#ifndef BUTTON_DEBOUNCE_DELAY
#define BUTTON_DEBOUNCE_DELAY               50          // Debounce delay (ms)
#endif

#ifndef BUTTON_DBLCLICK_DELAY
#define BUTTON_DBLCLICK_DELAY               500         // Time in ms to wait for a second (or third...) click
#endif

#ifndef BUTTON_LNGCLICK_DELAY
#define BUTTON_LNGCLICK_DELAY               1000        // Time in ms holding the button down to get a long click
#endif

#ifndef BUTTON_LNGLNGCLICK_DELAY
#define BUTTON_LNGLNGCLICK_DELAY            10000       // Time in ms holding the button down to get a long-long click
#endif

// =============================================================================
// RELAY MODULE
// =============================================================================

#ifndef RELAY_SUPPORT
#define RELAY_SUPPORT                       0
#endif

// Default boot mode: 0 means OFF, 1 ON and 2 whatever was before
#ifndef RELAY_BOOT_MODE
#define RELAY_BOOT_MODE                     RELAY_BOOT_OFF
#endif

// 0 means ANY, 1 zero or one and 2 one and only one
#ifndef RELAY_SYNC
#define RELAY_SYNC                          RELAY_SYNC_ANY
#endif

// Default pulse mode: 0 means no pulses, 1 means normally off, 2 normally on
#ifndef RELAY_PULSE_MODE
#define RELAY_PULSE_MODE                    RELAY_PULSE_NONE
#endif

// Default pulse time in seconds
#ifndef RELAY_PULSE_TIME
#define RELAY_PULSE_TIME                    1.0
#endif

// Relay requests flood protection window - in seconds
#ifndef RELAY_FLOOD_WINDOW
#define RELAY_FLOOD_WINDOW                  3
#endif

// Allowed actual relay changes inside requests flood protection window
#ifndef RELAY_FLOOD_CHANGES
#define RELAY_FLOOD_CHANGES                 5
#endif

// Pulse with in milliseconds for a latched relay
#ifndef RELAY_LATCHING_PULSE
#define RELAY_LATCHING_PULSE                10
#endif

// Do not save relay state after these many milliseconds
#ifndef RELAY_SAVE_DELAY
#define RELAY_SAVE_DELAY                    1000
#endif
