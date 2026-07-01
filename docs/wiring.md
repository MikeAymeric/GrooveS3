# GrooveS3 — Guida al Cablaggio

Guida pratica per il cablaggio hardware su breadboard.
Riferimento pinout completo in `pinout.md`.

---

## Come leggere questa guida

### Orientamento dei chip DIP

I chip con piedini (74HC595, 74HC165, 6N137) si inseriscono a cavallo della scanalatura centrale della breadboard. Il pin 1 si trova **a sinistra del segno a mezzaluna** (notch) o accanto al **punto** stampato sul package.

```
       notch o punto
          ▼
       ┌──┤├──┐
  1 ──►│       │◄── 8  (su chip DIP-8)
  2 ──►│       │◄── 7
  3 ──►│       │◄── 6
  4 ──►│       │◄── 5
       └───────┘
   i pin si contano in senso antiorario
```

### Condensatori di disaccoppiamento (100 nF ceramico)

Ogni IC (74HC595, 74HC165, 6N137) vuole un condensatore ceramico da **100 nF** tra il suo pin VCC e il suo pin GND, posizionato il più vicino possibile al chip. Serve a stabilizzare l'alimentazione del chip e ridurre il rumore.

**Come collegarlo:** un terminale del condensatore va nella stessa riga breadboard del pin VCC del chip, l'altro terminale nella riga del pin GND. Il condensatore ceramico da 100 nF non ha polarità — i due terminali sono intercambiabili.

### Resistenze di pull-up

Una resistenza di pull-up tiene una linea a livello alto (3.3V) quando nessun segnale la sta tirando a basso. Si collega tra la linea da proteggere e 3.3V. Nei circuiti che seguono, le vedrai scritte come:

```
segnale ──┬── resistenza (10 kΩ) → 3.3V
          └── (pin del chip o GPIO)
```

### LED — polarità

Il LED ha un terminale lungo (**anodo**, +) e uno corto (**catodo**, −). La corrente entra dall'anodo ed esce dal catodo.

```
  uscita HC595 → 220 Ω → [anodo | LED | catodo] → GND
                           (gamba lunga)  (gamba corta)
```

---

## Alimentazione (breadboard)

Il DevKit ESP32-S3 si alimenta via USB-C e già fornisce 3.3V e GND sui suoi pin. Non serve un regolatore esterno per il prototipo.

```
ESP32-S3 pin 3V3  →  binario rosso (+) della breadboard
ESP32-S3 pin GND  →  binario blu (−) della breadboard
```

Porta poi i binari 3.3V e GND anche sul lato opposto della breadboard se usi una breadboard doppia.

---

## I2C — OLED SH1106 128×64

Le due linee I2C (SDA e SCL) richiedono una **resistenza di pull-up da 4.7 kΩ ciascuna** verso 3.3V. Senza di esse il display non comunica. La resistenza va collegata tra la linea dati e 3.3V — non in serie lungo il filo.

```
3.3V ──┬── 4.7 kΩ ──┬── ESP32 GPIO 17 (SDA) ──── OLED SDA
       │             │
       └── 4.7 kΩ ──┴── ESP32 GPIO 18 (SCL) ──── OLED SCL

OLED VCC → 3.3V
OLED GND → GND
```

Il modulo OLED SH1106 a 4 pin ha già il bus I2C — non serve altro.

---

## I2S — DAC WCMCU-5102 (PCM5102A)

Il modulo WCMCU-5102 ha jack TRS da 3.5 mm e condensatori DC-block già integrati sulla board. Non serve aggiungere nulla all'uscita audio — colleghi direttamente le cuffie o un amplificatore al jack.

Pin del modulo che trovi stampati sulla board:

| Pin WCMCU-5102 | Collega a            | Perché                                     |
|----------------|----------------------|--------------------------------------------|
| **3.3V**       | 3.3V                 | Alimentazione (usa questo, non VCC)        |
| **GND**        | GND                  |                                            |
| **BCK**        | ESP32 GPIO 38        | Bit clock I2S                              |
| **LCK**        | ESP32 GPIO 39        | Left/right clock I2S                       |
| **DIN**        | ESP32 GPIO 40        | Dati audio                                 |
| **SCL**        | GND                  | Modalità PLL (no clock esterno)            |
| **FMT**        | GND                  | Formato I2S standard                       |
| **XMT**        | 3.3V                 | Unmute — **fondamentale**, senza non senti audio |
| **FLT**        | GND                  | Filtro normale                             |
| **DMP**        | GND                  | Nessuna de-emphasis                        |

