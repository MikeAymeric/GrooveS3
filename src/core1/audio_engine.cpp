#include "audio_engine.h"
#include "sample_player.h"
#include "../shared/midi_message.h"
#include "../shared/config.h"
#include "../shared/pinout.h"

#include <Arduino.h>
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
    // ESP32Synth handles I2S driver setup internally.
    // We pass pin assignments through the begin() call.
    sSynth.begin(
        AUDIO_SAMPLE_RATE,
        AUDIO_BITS_PER_SAMPLE,
        PIN_I2S_BCLK,
        PIN_I2S_LRCK,
        PIN_I2S_DIN
    );
}

// ============================================================
// MIDI dispatch
// ============================================================

static void dispatchMidi(const MidiMessage& msg) {
    switch (msg.type) {
        case MidiMsgType::NOTE_ON:
            if (msg.velocity > 0) {
                sSynth.noteOn(msg.channel, msg.note, msg.velocity);
            } else {
                // NOTE_ON with velocity 0 = NOTE_OFF per MIDI spec
                sSynth.noteOff(msg.channel, msg.note);
            }
            break;

        case MidiMsgType::NOTE_OFF:
            sSynth.noteOff(msg.channel, msg.note);
            break;

        case MidiMsgType::CC:
            // CC 7 = volume; others forwarded as generic CC
            sSynth.controlChange(msg.channel, msg.note, msg.velocity);
            break;

        case MidiMsgType::CLOCK:
        case MidiMsgType::START:
        case MidiMsgType::STOP:
            // Transport messages — handled by sequencer; ignore in audio engine
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
