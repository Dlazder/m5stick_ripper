#pragma once
#include "M5Unified.h"
#include <Wire.h>
#include <time.h>

// Capabilities
#define HAS_RTC            true
#define HAS_PHYSICAL_KB    false
#define HAS_SPEAKER        true

// IMU axis mapping for mouse and level tool
#define IMU_MOUSE_X(ax, ay)           ( (ay) )
#define IMU_MOUSE_Y(ax, ay)           ( (ax) )
#define IMU_LEVEL_ROLL(ax, ay)        ( atan2((ax), (ay)) * 180 / PI )
#define IMU_LEVEL_ANGLE(ax, ay)       ( abs(atan2((ay), (ax)) * 180 / PI) )
#define IMU_LEVEL_DISPLAY_OFFSET 90
#define IMU_LEVEL_BUBBLE_ROLL(ax, ay, az)   ( atan2((ay), (az)) * 180 / PI )
#define IMU_LEVEL_BUBBLE_PITCH(ax, ay, az)  ( -atan2((ax), (az)) * 180 / PI )

// IMU axis mapping for gyro keyboard (same as mouse for StickC Plus2)
#define IMU_KB_X(ax, ay)              ( (ay) )
#define IMU_KB_Y(ax, ay)              ( (ax) )

// NFC I2C pins
#define NFC_SDA  G32
#define NFC_SCL  G33

// IR transmitter — built-in LED on M5StickC Plus 2
#define IR_SEND_PIN 19
// IR receiver pin (OUT of the module connected to G26)
#define IR_RECEIVE_PIN 26

// UART terminal — GROVE port (G33=TX, G32=RX)
#define UART_TX_PIN 33
#define UART_RX_PIN 32
#define UART_BAUD_DEFAULT 115200

// Available GPIOs broken out on the headers, used for IR TX/RX pin configuration.
// Some pins drive built-in peripherals (SD: 0/26/32/36, Grove I2C: 32/33, buzzer: 2) —
// picking one of those for IR will disable the peripheral while it is in use.
static const uint8_t devicePins[] = {
	0, 26, 32, 33, 36
};
static constexpr uint8_t devicePinsCount = sizeof(devicePins) / sizeof(devicePins[0]);

// KEY_BACKSPACE value for BLE keyboard
#define BLE_KEY_BACKSPACE  KEY_BACKSPACE

// Init
inline void deviceInit() {
    auto cfg = DEVICE.config();
    DEVICE.begin(cfg);
}

inline void deviceUpdate() {
    DEVICE.update();
}

// Speaker
inline void deviceSpeakerBegin() { /* always on */ }
inline void deviceSpeakerEnd()   { /* always on */ }

// Microphone calibration table — maps raw M5 dB (dbFS + 94) to real dB SPL
static const float DEVICE_CALIB_TABLE[][2] = {
    {6.0f,  45.0f},
    {12.0f, 45.0f},
    {15.0f, 45.0f},
    {31.0f, 46.0f},
    {40.0f, 47.0f},
    {45.0f, 49.0f},
    {58.0f, 51.0f},
    {64.0f, 54.0f},
    {71.0f, 61.0f},
    {81.0f, 68.0f},
    {91.0f, 75.0f},
};
static constexpr int DEVICE_CALIB_TABLE_SIZE = sizeof(DEVICE_CALIB_TABLE) / sizeof(DEVICE_CALIB_TABLE[0]);

// Time
struct DeviceTime { int hours; int minutes; int seconds; };

inline DeviceTime deviceGetTime() {
    DeviceTime t;
    auto dt = DEVICE.Rtc.getDateTime();
    t.hours   = dt.time.hours;
    t.minutes = dt.time.minutes;
    t.seconds = dt.time.seconds;
    return t;
}

inline void deviceSetTime(int hours, int minutes, int seconds) {
    m5::rtc_datetime_t dt = DEVICE.Rtc.getDateTime();
    dt.time.hours   = hours;
    dt.time.minutes = minutes;
    dt.time.seconds = seconds;
    DEVICE.Rtc.setDateTime(&dt);
}
