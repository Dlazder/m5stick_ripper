#pragma once
// This file is included from globals.h AFTER BleCombo to avoid KEY_BACKSPACE
// redefinition by M5Cardputer headers.
#include "M5Unified.h"
#include <M5Cardputer.h>
#include <Wire.h>
#include <time.h>

// Capabilities
#define HAS_RTC            false
#define HAS_PHYSICAL_KB    true
#define HAS_SPEAKER        true

// IMU axis mapping for Cardputer orientation
#define IMU_MOUSE_X(ax, ay)           ( -(ax) )
#define IMU_MOUSE_Y(ax, ay)           ( (ay) )
#define IMU_LEVEL_ROLL(ax, ay)        ( -atan2((ay), (ax)) * 180 / PI )
#define IMU_LEVEL_ANGLE(ax, ay)       ( abs(atan2((ax), (ay)) * 180 / PI) )
#define IMU_LEVEL_DISPLAY_OFFSET 90
#define IMU_LEVEL_BUBBLE_ROLL(ax, ay, az)   ( -atan2((ax), (az)) * 180 / PI )
#define IMU_LEVEL_BUBBLE_PITCH(ax, ay, az)  ( -atan2((ay), (az)) * 180 / PI )

// IMU axis mapping for gyro keyboard (not used — Cardputer has physical KB)
#define IMU_KB_X(ax, ay)              ( -(ax) )
#define IMU_KB_Y(ax, ay)              ( (ay) )

// NFC I2C pins (Cardputer GROVE port — G2=SDA, G1=SCL)
#define NFC_SDA  2
#define NFC_SCL  1

// IR transmitter — built-in LED on G44
#define IR_SEND_PIN 44
// IR receiver — external module on G8 (SDA)
#define IR_RECEIVE_PIN 3

// UART terminal — GROVE port (G1=TX, G2=RX)
#define UART_TX_PIN 1
#define UART_RX_PIN 2
#define UART_BAUD_DEFAULT 115200

// Available GPIOs broken out, used for IR TX/RX pin configuration.
// Some pins drive built-in peripherals (keyboard matrix/SYS I2C: 8/9, SD: 12/40/14/39,
// USB: 19/20) — picking one of those for IR will disable the peripheral while it is in use.
static const uint8_t devicePins[] = {
	1, 2, 3, 4, 5, 6, 13, 14, 15, 39
};
static constexpr uint8_t devicePinsCount = sizeof(devicePins) / sizeof(devicePins[0]);

// KEY_BACKSPACE conflicts with M5Cardputer header — use raw HID value
#define BLE_KEY_BACKSPACE  0xB2

// Init
inline void deviceInit() {
    auto cfg = DEVICE.config();
    M5Cardputer.begin(cfg, true);
    // Wire.begin(SDA, SCL) expands to Wire.begin(8, 9) on the esp32s3 variant.
    // On original Cardputer:  GPIO8/GPIO9 are 74HC138 (keyboard matrix), not I2C.
    //    Using them as I2C breaks keyboard scanning → use GROVE pins (GPIO1/GPIO2).
    // On Cardputer-ADV:       GPIO8/GPIO9 are SYS I2C (where the GROVE port is wired).
    //    Wire.begin(8, 9) is correct there.
    if (DEVICE.getBoard() == m5::board_t::board_M5Cardputer) {
        Wire.begin(2, 1);   // original Cardputer — GROVE I2C on GPIO2(SDA)/GPIO1(SCL)
    } else {
        Wire.begin(SDA, SCL); // Cardputer-ADV and others — SYS I2C GPIO8(SDA)/GPIO9(SCL)
    }
}

// cardputerKbUpdate() is defined in utils/cardputerKbUtils.h, included after this file.
// We declare it here so deviceUpdate can call it.
void cardputerKbUpdate();

inline void deviceUpdate() {
    cardputerKbUpdate();
}

// Speaker
inline void deviceSpeakerBegin() { DEVICE.Speaker.begin(); }
inline void deviceSpeakerEnd()   { DEVICE.Speaker.end(); }

// Microphone calibration table — maps raw M5 dB (dbFS + 94) to real dB SPL
// TODO: calibrate with a reference SPL meter for Cardputer
static const float DEVICE_CALIB_TABLE[][2] = {
	{12.0f,  20.0f},
	{15.0f, 21.0f},
	{30.0f, 24.0f},
	{40.0f, 25.0f},
	{41.0f, 28.0f},
	{45.0f, 32.0f},
	{55.0f, 36.0f},
	{60.0f, 38.0f},
	{70.0f, 40.0f},
	{75.0f, 43.0f},
	{76.0f, 46.0f},
};
static constexpr int DEVICE_CALIB_TABLE_SIZE = sizeof(DEVICE_CALIB_TABLE) / sizeof(DEVICE_CALIB_TABLE[0]);

// Time
struct DeviceTime { int hours; int minutes; int seconds; };

inline DeviceTime deviceGetTime() {
    DeviceTime t;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    struct tm timeinfo;
    localtime_r(&tv.tv_sec, &timeinfo);
    t.hours   = timeinfo.tm_hour;
    t.minutes = timeinfo.tm_min;
    t.seconds = timeinfo.tm_sec;
    return t;
}

inline void deviceSetTime(int hours, int minutes, int seconds) {
    struct timeval tv;
    struct tm timeinfo = {};
    timeinfo.tm_hour = hours;
    timeinfo.tm_min  = minutes;
    timeinfo.tm_sec  = seconds;
    timeinfo.tm_year = 70;
    timeinfo.tm_mday = 1;
    tv.tv_sec  = mktime(&timeinfo);
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);
}
