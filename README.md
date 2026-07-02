# GrooveS3

![GrooveS3 logo](docs/logo.svg)

An open source, DIY alternative to the Teenage Engineering OP-1 — a full-featured groovebox built around the **ESP32-S3 DevKitC-1 N16R8**, designed to be assembled for under €50 in components.

16-step, 6-track step sequencer with AMY polyphonic synth engine (FM, PCM, wavetable, Karplus-Strong), PCM sample playback from SD, MIDI DIN I/O, and a 128×64 OLED interface inspired by the OP-1 UI paradigm.

---

## Hardware overview

| Subsystem | Component |
|-----------|-----------|
| MCU | ESP32-S3 DevKitC-1 N16R8 (16 MB Flash, 8 MB PSRAM) |
| Audio DAC | PCM5102A (WCMCU-5102 module) via I2S |
| Display | SH1106 128×64 OLED via I2C |
| Storage | MicroSD via SPI |
| Step buttons (in) | 74HC165 shift register × 2 (16 steps) |
| Function buttons (in) | 74HC165 shift register × 1, chained (8 buttons: PLAY/STOP, REC, OVERVIEW, PATTERN, SOUND, NOTE, FX, MIXER) |
| Step LEDs (out) | 74HC595 shift register × 2 (16 LEDs) |
| Navigation | 2× rotary encoder with push (ENC1: track/play-stop, ENC2: value/REC) + SHIFT held modifier |
| Control | 4× 10 kΩ potentiometer |
| MIDI | DIN 5-pin IN/OUT via UART1 (6N137 optocoupler) |

Full GPIO table: [docs/pinout.md](docs/pinout.md)  
Bill of materials: [docs/bom.md](docs/bom.md)

---

## Software architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        ESP32-S3                             │
│                                                             │
│  Core 0                         Core 1                      │
│  ──────────────────────         ──────────────────────      │
│  [Sequencer task]               [AudioEngine task]          │
│    • 16-step × 6-track            • AMY synthesizer         │
│    • BPM clock (FreeRTOS)         • PCM sample player       │
│    • Input polling                • I2S → PCM5102A          │
│      (HC165, encoders, ADC)       • PSRAM sample buffers    │
│  [UI task]                                                  │
│    • SH1106 OLED @ ≤30 fps   ←── FreeRTOS Queue ──────────►│
│    • ENC1/ENC2 navigation         MidiMessage               │
│  [MIDI task]                      (NOTE_ON/OFF, CC, CLOCK)  │
│    • DIN IN parse → queue                                   │
│    • Queue → DIN OUT                                        │
└─────────────────────────────────────────────────────────────┘
```

Core 0 produces `MidiMessage` structs into a FreeRTOS queue.  
Core 1 drains the queue and dispatches to AMY synthesizer or sample player.  
Both cores are completely decoupled — the queue is the only shared state.

---

## Directory structure

```
GrooveS3/
├── platformio.ini
├── src/
│   ├── main.cpp               # dual-core task launch
│   ├── shared/
│   │   ├── pinout.h           # all GPIO #defines
│   │   ├── config.h           # BPM, steps, sample rate, etc.
│   │   └── midi_message.h     # MidiMessage struct + queue handle
│   ├── core0/
│   │   ├── sequencer.*        # step sequencer + BPM clock
│   │   ├── input.*            # HC165/HC595, encoders, ADC
│   │   ├── ui.*               # OLED display
│   │   └── midi.*             # DIN MIDI IN/OUT
│   └── core1/
│       ├── audio_engine.*     # AMY synthesizer wrapper
│       └── sample_player.*    # PCM WAV from SD via PSRAM
├── docs/
│   ├── pinout.md
│   └── bom.md
└── lib/                       # local dependencies (if any)
```

---

## Setup

### Requirements

- [PlatformIO](https://platformio.org/) (VS Code extension or CLI)
- ESP32-S3 DevKitC-1 N16R8 board

### Build & flash

```bash
# Install dependencies and build
pio run

# Flash
pio run --target upload