> **VCC**: lascialo non collegato. Alimenti il modulo direttamente dal pin 3.3V, che bypassa il regolatore interno.

---

## SPI — Bus condiviso (SD card + 74HC595 + 74HC165)

I tre dispositivi condividono le stesse tre linee SPI (MOSI, MISO, SCK). Ogni dispositivo ha poi un proprio pin di selezione (chip select o latch) che lo attiva individualmente.

```
Linee SPI condivise:
  ESP32 GPIO 11 (MOSI) → HC595 #1 SER (pin 14)  +  SD MOSI
  ESP32 GPIO 13 (MISO) → HC165 #2 QH (pin 9)    +  SD MISO
  ESP32 GPIO 12 (SCK)  → HC595 #1/#2 SRCLK (pin 11)
                       → HC165 #1/#2 CLK (pin 2)
                       → SD SCK

Chip select individuali:
  ESP32 GPIO 10  → SD CS (active-low)
  ESP32 GPIO  9  → HC595 #1/#2 RCLK (pin 12)   ← latch LED
  ESP32 GPIO  8  → HC165 #1/#2 SH/LD (pin 1)   ← carica bottoni
```

---

### 74HC595 — Shift register uscite (LED step)

Due HC595 in cascata pilotano i 16 LED degli step. Il chip riceve dati seriali e li espone su 8 uscite parallele.

**Pinout 74HC595 DIP-16:**

```
        notch
       ┌──┤├──┐
QB  1 ─┤       ├─ 16  VCC      → 3.3V
QC  2 ─┤       ├─ 15  QA
QD  3 ─       ─ 14  SER      → dati in (MOSI o QH' del chip precedente)
QE  4 ─┤       ├─ 13  OE       → GND  (output enable attivo basso)
QF  5 ─┤       ├─ 12  RCLK    → GPIO 9  (latch: aggiorna le uscite)
QG  6 ─┤       ├─ 11  SRCLK  → GPIO 12 (clock)
QH  7 ─┤       ├─ 10  SRCLR  → 3.3V  (reset attivo basso: tienilo alto)
GND 8 ─┤       ├─  9  QH'     → SER del chip successivo (cascade)
       └───────┘
  Uscite: QA(15), QB(1), QC(2), QD(3), QE(4), QF(5), QG(6), QH(7)
```

**Connessioni HC595 #1 (step 1–8):**

```
HC595 #1 pin 16 (VCC)   → 3.3V
HC595 #1 pin  8 (GND)   → GND
HC595 #1 pin 14 (SER)   → ESP32 GPIO 11 (MOSI)
HC595 #1 pin 11 (SRCLK) → ESP32 GPIO 12 (SCK)
HC595 #1 pin 12 (RCLK)  → ESP32 GPIO 9  (LATCH)
HC595 #1 pin 13 (OE)    → GND
HC595 #1 pin 10 (SRCLR) → 3.3V
HC595 #1 pin  9 (QH')   → HC595 #2 pin 14 (SER)   ← cascade
```

**Connessioni HC595 #2 (step 9–16):**

```
HC595 #2 pin 16 (VCC)   → 3.3V
HC595 #2 pin  8 (GND)   → GND
HC595 #2 pin 14 (SER)   → HC595 #1 pin 9 (QH')    ← dal chip precedente
HC595 #2 pin 11 (SRCLK) → ESP32 GPIO 12 (SCK)
HC595 #2 pin 12 (RCLK)  → ESP32 GPIO 9  (LATCH)
HC595 #2 pin 13 (OE)    → GND
HC595 #2 pin 10 (SRCLR) → 3.3V
```

**LED collegati alle uscite** (mappa step → pin HC595):

