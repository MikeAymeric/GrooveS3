# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Flash

```bash
# Build only
pio run

# Build + flash + open serial monitor
pio run --target upload --target monitor

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
  audioEngineTask() ← drains gMidiQueue → dispatchMidi() → ESP32Synth voices
  samplePlayerInit() also runs here (SD card on SPI, PSRAM sample buffers)
```

**Queue flow:** sequencer (Core 0) calls `xQueueSend(&MidiMessage)` → audio engine (Core 1) calls `xQueueReceive` each loop iteration.

## Key Design Decisions

**ESP32Synth is voice-based, not MIDI-channel-based.** `noteOn(voice, freqCentiHz, volume)` — notes use CentiHz (not MIDI note numbers). `midiNoteToCentiHz()` in `audio_engine.cpp` converts. Each of the 6 sequencer tracks maps to a unique voice index via `sTracks[t].channel = t` set in `seqInit()`.

**SPI bus is shared between two cores.** HC165/HC595 (Core 0, input.cpp) and SD card (Core 1, sample_player.cpp) both use SPI. Access is serialised via `gSpiMutex` (defined in `sample_player.cpp`, extern in `input.cpp`). The mutex is created in `samplePlayerInit()` — HC165/HC595 functions guard on `gSpiMutex != nullptr` and skip silently if called before it exists.

**`SMODE_PWM` on GPIO47** is available for audio testing without the PCM5102A DAC. Change `initI2S()` in `audio_engine.cpp` to `sSynth.begin(PIN_SPARE, SMODE_PWM, -1, -1, I2S_16BIT)` for a wire-a-jack-and-go audio POC.

**Encoder ISRs must be `IRAM_ATTR`** — both the dispatcher stubs (`isr_enc0/1`) and the shared helper (`isr_enc`) carry the attribute. Any function called from an ISR on ESP32-S3 must also be in IRAM or it will crash on cache miss.

**Encoder delta reads are protected** with `portDISABLE_INTERRUPTS()` / `portENABLE_INTERRUPTS()` to prevent the ISR from firing between the read and the clear of `sEnc[enc].position`.

## Pinout Quick Reference

Full table in `docs/pinout.md`. Critical constraints:
- **GPIO 35/36/37** — reserved for Octal PSRAM, never use externally on N16R8
- **GPIO 19/20** — USB D−/D+
- **GPIO 43/44** — UART0 (Serial monitor)
- **ADC1 only** (GPIO 1–10) for potentiometers — ADC2 (GPIO 11–20) is unreliable with BT/WiFi

## ESP32Synth Notes

Library: `danilogcrf2-oss/ESP32Synth` v2.4.1. `MAX_VOICES` is defined as 80 in `ESP32Synth_Config.hpp`. **Do not redefine `MAX_VOICES` anywhere** — use `SEQ_TRACKS` (6) for our voice count. `struct Voice` is also defined by the library globally — never declare another `struct Voice`.

DMA latency is configured via build flags in `platformio.ini`:
```
-DSYNTH_DMA_BUF_LEN=128
-DSYNTH_DMA_BUF_COUNT=2
```
This gives ~5ms latency, suitable for a groovebox.

## Branch Strategy

- `master` — stable, releasable
- `develop` — active development, all work goes here
