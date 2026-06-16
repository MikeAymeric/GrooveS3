# GrooveS3 — GPIO Pinout

**Board:** ESP32-S3 DevKitC-1 N16R8 (16 MB Flash, 8 MB Octal PSRAM)

All GPIO assignments verified conflict-free. No ADC/Strapping/USB pin overlaps.

---

## I2C — SH1106 OLED 128×64

| GPIO | Signal | Notes |
|------|--------|-------|
| 17 | SDA | 4.7 kΩ pull-up to 3.3V |
| 18 | SCL | 4.7 kΩ pull-up to 3.3V |

---

## I2S — PCM5102A DAC

| GPIO | Signal | Notes |
|------|--------|-------|
| 38 | BCLK | Bit clock |
| 39 | LRCK | Left/right word clock |
| 40 | DIN  | Serial data to DAC |

PCM5102A SCK → GND (auto-clock mode), FMT → GND (I2S format), XSMT → 3.3V (unmute).

---

## SPI — shared bus (SD card + 74HC595 + 74HC165)

| GPIO | Signal | Notes |
|------|--------|-------|
| 11 | MOSI | Common data-out |
| 13 | MISO | Common data-in (only SD uses this) |
| 12 | SCK  | Common clock |
| 10 | SD_CS | MicroSD chip select (active-low) |
| 9  | HC595_LATCH | 74HC595 storage register clock (rising edge) |
| 8  | HC165_LOAD  | 74HC165 parallel load (active-low) |

HC595 OE → GND (always enabled). HC165 CLK_INH → GND.

---

## UART1 — MIDI DIN 5-pin

| GPIO | Signal | Notes |
|------|--------|-------|
| 41 | MIDI_TX | Via 220 Ω + 6N138 optocoupler to DIN OUT |
| 42 | MIDI_RX | Via 6N138 optocoupler from DIN IN |

Baud rate: 31250.

---

## ADC — Potentiometers (Alpha RD901F 10 kΩ)

| GPIO | Signal | Range |
|------|--------|-------|
| 1 | POT_BPM   | 40–200 BPM |
| 2 | POT_VOL   | Master volume 0–100% |
| 4 | POT_PARAM | Generic parameter |

Wiper to GPIO; other terminals to 3.3V and GND. 100 nF cap wiper-to-GND recommended for noise.

---

## Encoders — ALPS EC11 (×2)

| GPIO | Signal | Notes |
|------|--------|-------|
| 5  | ENC1_A | Interrupt (CHANGE), 10 kΩ pull-up |
| 6  | ENC1_B | Interrupt (CHANGE), 10 kΩ pull-up |
| 7  | ENC1_BTN | Push switch, INPUT_PULLUP |
| 14 | ENC2_A | Interrupt (CHANGE), 10 kΩ pull-up |
| 15 | ENC2_B | Interrupt (CHANGE), 10 kΩ pull-up |
| 16 | ENC2_BTN | Push switch, INPUT_PULLUP |

ENC1: track/pattern navigation. ENC2: value/step editing.

---

## Function Buttons

| GPIO | Signal | Notes |
|------|--------|-------|
| 21 | SHIFT_BTN | Modifier key, INPUT_PULLUP |

Step buttons and 4 function buttons are read via 74HC165 shift register on SPI.

---

## Spare / Reserved

| GPIO | Signal | Notes |
|------|--------|-------|
| 47 | SPARE | Available for expansion |

---

## Total GPIO used: 24 of 45 available