| Step | HC595 | Pin uscita | Connessione                        |
|------|-------|------------|------------------------------------|
| 1    | #1    | QA (15)    | pin 15 → 220 Ω → LED anodo → GND  |
| 2    | #1    | QB (1)     | pin 1  → 220 Ω → LED anodo → GND  |
| 3    | #1    | QC (2)     | pin 2  → 220 Ω → LED anodo → GND  |
| 4    | #1    | QD (3)     | pin 3  → 220 Ω → LED anodo → GND  |
| 5    | #1    | QE (4)     | pin 4  → 220 Ω → LED anodo → GND  |
| 6    | #1    | QF (5)     | pin 5  → 220 Ω → LED anodo → GND  |
| 7    | #1    | QG (6)     | pin 6  → 220 Ω → LED anodo → GND  |
| 8    | #1    | QH (7)     | pin 7  → 220 Ω → LED anodo → GND  |
| 9    | #2    | QA (15)    | pin 15 → 220 Ω → LED anodo → GND  |
| 10   | #2    | QB (1)     | pin 1  → 220 Ω → LED anodo → GND  |
| 11   | #2    | QC (2)     | pin 2  → 220 Ω → LED anodo → GND  |
| 12   | #2    | QD (3)     | pin 3  → 220 Ω → LED anodo → GND  |
| 13   | #2    | QE (4)     | pin 4  → 220 Ω → LED anodo → GND  |
| 14   | #2    | QF (5)     | pin 5  → 220 Ω → LED anodo → GND  |
| 15   | #2    | QG (6)     | pin 6  → 220 Ω → LED anodo → GND  |
| 16   | #2    | QH (7)     | pin 7  → 220 Ω → LED anodo → GND  |

> La **resistenza da 220 Ω** limita la corrente al LED e va messa **in serie** tra il pin di uscita del HC595 e l'anodo del LED. Senza di essa il LED si brucia (e potenzialmente il chip).

> **100 nF** di disaccoppiamento su ogni HC595 tra VCC (pin 16) e GND (pin 8).

---

### 74HC165 — Shift register ingressi (pulsanti step)

Tre HC165 in cascata leggono i 16 pulsanti degli step più gli 8 function button. Ogni chip campiona 8 ingressi paralleli e li invia in serie all'ESP32 (24 bit totali).

**Pinout 74HC165 DIP-16:**

```
        notch
       ┌──┤├──┐
SH/LD 1 ─┤       ├─ 16  VCC      → 3.3V
CLK   2 ─┤       ├─ 15  CLK_INH → GND  (clock sempre attivo)
E     3 ─┤       ├─ 14  D
F     4 ─┤       ├─ 13  C
G     5 ─┤       ├─ 12  B
H     6 ─┤       ├─ 11  A
QH'   7 ─┤       ├─ 10  SER     → QH del chip precedente (cascade)
GND   8 ─┤       ├─  9  QH      → dati seriali out
       └───────┘
  Ingressi paralleli: A(11), B(12), C(13), D(14), E(3), F(4), G(5), H(6)
```

**Connessioni HC165 #1 (step 1–8):**

```
HC165 #1 pin 16 (VCC)     → 3.3V
HC165 #1 pin  8 (GND)     → GND
HC165 #1 pin  1 (SH/LD)   → ESP32 GPIO 8  (HC165_LOAD)
HC165 #1 pin  2 (CLK)     → ESP32 GPIO 12 (SCK)
HC165 #1 pin 15 (CLK_INH) → GND
HC165 #1 pin 10 (SER)     → GND  (nessun dato in ingresso seriale sul primo chip)
HC165 #1 pin  9 (QH)      → HC165 #2 pin 10 (SER)   ← cascade
HC165 #1 pin  7 (QH')     → non collegato
```

**Connessioni HC165 #2 (step 9–16):**

```
HC165 #2 pin 16 (VCC)     → 3.3V
HC165 #2 pin  8 (GND)     → GND
HC165 #2 pin  1 (SH/LD)   → ESP32 GPIO 8  (HC165_LOAD)   ← stesso filo di #1
HC165 #2 pin  2 (CLK)     → ESP32 GPIO 12 (SCK)          ← stesso filo di #1
HC165 #2 pin 15 (CLK_INH) → GND
HC165 #2 pin 10 (SER)     → HC165 #1 pin 9 (QH)
HC165 #2 pin  9 (QH)      → HC165 #3 pin 10 (SER)        ← cascade verso function buttons
HC165 #2 pin  7 (QH')     → non collegato
```

