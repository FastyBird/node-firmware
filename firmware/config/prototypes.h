/*

MODULES PROTOTYPES

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

#include <Vector.h>

// =============================================================================
// DATA STRUCTURES
// =============================================================================

typedef union {
    uint8_t     number;
    uint8_t     bytes[4];
} UINT8_UNION_t;

typedef union {
    uint16_t    number;
    uint8_t     bytes[4];
} UINT16_UNION_t;

typedef union {
    uint32_t    number;
    uint8_t     bytes[4];
} UINT32_UNION_t;

typedef union {
    int8_t      number;
    uint8_t     bytes[4];
} INT8_UNION_t;

typedef union {
    int16_t     number;
    uint8_t     bytes[4];
} INT16_UNION_t;

typedef union {
    int32_t     number;
    uint8_t     bytes[4];
} INT32_UNION_t;

typedef union {
    float       number;
    uint8_t     bytes[4];
} FLOAT32_UNION_t;

struct communication_binary_register_t {
    bool value;
};

struct communication_analog_register_t {
    uint8_t data_type;
    uint8_t size;
    char value[4];
};

typedef struct {
    Vector<communication_binary_register_t> digital_inputs;
    Vector<communication_binary_register_t> digital_outputs;

    Vector<communication_analog_register_t> analog_inputs;
    Vector<communication_analog_register_t> analog_outputs;
} communication_register_t;

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

#define PJON_INCLUDE_TS

#ifndef PJON_PACKET_MAX_LENGTH
    #define PJON_PACKET_MAX_LENGTH 80
#endif

#include <PJON.h>