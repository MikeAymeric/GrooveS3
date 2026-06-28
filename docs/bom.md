# GrooveS3 — Bill of Materials

Last updated: 2026-06-17

Prices are indicative (EUR, AliExpress/LCSC/Mouser). Quantities are per unit.

---

## Microcontroller

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 1 | MCU dev board | ESP32-S3 DevKitC-1 N16R8 | 1 | €8.00 | 16 MB Flash, 8 MB Octal PSRAM |

---

## Audio

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 2 | I2S DAC | PCM5102A module | 1 | €3.50 | 3.3V, I2S, 32-bit 384 kHz |
| 3 | 3.5 mm stereo jack | PJ-320A | 1 | €0.15 | TRS output |
| 4 | Electrolytic cap | 10 µF 16V | 2 | €0.05 | DC blocking on audio out |

---

## Display

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 5 | OLED display | SH1106 128×64 I2C | 1 | €3.00 | 1.3", 4-pin I2C module |

---

## Storage

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 6 | MicroSD slot | Standard SPI slot module | 1 | €0.80 | 3.3V, SPI, with level shift |
| 7 | MicroSD card | 32 GB Class 10 | 1 | €5.00 | FAT32, for samples |

---

## Input — Shift Registers

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 8  | Parallel-in shift reg. | 74HC165 DIP-16 | 2 | €0.20 | Chained for 16 step buttons |
| 9  | Serial-out shift reg.  | 74HC595 DIP-16 | 2 | €0.20 | Chained for 16 step LEDs |

---

## Input — Step Buttons & LEDs

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 10 | Tactile buttons | 6×6 mm, 4-pin | 20 | €0.05 | 16 steps + 4 function buttons |
| 11 | LEDs | 3 mm green, 20 mA | 16 | €0.03 | Step indicators |
| 12 | Resistors | 220 Ω 1/4W | 16 | €0.02 | LED current limiting |

---

## Input — Encoders

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 13 | Rotary encoder | ALPS EC11 with push | 2 | €1.80 | 20 detents, 6 mm D-shaft |
| 14 | Encoder knobs | 6 mm knurled | 2 | €0.40 | Any matching knob |

---

## Input — Potentiometers

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 15 | Potentiometer | Alpha RD901F 10 kΩ linear | 3 | €0.90 | BPM, Volume, Param |
| 16 | Pot knobs | 6 mm D-shaft | 3 | €0.30 | Any matching knob |

---

## MIDI

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 17 | DIN-5 socket | Female, PCB mount | 2 | €0.25 | MIDI IN + OUT |
| 18 | Optocoupler | 6N137 | 2 | €0.35 | MIDI isolation (TX + RX) — note: pin 7 Enable must be tied to VCC |
| 19 | Resistors | 220 Ω / 270 Ω / 10 kΩ | 8 | €0.02 | MIDI TX: 220 Ω serie LED + 270 Ω output; MIDI RX: 10 kΩ pull-up output + 10 kΩ Enable; x2 optocouplers |
| 20 | Diode | 1N4148 | 1 | €0.05 | MIDI IN protection (antiparallelo LED) |

---

## Power

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 21 | LDO regulator | AMS1117-3.3 | 1 | €0.20 | 3.3V from 5V USB |
| 22 | Decoupling caps | 100 nF ceramic | 10 | €0.02 | Per IC VCC pin |
| 23 | Bulk cap | 100 µF 10V | 2 | €0.10 | Power rail stabilisation |
| 24 | USB-C connector | PCB mount | 1 | €0.40 | 5V power input |

---

## Miscellaneous

| # | Component | Model | Qty | Unit Price | Notes |
|---|-----------|-------|-----|-----------|-------|
| 25 | Pin headers | 2.54 mm, 40-pin strip | 2 | €0.30 | Prototyping |
| 26 | PCB | Custom 100×80 mm 2-layer | 1 | €3.00 | JLCPCB 5-pack ÷ 5 |
| 27 | Enclosure | Hammond 1455 or custom acrylic | 1 | €8.00 | TBD |

---

## Total estimated BOM cost: ~€50 (prototype, single unit)