**Connessioni HC165 #3 (8 function buttons):**

```
HC165 #3 pin 16 (VCC)     → 3.3V
HC165 #3 pin  8 (GND)     → GND
HC165 #3 pin  1 (SH/LD)   → ESP32 GPIO 8  (HC165_LOAD)   ← stesso filo di #1 e #2
HC165 #3 pin  2 (CLK)     → ESP32 GPIO 12 (SCK)          ← stesso filo di #1 e #2
HC165 #3 pin 15 (CLK_INH) → GND
HC165 #3 pin 10 (SER)     → HC165 #2 pin 9 (QH)
HC165 #3 pin  9 (QH)      → ESP32 GPIO 13 (MISO)         ← ultimo della catena
HC165 #3 pin  7 (QH')     → non collegato
```

**Function buttons collegati agli ingressi di HC165 #3:**

| Button | HC165 #3 | Pin ingresso | Funzione |
|--------|----------|--------------|----------|
| FB1    | #3       | A (11)       | PLAY/STOP |
| FB2    | #3       | B (12)       | REC |
| FB3    | #3       | C (13)       | MODE: OVERVIEW |
| FB4    | #3       | D (14)       | MODE: PATTERN |
| FB5    | #3       | E (3)        | MODE: SOUND |
| FB6    | #3       | F (4)        | MODE: NOTE |
| FB7    | #3       | G (5)        | MODE: FX |
| FB8    | #3       | H (6)        | MODE: MIXER |

Stesso schema di pull-up dei pulsanti step: `3.3V → 10 kΩ → pin HC165 + pulsante → GND`.

**Pulsanti collegati agli ingressi** (mappa step → pin HC165):

| Step | HC165 | Pin ingresso | Connessione                                      |
|------|-------|--------------|--------------------------------------------------|
| 1    | #1    | A (11)       | 10 kΩ → 3.3V  +  pulsante → GND                |
| 2    | #1    | B (12)       | 10 kΩ → 3.3V  +  pulsante → GND                |
| 3    | #1    | C (13)       | 10 kΩ → 3.3V  +  pulsante → GND                |
| 4    | #1    | D (14)       | 10 kΩ → 3.3V  +  pulsante → GND                |
| 5    | #1    | E (3)        | 10 kΩ → 3.3V  +  pulsante → GND                |
| 6    | #1    | F (4)        | 10 kΩ → 3.3V  +  pulsante → GND                |
| 7    | #1    | G (5)        | 10 kΩ → 3.3V  +  pulsante → GND                |
| 8    | #1    | H (6)        | 10 kΩ → 3.3V  +  pulsante → GND                |
| 9    | #2    | A (11)       | 10 kΩ → 3.3V  +  pulsante → GND                |
| ...  | #2    | ...          | ...                                              |
| 16   | #2    | H (6)        | 10 kΩ → 3.3V  +  pulsante → GND                |

Ogni ingresso del HC165 ha questo schema:

```
3.3V ─── 10 kΩ ─── pin ingresso HC165 ─── terminale pulsante A
                                           terminale pulsante B ─── GND
```

Quando il pulsante non è premuto: il pin legge 3.3V (HC165 vede 1 → il codice inverte → step OFF).
Quando il pulsante è premuto: il pin viene tirato a GND (HC165 vede 0 → il codice inverte → step ON).

> La **resistenza da 10 kΩ** (pull-up) è necessaria perché la HC165 non ha pull-up interni. Senza di essa l'ingresso fluttua e legge valori casuali.

> **100 nF** di disaccoppiamento su ogni HC165 tra VCC (pin 16) e GND (pin 8).

---

### MicroSD — modulo SPI

Usa un modulo con level shifter integrato (etichettato "3.3V" o "3.3V/5V").

