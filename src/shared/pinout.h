#pragma once

// ============================================================
// GrooveS3 — GPIO Pinout (ESP32-S3 DevKitC-1 N16R8)
// All pins verified conflict-free.
// ============================================================

// --- I2C — SH1106 OLED 128x64 ---
#define PIN_I2C_SDA     17
#define PIN_I2C_SCL     18

// --- I2S — PCM5102A DAC ---
#define PIN_I2S_BCLK    38
#define PIN_I2S_LRCK    39
#define PIN_I2S_DIN     40

// --- SPI shared bus (SD + 74HC595 + 74HC165) ---
#define PIN_SPI_MOSI    11
#define PIN_SPI_MISO    13
#define PIN_SPI_SCK     12

// --- SPI chip selects / control ---
#define PIN_SD_CS       10   // MicroSD card
#define PIN_HC595_LATCH  9   // 74HC595 shift register (step LEDs) — active-high latch
#define PIN_HC165_LOAD   8   // 74HC165 shift register (buttons)  — active-low load

// --- UART1 — MIDI DIN 5-pin ---
#define PIN_MIDI_TX     41
#define PIN_MIDI_RX     42

// --- ADC — potentiometers (12-bit, 0-4095) ---
#define PIN_POT_BPM      1   // Alpha RD901F — BPM control (40-200 BPM)
#define PIN_POT_VOL      2   // Alpha RD901F — master volume
#define PIN_POT_PARAM    4   // Alpha RD901F — generic parameter

// --- Encoder 1 (ALPS EC11) — track / pattern navigation ---
#define PIN_ENC1_A       5   // interrupt-capable
#define PIN_ENC1_B       6   // interrupt-capable
#define PIN_ENC1_BTN     7   // push switch

// --- Encoder 2 (ALPS EC11) — value / step editing ---
#define PIN_ENC2_A      14   // interrupt-capable
#define PIN_ENC2_B      15   // interrupt-capable
#define PIN_ENC2_BTN    16   // push switch

// --- Function buttons ---
#define PIN_SHIFT_BTN   21   // shift / modifier button

// --- Spare ---
#define PIN_SPARE       47   // unassigned, available for expansion
