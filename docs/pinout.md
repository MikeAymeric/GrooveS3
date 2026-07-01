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

WCMCU-5102 pin names: SCL → GND (PLL mode), FMT → GND (I2S format), XMT → 3.3V (unmute), FLT → GND, DMP → GND. Usa il pin **3.3V** della board (non VCC). Jack TRS integrato sulla board.

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

**Schemi di collegamento:**
- [HC595 x2 in cascata — 16 LED step](hc595_wiring.png)
- [HC165 x3 in cascata — 16 step + 8 function button](hc165_wiring.png)

---

## UART1 — MIDI DIN 5-pin

| GPIO | Signal | Notes |
|------|--------|-------|
| 41 | MIDI_TX | Via 220 Ω + 6N137 optocoupler to DIN OUT |
| 42 | MIDI_RX | Via 6N137 optocoupler from DIN IN |

Baud rate: 31250.

### 6N137 wiring notes

The 6N137 is **not pin-compatible** with 6N138. DIP-8 pinout: 1=NC, 2=Anode, 3=Cathode, 4=NC, 5=GND, 6=VO (output), 7=VE (Enable), 8=VCC.

- **Pin 2** — LED Anode
- **Pin 3** — LED Cathode
- **Pin 5** — GND
- **Pin 6** — VO, open-collector output (pull-up 10 kΩ to VCC)
- **Pin 7** — Enable (active-high, must be pulled to VCC via 10 kΩ — leave floating = output always disabled)
- **Pin 8** — VCC (must be connected to 3.3V)

**MIDI TX circuit (GPIO 41 → DIN OUT pin 5):**
```
GPIO 41 → 220 Ω → 6N137 pin 2 (anode)
6N137 pin 3 (cathode) → GND
6N137 pin 5 (GND) → GND
6N137 pin 8 (VCC) → 3.3V
6N137 pin 7 (Enable) → 10 kΩ → 3.3V
6N137 pin 6 (output) → 270 Ω → DIN OUT pin 5
DIN OUT pin 2 → GND
```

**MIDI RX circuit (DIN IN → GPIO 42):**
```
DIN IN pin 5 → 220 Ω → 6N137 pin 2 (anode)
DIN IN pin 4 → 6N137 pin 3 (cathode)
1N4148 in antiparallelo tra pin 2 e pin 3 (protezione inversione)
6N137 pin 5 (GND) → GND
6N137 pin 8 (VCC) → 3.3V
6N137 pin 7 (Enable) → 10 kΩ → 3.3V
6N137 pin 6 (output) → 10 kΩ → 3.3V → GPIO 42
```

---

## ADC — Potentiometers (Alpha RD901F 10 kΩ)

| GPIO | Signal | Notes |
|------|--------|-------|
| 1 | POT_1 | Page-dependent function |
| 2 | POT_2 | Page-dependent function |
| 3 | POT_3 | Page-dependent function |
| 4 | POT_4 | Page-dependent function |

Wiper to GPIO; other terminals to 3.3V and GND. 100 nF cap wiper-to-GND recommended for noise.

**Schema di collegamento:** [Potenziometro + condensatore anti-rumore](pot_capacitor_wiring.png)

Functions are mode-dependent. See UI/UX design doc for complete per-mode mapping.

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
| 21 | SHIFT_BTN | Held modifier key, INPUT_PULLUP — never toggle |

8 function buttons read via 74HC165 #3, chained after the 2 step-button HC165s on the same SPI bus (GPIO 8 LOAD, GPIO 12 CLK). Mapping: FB1=PLAY/STOP, FB2=REC, FB3=OVERVIEW, FB4=PATTERN, FB5=SOUND, FB6=NOTE, FB7=FX, FB8=MIXER.

---

## Spare / Reserved

| GPIO | Signal | Notes |
|------|--------|-------|
| 47 | SPARE | Available for expansion |

---

## Total GPIO used: 24 of 45 available