```
SD modulo CS   → ESP32 GPIO 10
SD modulo MOSI → ESP32 GPIO 11
SD modulo MISO → ESP32 GPIO 13
SD modulo SCK  → ESP32 GPIO 12
SD modulo VCC  → 3.3V
SD modulo GND  → GND
```

---

## MIDI — 6N137 optocoupler

Il 6N137 isola elettricamente il MIDI IN e OUT. Ne servono due: uno per TX, uno per RX.

> **Attenzione:** il 6N137 **non è pin-compatibile** con il 6N138. Controlla sempre che il chip sia un 6N137.

**Pinout 6N137 DIP-8:**

```
        notch o punto
           ▼
       ┌───┤├───┐
NC  1 ─┤         ├─ 8  NC
A   2 ─┤         ├─ 7  Enable   → 10 kΩ → 3.3V  (OBBLIGATORIO)
K   3 ─┤         ├─ 6  VCC      → 3.3V
GND 4 ─┤         ├─ 5  Output   → 10 kΩ → 3.3V (lato RX) / 270 Ω (lato TX)
       └─────────┘
  A = anodo LED interno
  K = catodo LED interno
```

> **Pin 7 (Enable):** deve essere collegato a 3.3V tramite 10 kΩ. Se lasciato flottante, il chip non produce mai output — è la causa più comune di MIDI che non funziona.

---

### MIDI OUT (TX) — GPIO 41 → DIN OUT

```
ESP32 GPIO 41 ──→ 220 Ω ──→ 6N137 #1 pin 2 (A, anodo)
                              6N137 #1 pin 3 (K, catodo) ──→ GND
                              6N137 #1 pin 4 (GND)       ──→ GND
                              6N137 #1 pin 6 (VCC)       ──→ 3.3V
                              6N137 #1 pin 7 (Enable)    ──→ 10 kΩ ──→ 3.3V
                              6N137 #1 pin 5 (Output)    ──→ 270 Ω ──→ DIN OUT pin 5

DIN OUT pin 2 ──→ GND
DIN OUT pin 1, 3, 4 → non collegati
```

La **resistenza da 220 Ω** limita la corrente al LED interno del 6N137.
La **resistenza da 270 Ω** limita la corrente sul cavo MIDI verso il dispositivo esterno.

---

### MIDI IN (RX) — DIN IN → GPIO 42

```
DIN IN pin 5 ──→ 220 Ω ──→ 6N137 #2 pin 2 (A, anodo)
DIN IN pin 4 ──────────→ 6N137 #2 pin 3 (K, catodo)
                          1N4148 in antiparallelo tra pin 2 e pin 3
                          (anodo diodo su pin 3, catodo su pin 2 — protezione inversione)

                          6N137 #2 pin 4 (GND)    ──→ GND
                          6N137 #2 pin 6 (VCC)    ──→ 3.3V
                          6N137 #2 pin 7 (Enable) ──→ 10 kΩ ──→ 3.3V
                          6N137 #2 pin 5 (Output) ──→ 10 kΩ ──→ 3.3V
                                                   ──→ ESP32 GPIO 42

DIN IN pin 2 → non collegato
```

La **resistenza da 10 kΩ** sul pin 5 (Output) è il pull-up dell'uscita open-collector del 6N137 — senza di essa l'uscita fluttua e GPIO 42 legge dati a caso.

Il **diodo 1N4148** protegge da eventuali connessioni MIDI invertite.

> **100 nF** di disaccoppiamento su ogni 6N137 tra VCC (pin 6) e GND (pin 4).

---

## ADC — Potenziometri

I potenziometri (pot) hanno tre terminali: due fissi (CW e CCW) e uno centrale che si muove (Wiper). Il Wiper va all'ESP32.

```
       CW  ── 3.3V
      │
   [pot 10 kΩ]
      │
     Wiper ──┬── 100 nF ── GND   (condensatore anti-rumore)
             └── ESP32 GPIO

       CCW ── GND
```

| Pot | GPIO | Funzione (OVERVIEW) |
|-----|------|---------------------|
| 1   |  1   | BPM (tempo)         |
| 2   |  2   | Volume master       |
| 3   |  3   | Parametro           |
| 4   |  4   | Parametro           |

