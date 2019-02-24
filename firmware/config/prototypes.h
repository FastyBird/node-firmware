/*

MODULES PROTOTYPES

Copyright (C) 2018 FastyBird Ltd. <info@fastybird.com>

*/

// =============================================================================
// COMMUNICATION MODULE
// =============================================================================

#define PJON_INCLUDE_TS

#ifndef PJON_PACKET_MAX_LENGTH
    #define PJON_PACKET_MAX_LENGTH 80
#endif

//#ifndef TS_TIME_IN
//    #define TS_TIME_IN 0
//#endif

//#ifndef PJON_INCLUDE_PACKET_ID
//    #define PJON_INCLUDE_PACKET_ID true
//#endif

// Include async ack defining PJON_INCLUDE_ASYNC_ACK before including PJON.h
//#define PJON_INCLUDE_ASYNC_ACK  true
// Synchronous acknowledgement is not used, set TS_RESPONSE_TIME_OUT to 0
//#define TS_RESPONSE_TIME_OUT    0

#include <PJON.h>