# Serial monitor (run in an external terminal, not Cursor's integrated terminal)
pio device monitor --port /dev/ttyACM0 --baud 115200
```

### SD card

Format the card as FAT32.  
Create a `/samples/` folder and place WAV files (16-bit PCM, 44100 Hz, mono or stereo):

```
/samples/
  kick.wav
  snare.wav
  hihat_cl.wav
  hihat_op.wav
  clap.wav
  clave.wav
```

---

## Roadmap

### Phase 1 — Audio + display core ✓
- [x] Project scaffold and pinout
- [x] AMY synthesizer on Core 1, inter-core queue, dual-core FreeRTOS architecture
- [x] PCM drum voices (808 kick + snare via AMY presets)
- [x] I2S → PCM5102A DAC — tested and working
- [x] SH1106 OLED — step grid, BPM, track, volume bar with speaker icon
- [x] BPM potentiometer (GPIO1, 40–200 BPM)
- [x] Volume potentiometer (GPIO2, AMY master volume)
- [x] Encoder navigation: track select + play/stop toggle

### Phase 2 — Input hardware
- [x] HC595 → 16 step LEDs (blip cursor: flash on/off at step change, no flicker)
- [ ] HC165 × 2 → 16 step buttons (toggle steps live)
- [ ] HC165 × 1 → 8 function buttons (PLAY/STOP, REC, OVERVIEW, PATTERN, SOUND, NOTE, FX, MIXER)
- [ ] SHIFT button (GPIO 21, held modifier)
- [ ] Second encoder (ENC2, click = REC)
- [ ] MicroSD card mount + sample loading
- [ ] All 6 PCM drum voices (hi-hat, open hi-hat, clap, clave)

### Phase 3 — Performance features
- [ ] Live pattern record (TR-808 style, ENC2 click = REC)
- [ ] Melodic tracks (waveform: sine/saw/FM/PCM; ENC1 = pitch, ENC2 = parameter)
- [ ] Step buttons as chromatic keyboard (16 semitones C→D#, octave via ENC2)
- [ ] Arpeggiator (rate, pattern: up/down/random/chord, octave range)
- [ ] Chord mode (one step triggers a full chord)
- [ ] Scale quantization (root note + mode: major, minor, dorian, pentatonic…)
- [ ] LFO (modulate AMY parameters: cutoff, pitch, volume)
- [ ] Per-step parameter locks (velocity, pitch, filter)
- [ ] Pattern chaining / song mode
- [ ] Save/load patterns to SD

### Phase 4 — UI/UX design + implementation
- [x] **UI/UX design session** completed 2026-06-30 — 7 modes (OVERVIEW, PATTERN, SOUND, NOTE, FX×8, MIXER), 8 function buttons, full ENC/POT/SHIFT mapping, 70 conflicts resolved
- [ ] 7-mode navigation system implementation
- [ ] Contextual OLED labels per mode
- [ ] Animated waveform display (SOUND/MELODIC)
- [ ] Arcade 80s visuals for FX screens
- [ ] Velocity lane overlay (SHIFT held in OVERVIEW)
- [ ] Parameter lock visual feedback (`▒` on OLED)

### Phase 5 — Advanced synthesis
- [ ] FM synthesis (DX7-style ALGO engine via AMY)
- [ ] Karplus-Strong (physical string modeling)
- [ ] Per-voice effects: reverb, delay, chorus, filter
- [ ] Custom sample loading from SD at runtime

### Phase 6 — Hardware finalization
- [ ] PCB design in KiCad (open source, all schematics public)
- [ ] Gerber files released for self-manufacturing
- [ ] BOM optimized for lowest cost

### Nice to have
- [ ] Tape recorder / looper (record AMY output to SD in real-time)

---

## Inspiration

Inspired by the **Teenage Engineering OP-1** — this project aims to replicate its core feature set as an open source DIY instrument accessible to anyone willing to solder.

Architecture inspired by the **Wee Noise Makers PGB-1**: sequencer and synth engine fully decoupled, communicating only via MIDI-like messages.

## License

MIT
