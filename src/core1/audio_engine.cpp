#include "audio_engine.h"
#include "sample_player.h"
#include "../shared/midi_message.h"
#include "../shared/config.h"
#include "../shared/pinout.h"

#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// ESP32Synth — included from lib_deps
#include <ESP32Synth.h>

static ESP32Synth sSynth;

// ============================================================
// I2S configuration (PCM5102A, 16-bit stereo 44.1 kHz)
// ============================================================

static void initI2S() {
    // Signature: begin(dataPin, outputMode, bckPin, wsPin, i2sDepth)
    // ESP32Synth defaults to 48kHz internally; PCM5102A handles it fine.
    sSynth.begin(PIN_I2S_DIN, SMODE_I2S, PIN_I2S_BCLK, PIN_I2S_LRCK, I2S_16BIT);
}

// ============================================================
// MIDI dispatch
// ============================================================

// Convert MIDI note number (0-127) to CentiHz (used by ESP32Synth).
// Equal temperament: freq = 440 * 2^((note-69)/12), expressed in CentiHz (* 100).
static uint32_t midiNoteToCentiHz(uint8_t note) {
    // Use integer approximation to avoid float in audio path.
    // 440 Hz = A4 = MIDI 69 = 44000 CentiHz
    // Each semitone up multiplies by 2^(1/12) ≈ 1.05946
    // We use a 128-entry LUT approach via the library's ESP32SynthNotes.h mapping,
    // but since notes are numbered 0-127, we compute it directly here.
    static const uint32_t kA4CentiHz = 44000;
    int8_t semitones = (int8_t)note - 69;  // offset from A4
    // Use float only at init-time note calculation, not in render loop
    float freq = kA4CentiHz * powf(2.0f, semitones / 12.0f);
    return (uint32_t)freq;
}

static void dispatchMidi(const MidiMessage& msg) {
    // ESP32Synth is voice-based (voice 0..MAX_VOICES-1), not MIDI-channel-based.
    // We map: track/channel (0-5) → voice index directly.
    uint16_t voice = msg.channel;  // channel 0-5 maps to voice 0-5

    switch (msg.type) {
        case MidiMsgType::NOTE_ON:
            if (msg.velocity > 0) {
                sSynth.noteOn(voice, midiNoteToCentiHz(msg.note), msg.velocity * 2);
            } else {
                // NOTE_ON vel=0 = NOTE_OFF per MIDI spec
                sSynth.noteOff(voice);
            }
            break;

        case MidiMsgType::NOTE_OFF:
            sSynth.noteOff(voice);
            break;

        case MidiMsgType::CC:
            // CC 7 = volume → setMasterVolume; ignore others for now
            if (msg.note == 7) sSynth.setMasterVolume(msg.velocity * 2);
            break;

        case MidiMsgType::CLOCK:
        case MidiMsgType::START:
        case MidiMsgType::STOP:
            break;

        default:
            break;
    }
}

// ============================================================
// Init & task
// ============================================================

void audioEngineInit() {
    initI2S();
    samplePlayerInit();
}

void audioEngineTask(void* /*arg*/) {
    audioEngineInit();

    MidiMessage msg;
    for (;;) {
        // Drain all pending MIDI messages (non-blocking peek)
        while (xQueueReceive(gMidiQueue, &msg, 0) == pdTRUE) {
            dispatchMidi(msg);
        }

        // Process sample player (mix+write one buffer to I2S)
        samplePlayerProcess(&sSynth);

        // Yield briefly — DMA callbacks keep audio continuous
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