Funzioni contestuali per modalità — vedi UI/UX design doc per la mappatura completa.

> Il **condensatore da 100 nF** tra Wiper e GND filtra il rumore elettrico dell'ADC. Non è strettamente obbligatorio ma riduce molto il jitter nella lettura.

> Non usare GPIO 11–20 per ADC — appartengono ad ADC2 che è inaffidabile sul ESP32-S3.

---

## Encoder rotativo ALPS EC11

L'encoder ha 5 terminali: A, B, C (comune), SW1 e SW2 (pulsante).

```
       Encoder
       ┌─────┐
   A ──┤     ├── 10 kΩ → 3.3V → ESP32 GPIO (5 o 14)
   B ──┤     ├── 10 kΩ → 3.3V → ESP32 GPIO (6 o 15)
   C ──┤     ├────────────────→ GND   (comune)
  SW1──┤     ├────────────────→ ESP32 GPIO (7 o 16)
  SW2──┤     ├────────────────→ GND
       └─────┘
```

> Le **resistenze da 10 kΩ** su A e B sono pull-up fisici necessari — anche se l'ESP32 ha INPUT_PULLUP interno, i pull-up esterni rendono il segnale più pulito e riducono i falsi trigger.

> SW1 e SW2 sono il pulsante dell'encoder. **Non serve resistenza esterna**: il codice usa `INPUT_PULLUP` interno dell'ESP32.

| Encoder | A    | B    | BTN  |
|---------|------|------|------|
| ENC1    | GPIO 5 | GPIO 6 | GPIO 7  |
| ENC2    | GPIO 14 | GPIO 15 | GPIO 16 |

---

## Pulsante SHIFT

```
ESP32 GPIO 21 ──── terminale A pulsante
                   terminale B pulsante ──── GND
```

Nessuna resistenza esterna — il codice usa `INPUT_PULLUP` interno dell'ESP32.

---

## Pin da non toccare

| GPIO | Motivo                                    |
|------|-------------------------------------------|
| 35, 36, 37 | Octal PSRAM — collegamento interno N16R8 |
| 19, 20     | USB D− / D+                              |
| 43, 44     | UART0 — monitor seriale                  |

---

## Checklist pre-accensione

Misura tutto con un multimetro prima di collegare l'USB.

**Alimentazione:**
- [ ] 3.3V rail presente sulla breadboard (misura tra binario rosso e blu)
- [ ] Nessun cortocircuito tra 3.3V e GND (multimetro in modalità continuità)

**DAC WCMCU-5102:**
- [ ] XMT collegato a 3.3V (unmute)
- [ ] SCL, FMT, FLT, DMP collegati a GND
- [ ] Pin 3.3V alimentato (non VCC)

**74HC595 (LED):**
- [ ] OE (pin 13) → GND su entrambi i chip
- [ ] SRCLR (pin 10) → 3.3V su entrambi i chip
- [ ] 100 nF tra VCC e GND su ogni chip
- [ ] Ogni LED ha la resistenza da 220 Ω in serie

**74HC165 (pulsanti step + function buttons, 3 chip):**
- [ ] CLK_INH (pin 15) → GND su tutti e 3 i chip
- [ ] SER (pin 10) di HC165 #1 → GND (primo della catena)
- [ ] 100 nF tra VCC e GND su ogni chip
- [ ] Ogni ingresso pulsante ha pull-up da 10 kΩ verso 3.3V
- [ ] HC165 #2 QH → HC165 #3 SER (cascade)
- [ ] HC165 #3 QH → ESP32 GPIO 13 (MISO)

**MIDI 6N137:**
- [ ] Pin 7 (Enable) → 10 kΩ → 3.3V su **entrambi** gli optocoupler
- [ ] Pull-up 10 kΩ su pin 5 (Output) del 6N137 per MIDI RX
- [ ] 100 nF tra VCC (pin 6) e GND (pin 4) su ogni chip

**SPI:**
- [ ] SD CS (GPIO 10) non attivo a riposo (deve essere HIGH)

**Encoder:**
- [ ] Pull-up 10 kΩ su A e B di ogni encoder
- [ ] Comune (C) collegato a GND
