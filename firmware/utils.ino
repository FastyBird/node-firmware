/*

FIRMWARE UTILITIES

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

#include "config/all.h"

#include <Arduino.h>

uint32_t getUptime()
{
    static uint32_t last_uptime = 0;
    static uint8_t uptime_overflows = 0;

    if (millis() < last_uptime) {
        ++uptime_overflows;
    }

    last_uptime = millis();

    uint32_t uptime_seconds = uptime_overflows * (SYSTEM_UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);

    return uptime_seconds;
}

// -----------------------------------------------------------------------------

void niceDelay(
    uint32_t ms
) {
    uint32_t start = millis();
    while (millis() - start < ms) delay(1);
}
