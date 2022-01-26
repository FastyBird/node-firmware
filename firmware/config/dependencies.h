/*

DEPENDENCIES CHECK

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#pragma once

//------------------------------------------------------------------------------
// Do not change this file unless you know what you are doing
// Configuration settings are in the general.h file
//------------------------------------------------------------------------------

#if DEBUG_SUPPORT == 0
    #undef DEBUG_COMMUNICATION_SUPPORT
    #define DEBUG_COMMUNICATION_SUPPORT         0   // Disable communication module debug
#endif
