# GrooveS3 — Guida al Cablaggio

Guida pratica per il cablaggio hardware. Riferimento pinout completo in `pinout.md`.

---

## Alimentazione

```
USB-C 5V → AMS1117-3.3 IN
AMS1117-3.3 OUT → 3.3V rail
100 nF ceramic cap ogni IC (VCC pin → GND, più vicino possibile all'IC)
100 µF bulk cap su 3.3V rail (vicino al regolatore)
```

---

## I2C — SH1106 OLED

```
ESP32 GPIO 17 (SDA) → 4.7 kΩ → 3.3V
                     → OLED SDA
ESP32 GPIO 18 (SCL) → 4.7 kΩ → 3.3V
                     → OLED SCL
OLED VCC → 3.3V
OLED GND → GND
```

---

## I2S — PCM5102A DAC

```
ESP32 GPIO 38 (BCLK) → PCM5102A BCK
ESP32 GPIO 39 (LRCK) → PCM5102A LRCK
ESP32 GPIO 40 (DIN)  → PCM5102A DIN

PCM5102A VCC  → 3.3V
PCM5102A GND  → GND
PCM5102A SCK  → GND   (modalità auto-clock)
PCM5102A FMT  → GND   (formato I2S standard)
PCM5102A XSMT → 3.3V  (unmute)

Output audio:
PCM5102A OUTL → 10 µF (DC block) → Jack TRS Left
PCM5102A OUTR → 10 µF (DC block) → Jack TRS Right
Jack GND → GND
```

---

## SPI — Bus Condiviso

Tutti e tre i dispositivi condividono MOSI/SCK. Chip select separati.

```
ESP32 GPIO 11 (MOSI) → SD MOSI, HC595 SER, HC165 QH (non usato in scrittura)
ESP32 GPIO 13 (MISO) → SD MISO
ESP32 GPIO 12 (SCK)  → SD SCK, HC595 SRCLK, HC165 CLK

ESP32 GPIO 10 (SD_CS)      → MicroSD CS (active-low)
ESP32 GPIO  9 (HC595_LATCH)→ 74HC595 RCLK (rising edge → aggiorna uscite)
ESP32 GPIO  8 (HC165_LOAD) → 74HC165 SH/LD (active-low → campiona ingressi)

74HC595 OE    → GND   (output sempre abilitato)
74HC595 SRCLR → 3.3V  (non resettare)
74HC595 VCC   → 3.3V

74HC165 CLK_INH → GND  (clock sempre attivo)
74HC165 VCC     → 3.3V

MicroSD VCC   → 3.3V (usare modulo con level shifter integrato se 5V)
```

**Chain 16 step LEDs (2× HC595):**
```
HC595 #1 SER  ← MOSI
HC595 #1 QH'  → HC595 #2 SER  (cascade)
HC595 #1/#2 RCLK ← HC595_LATCH
HC595 #1/#2 SRCLK ← SCK
Uscite Q0–Q7 (#1) → 220 Ω → LED step 1–8 → GND
Uscite Q0–Q7 (#2) → 220 Ω → LED step 9–16 → GND
```

**Chain 16 step buttons + 4 function buttons (2× HC165):**
```
HC165 #1 QH  → HC165 #2 SER  (cascade)
HC165 #2 QH  → MISO (ma non condiviso con SD — collegare a GPIO separato se necessario)
HC165 #1/#2 SH/LD ← HC165_LOAD
HC165 #1/#2 CLK   ← SCK
Ingressi A–H (#1) → pulsanti step 1–8 (pull-up interno 10 kΩ su ogni input, altro terminale a GND)
Ingressi A–H (#2) → step 9–12 + 4 function buttons
```

---

## MIDI DIN 5-pin — 6N137

> **Attenzione:** il pin 7 (Enable) del 6N137 deve essere collegato a VCC tramite 10 kΩ.
> Se lasciato flottante, l'uscita è sempre disabilitata e il MIDI non funziona.

### MIDI OUT (TX) — GPIO 41

```
ESP32 GPIO 41 ──→ 220 Ω ──→ 6N137 pin 2 (Anode)
                             6N137 pin 3 (Cathode) ──→ GND
                             6N137 pin 4 (GND)     ──→ GND
                             6N137 pin 6 (VCC)     ──→ 3.3V
                             6N137 pin 7 (Enable)  ──→ 10 kΩ ──→ 3.3V
                             6N137 pin 5 (Output)  ──→ 270 Ω ──→ DIN OUT pin 5

DIN OUT pin 2 ──→ GND
DIN OUT pin 4 e 1, 3 → non collegati
```

### MIDI IN (RX) — GPIO 42

```
DIN IN pin 5 ──→ 220 Ω ──→ 6N137 pin 2 (Anode)
DIN IN pin 4 ──────────→ 6N137 pin 3 (Cathode)
                          1N4148 antiparallelo tra pin 2 e pin 3 (anodo su pin 3)

                          6N137 pin 4 (GND)    ──→ GND
                          6N137 pin 6 (VCC)    ──→ 3.3V
                          6N137 pin 7 (Enable) ──→ 10 kΩ ──→ 3.3V
                          6N137 pin 5 (Output) ──→ 10 kΩ ──→ 3.3V
                                                            ──→ ESP32 GPIO 42

DIN IN pin 2 → non collegato
```

---

## ADC — Potenziometri

Cablaggio identico per tutti e tre (RD901F 10 kΩ lineare B):

```
Terminale CW  → 3.3V
Terminale CCW → GND
Wiper (centro)→ 100 nF cap → GND  (rumore)
               → ESP32 GPIO (1, 2, o 4)
```

**Non usare GPIO 11–20 per ADC** — ADC2 è inaffidabile con WiFi/BT attivo.

---

## Encoder ALPS EC11

Cablaggio identico per entrambi (ENC1 e ENC2):

```
Pin A → 10 kΩ → 3.3V → ESP32 GPIO (5 o 14)
Pin B → 10 kΩ → 3.3V → ESP32 GPIO (6 o 15)
Pin C (comune) → GND
Pin SW1 → ESP32 GPIO (7 o 16)  (INPUT_PULLUP interno, no resistore esterno)
Pin SW2 → GND
```

**Gli ISR degli encoder devono essere IRAM_ATTR** — già gestito in `input.cpp`.

---

## Pulsante SHIFT

```
ESP32 GPIO 21 → pulsante → GND  (INPUT_PULLUP interno, no resistore esterno)
```

---

## Pin da non toccare

| GPIO | Motivo |
|------|--------|
| 35, 36, 37 | Octal PSRAM — collegamento interno N16R8 |
| 19, 20 | USB D− / D+ |
| 43, 44 | UART0 Serial monitor |

---

## Checklist pre-accensione

- [ ] 3.3V rail presente (misura con multimetro prima di inserire il DevKit)
- [ ] Nessun cortocircuito VCC-GND
- [ ] Pin Enable (pin 7) del 6N137 collegato a 3.3V su entrambi gli optocoupler
- [ ] Pull-up 10 kΩ su output 6N137 (pin 5) per MIDI RX
- [ ] SPI: nessun CS attivo a riposo (tutti high)
- [ ] Encoder: pull-up 10 kΩ su A e B, comune a GND
- [ ] Potenziometri: cap 100 nF wiper-GND installati
