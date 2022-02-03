/*

MODULES PROTOTYPES

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

// =============================================================================
// DATA STRUCTURES
// =============================================================================

typedef union {
    uint8_t     number;
    uint8_t     bytes[1];
} UINT8_UNION_t;

typedef union {
    uint16_t    number;
    uint8_t     bytes[2];
} UINT16_UNION_t;

typedef union {
    uint32_t    number;
    uint8_t     bytes[4];
} UINT32_UNION_t;

typedef union {
    int8_t      number;
    uint8_t     bytes[1];
} INT8_UNION_t;

typedef union {
    int16_t     number;
    uint8_t     bytes[2];
} INT16_UNION_t;

typedef union {
    int32_t     number;
    uint8_t     bytes[4];
} INT32_UNION_t;

typedef union {
    float       number;
    uint8_t     bytes[4];
} FLOAT32_UNION_t;

typedef union {
    uint32_t    number;
    uint8_t     bytes[4];
} BOOLEAN_UNION_t;

typedef union {
    uint8_t     number;
    uint8_t     bytes[1];
} SWITCH_UNION_t;

typedef union {
    uint8_t     number;
    uint8_t     bytes[1];
} BUTTON_UNION_t;

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

#define PJON_INCLUDE_TSA

#ifndef PJON_PACKET_MAX_LENGTH
    #define PJON_PACKET_MAX_LENGTH 90
#endif

#include <PJON.h>

// =============================================================================
// REGISTER MODULE
// =============================================================================

typedef struct {
    uint8_t data_type;
    uint8_t value[4];
    uint8_t flash_address;
} register_io_register_t;

typedef struct {
    char name[16];
    uint8_t data_type;
    bool settable;
    bool queryable;
    uint8_t value[4];
    uint8_t flash_address;
} register_attr_register_t;

// =============================================================================
// LED MODULE
// =============================================================================

typedef struct {
    uint8_t pin;
    bool reverse;
    uint8_t mode;
    unsigned long next;
} led_t;

// =============================================================================
// BUTTON MODULE
// =============================================================================

#include <DebounceEvent.h>

typedef struct {
    DebounceEvent * button;
    uint8_t register_address;   // Address in communication register to store state
    uint8_t current_status;
} button_t;

// =============================================================================
// RELAY MODULE
// =============================================================================

typedef struct {
    // Configuration variables
    uint8_t pin;                // GPIO pin for the relay
    uint8_t type;               // RELAY_TYPE_NORMAL, RELAY_TYPE_INVERSE, RELAY_TYPE_LATCHED or RELAY_TYPE_LATCHED_INVERSE
    uint8_t reset_pin;          // GPIO to reset the relay if RELAY_TYPE_LATCHED

    uint8_t register_address;   // Address in communication register to store state

    unsigned long delay_on;     // Delay to turn relay ON
    unsigned long delay_off;    // Delay to turn relay OFF

    // Status variables
    bool current_status;        // Holds the current (physical) status of the relay
    bool target_status;         // Holds the target status
    unsigned long fw_start;     // Flood window start time
    uint8_t fw_count;           // Number of changes within the current flood window
    unsigned long change_time;  // Scheduled time to change
} relay_t;
