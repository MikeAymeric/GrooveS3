# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Flash

```bash
# Build only
pio run

# Build + flash
pio run --target upload

# Serial monitor only (run in an external WSL terminal — not Cursor's integrated terminal,
# which fails with termios.error due to missing TTY)
pio device monitor --port /dev/ttyACM0 --baud 115200

# Clean build
pio run --target clean
```

No test suite — validation is done by flashing and observing Serial output at 115200 baud.

## Architecture

Dual-core FreeRTOS on ESP32-S3. The two cores are completely decoupled and communicate only through `gMidiQueue` (a FreeRTOS queue of `MidiMessage` structs defined in `src/main.cpp`).

```
Core 0 — taskSequencer (priority 1, stack 8192)
  seqTask() ← main loop: runs inputPoll() + midiPoll() + uiUpdate() every 5ms,
              and fires step clock at BPM-derived interval
  inputInit / uiInit / midiInit called before entering the loop

Core 1 — taskAudio (priority 2, stack 16384)
  audioEngineTask() ← drains gMidiQueue → dispatchMidi() → AMY voices
  samplePlayerInit() also runs here (SD card on SPI, PSRAM sample buffers)
```

**Queue flow:** sequencer (Core 0) calls `xQueueSend(&MidiMessage)` → audio engine (Core 1) calls `xQueueReceive` each loop iteration.

## Key Design Decisions

**AMY is voice-based, not MIDI-channel-based.** `amy_add_event(&e)` with `e.osc = voice` and `e.midi_note` (native MIDI note number, no conversion needed). Each of the 6 sequencer tracks maps to a unique voice index via `sTracks[t].channel = t` set in `seqInit()`.

**SPI bus is shared between two cores.** HC165/HC595 (Core 0, input.cpp) and SD card (Core 1, sample_player.cpp) both use SPI. Access is serialised via `gSpiMutex` (defined in `sample_player.cpp`, extern in `input.cpp`). The mutex is created in `samplePlayerInit()` — HC165/HC595 functions guard on `gSpiMutex != nullptr` and skip silently if called before it exists.

**AMY audio output requires the PCM5102A DAC** — AMY uses I2S and has no PWM fallback mode. Without the DAC, there is no audio output. Pins: `i2s_dout=PIN_I2S_DIN`, `i2s_bclk=PIN_I2S_BCLK`, `i2s_lrc=PIN_I2S_LRCK`, configured via `amy_config_t` in `initI2S()` (`audio_engine.cpp`).

**Encoder ISRs must be `IRAM_ATTR`** — both the dispatcher stubs (`isr_enc0/1`) and the shared helper (`isr_enc`) carry the attribute. Any function called from an ISR on ESP32-S3 must also be in IRAM or it will crash on cache miss.

**Encoder delta reads are protected** with `portDISABLE_INTERRUPTS()` / `portENABLE_INTERRUPTS()` to prevent the ISR from firing between the read and the clear of `sEnc[enc].position`.

## Pinout Quick Reference

Full table in `docs/pinout.md`. Critical constraints:
- **GPIO 35/36/37** — reserved for Octal PSRAM, never use externally on N16R8
- **GPIO 19/20** — USB D−/D+
- **GPIO 43/44** — UART0 (Serial monitor)
- **ADC1 only** (GPIO 1–10) for potentiometers — ADC2 (GPIO 11–20) is unreliable with BT/WiFi
- **GPIO 48** — onboard WS2812 RGB LED (use Adafruit NeoPixel, `NEO_GRB + NEO_KHZ800`)
- **I2C for OLED SH1106** — SDA=GPIO 17, SCL=GPIO 18 (chosen during hardware bring-up)

## AMY Notes

Library: `shorepine/AMY` v1.2.7. AMY manages I2S internally and starts its own render thread on ESP32-S3. Call `amy_update()` in the audio task loop each iteration.

**API pattern:**
```cpp
amy_event e = amy_default_event();
e.osc       = voice;          // 0–5, maps to sequencer track
e.midi_note = (float)note;    // native MIDI note number, no conversion
e.velocity  = vel / 127.0f;   // 0.0 = note off, >0 = note on
amy_add_event(&e);
```

AMY has no user `IRAM_ATTR` code — IRAM at 100% is entirely the ESP32 IDF framework overhead, not AMY. The linker succeeds; this is not an actionable problem.

## Platform

Uses **pioarduino** (`platform-espressif32` fork) instead of the official Espressif PlatformIO platform, because AMY requires ESP-IDF 5.x (`driver/i2s_pdm.h`). The official platform was still on IDF 4.4 at time of setup.

Current: `pioarduino/platform-espressif32 55.03.39` → arduino-esp32 3.3.9 + IDF 5.5.4.

**SdFat + arduino-esp32 3.x conflict:** arduino-esp32 3.x redefines `FILE_READ` as `"r"` (string), which conflicts with `SdFat::open()` expecting an `oflag_t`. Always use `O_RDONLY` instead of `FILE_READ` when opening files with SdFat.

## Branch Strategy

- `main` — stable, releasable
- `develop` — active development, all work goes here
