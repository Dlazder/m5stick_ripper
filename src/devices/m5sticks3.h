#pragma once
#include "M5Unified.h"
#include <Wire.h>
#include <time.h>

// Capabilities
#define HAS_RTC         false
#define HAS_PHYSICAL_KB false
#define HAS_SPEAKER     true

// IMU axis mapping for Sticks3 orientation (horizontal / landscape)
#define IMU_MOUSE_X(ax, ay) ( -(ax) )
#define IMU_MOUSE_Y(ax, ay) ( (ay) )
#define IMU_LEVEL_ROLL(ax, ay)  ( atan2((ax), (ay)) * 180 / PI )
#define IMU_LEVEL_ANGLE(ax, ay) ( abs(atan2((ax), (ay)) * 180 / PI) )
#define IMU_LEVEL_DISPLAY_OFFSET 0
#define IMU_LEVEL_BUBBLE_ROLL(ax, ay, az)   ( -atan2((ax), (az)) * 180 / PI )
#define IMU_LEVEL_BUBBLE_PITCH(ax, ay, az)  ( -atan2((ay), (az)) * 180 / PI )

// IMU axis mapping for gyro keyboard (unswapped vs mouse for landscape)
#define IMU_KB_X(ax, ay) ( -(ax) )
#define IMU_KB_Y(ax, ay) ( (ay) )

// NFC I2C pins
#define NFC_SDA 9
#define NFC_SCL 10

// IR — RMT-based receiver (Arduino 3.x / ESP-IDF 5.x required)
#define IR_USE_RMT
// IR transmitter — built-in LED
#define IR_SEND_PIN 46
// IR receiver — external module connected to G5
#define IR_RECEIVE_PIN 42

// UART terminal — GROVE port (G1=TX, G2=RX)
#define UART_TX_PIN 1
#define UART_RX_PIN 2
#define UART_BAUD_DEFAULT 115200

// Available GPIOs broken out on the headers, used for IR TX/RX pin configuration.
// Some pins drive built-in peripherals (SD: 4/5/6/7, NFC: 9/10, SYS I2C: 11/12, USB: 19/20) —
// picking one of those for IR will disable the peripheral while it is in use.
static const uint8_t devicePins[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 43, 44
};
static constexpr uint8_t devicePinsCount = sizeof(devicePins) / sizeof(devicePins[0]);

// KEY_BACKSPACE value for BLE keyboard
#define BLE_KEY_BACKSPACE KEY_BACKSPACE

// Init
inline void deviceInit() {
	auto cfg = DEVICE.config();
	DEVICE.begin(cfg);
}

inline void deviceUpdate() {
	DEVICE.update();
}

// Speaker
inline void deviceSpeakerBegin() { DEVICE.Speaker.begin(); }
inline void deviceSpeakerEnd() { DEVICE.Speaker.end(); }

// Microphone calibration table — maps raw M5 dB (dbFS + 94) to real dB SPL
// TODO: calibrate with a reference SPL meter for M5StickS3
static const float DEVICE_CALIB_TABLE[][2] = {
	{6.0f,  48.0f},
	{12.0f, 50.0f},
	{30.0f, 52.0f},
	{40.0f, 55.0f},
	{42.0f, 57.0f},
	{50.0f, 60.0f},
	{65.0f, 65.0f},
	{67.0f, 73.0f},
	{75.0f, 75.0f},
	{80.0f, 80.0f},
	{89.0f, 93.0f},
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
	t.hours = timeinfo.tm_hour;
	t.minutes = timeinfo.tm_min;
	t.seconds = timeinfo.tm_sec;
	return t;
}

inline void deviceSetTime(int hours, int minutes, int seconds) {
	struct timeval tv;
	struct tm timeinfo = {};
	timeinfo.tm_hour = hours;
	timeinfo.tm_min = minutes;
	timeinfo.tm_sec = seconds;
	timeinfo.tm_year = 70;
	timeinfo.tm_mday = 1;
	tv.tv_sec = mktime(&timeinfo);
	tv.tv_usec = 0;
	settimeofday(&tv, nullptr);
}
