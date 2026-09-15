#pragma once
// SD card SPI bus pins + capability, per device.
// Selected by the same build flags as devices/device.h.
// Kept separate from devices/ so HAL/service translation units can include it
// without dragging in the display/input stack (M5Unified, M5Cardputer).

#if defined(CARDPUTER)
    #define HAS_SD        true
    #define SD_CS_PIN     12
    #define SD_CLK_PIN    40
    #define SD_MOSI_PIN   14
    #define SD_MISO_PIN   39
#elif defined(STICKS3)
    #define HAS_SD        true
    #define SD_CS_PIN     7
    #define SD_CLK_PIN    5
    #define SD_MOSI_PIN   6
    #define SD_MISO_PIN   4
#else
    #define HAS_SD        true
    #define SD_CS_PIN     32
    #define SD_CLK_PIN    0
    #define SD_MOSI_PIN   26
    #define SD_MISO_PIN   36
#endif
