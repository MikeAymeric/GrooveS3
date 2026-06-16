# GrooveS3

A DIY groovebox built around the **ESP32-S3 DevKitC-1 N16R8**.  
16-step, 6-track step sequencer with polyphonic synth engine, PCM sample playback from SD, MIDI DIN I/O, and a 128×64 OLED interface.

---

## Hardware overview

| Subsystem | Component |
|-----------|-----------|
| MCU | ESP32-S3 DevKitC-1 N16R8 (16 MB Flash, 8 MB PSRAM) |
| Audio DAC | PCM5102A via I2S |
| Display | SH1106 128×64 OLED via I2C |
| Storage | MicroSD via SPI |
| Step buttons (in) | 74HC165 shift register × 2 (16 steps + 4 function) |
| Step LEDs (out) | 74HC595 shift register × 2 (16 LEDs) |
| Navigation | 2× ALPS EC11 rotary encoder with push |
| Control | 3× Alpha RD901F 10 kΩ potentiometer |
| MIDI | DIN 5-pin IN/OUT via UART1 (6N138 optocoupler) |

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
│    • 16-step × 6-track            • ESP32Synth              │
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
Core 1 drains the queue and dispatches to ESP32Synth or sample player.  
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
│       ├── audio_engine.*     # ESP32Synth wrapper
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

# Flash and open monitor
pio run --target upload --target monitor
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

### Phase 1 — Audio POC
- [x] Project scaffold and pinout
- [ ] ESP32Synth on Core 1 producing tone via I2S → PCM5102A
- [ ] SD card mount and single WAV playback
- [ ] Inter-core queue: manual trigger from Serial

### Phase 2 — Sequencer + UI
- [ ] 16-step sequencer with BPM clock on Core 0
- [ ] HC165 step button input + HC595 LED output
- [ ] OLED UI: step grid, BPM, active track
- [ ] Encoder navigation (track select, play/stop)
- [ ] Potentiometer BPM control

### Phase 3 — Full groovebox
- [ ] 6-track pattern with per-track sample assignment
- [ ] MIDI DIN IN/OUT (clock sync + note I/O)
- [ ] Pattern chaining / song mode
- [ ] Per-step parameter locks (velocity, pitch)
- [ ] Saving/loading patterns to SD

---

## Inspiration

Architecture inspired by the **Wee Noise Makers PGB-1**: sequencer and synth engine fully decoupled, communicating only via MIDI-like messages.

## License

MIT
