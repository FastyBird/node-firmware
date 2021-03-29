/*

FIRMWARE UTILITIES

Copyright (C) 2018 FastyBird s.r.o. <code@fastybird.com>

*/

unsigned long getUptime()
{
    static unsigned long last_uptime = 0;
    static unsigned char uptime_overflows = 0;

    if (millis() < last_uptime) {
        ++uptime_overflows;
    }

    last_uptime = millis();

    unsigned long uptime_seconds = uptime_overflows * (SYSTEM_UPTIME_OVERFLOW / 1000) + (last_uptime / 1000);

    return uptime_seconds;
}

// -----------------------------------------------------------------------------

void niceDelay(
    unsigned long ms
) {
    unsigned long start = millis();
    while (millis() - start < ms) delay(1);
}
