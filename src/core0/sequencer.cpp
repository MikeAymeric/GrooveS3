#include "sequencer.h"
#include "input.h"
#include "../shared/midi_message.h"
#include "../shared/config.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ============================================================
// Default MIDI note map per track (GM drum layout)
// ============================================================
static const uint8_t kDefaultNotes[SEQ_TRACKS] = {
    36,  // kick
    38,  // snare
    42,  // closed hi-hat
    46,  // open hi-hat
    39,  // clap
    75,  // clave / cowbell
};

// ============================================================
// State
// ============================================================

static Track    sTracks[SEQ_TRACKS];
static uint8_t  sCurrentStep  = 0;
static uint8_t  sActiveTrack  = 0;
static uint16_t sBpm          = SEQ_BPM_DEFAULT;
static bool     sPlaying      = false;

// ============================================================
// Helpers
// ============================================================

static void sendNote(uint8_t track, bool on) {
    MidiMessage msg;
    msg.type     = on ? MidiMsgType::NOTE_ON : MidiMsgType::NOTE_OFF;
    msg.channel  = sTracks[track].channel;
    msg.note     = sTracks[track].note;
    msg.velocity = on ? sTracks[track].velocity : 0;
    xQueueSend(gMidiQueue, &msg, 0);  // non-blocking; drop if full
}

static uint32_t stepIntervalMs() {
    // One step = one 16th note = 60000 / (BPM * 4)
    return 60000UL / ((uint32_t)sBpm * 4);
}

// ============================================================
// Public API
// ============================================================

void seqInit() {
    for (int t = 0; t < SEQ_TRACKS; t++) {
        sTracks[t].note     = kDefaultNotes[t];
        sTracks[t].velocity = 100;
        sTracks[t].channel  = 0;       // channel 1
        sTracks[t].midiLen  = 1;
        for (int s = 0; s < SEQ_STEPS; s++) sTracks[t].steps[s] = false;
    }
}

void seqToggleStep(uint8_t track, uint8_t step) {
    if (track >= SEQ_TRACKS || step >= SEQ_STEPS) return;
    sTracks[track].steps[step] = !sTracks[track].steps[step];
}

bool seqGetStep(uint8_t track, uint8_t step) {
    if (track >= SEQ_TRACKS || step >= SEQ_STEPS) return false;
    return sTracks[track].steps[step];
}

void seqSetBpm(uint16_t bpm) {
    sBpm = constrain(bpm, SEQ_BPM_MIN, SEQ_BPM_MAX);
}

uint16_t seqGetBpm()         { return sBpm; }
void     seqSetActiveTrack(uint8_t t) { if (t < SEQ_TRACKS) sActiveTrack = t; }
uint8_t  seqGetActiveTrack() { return sActiveTrack; }
uint8_t  seqGetCurrentStep() { return sCurrentStep; }
bool     seqIsPlaying()      { return sPlaying; }
void     seqPlay()           { sPlaying = true; sCurrentStep = 0; }
void     seqStop()           { sPlaying = false; }

// ============================================================
// FreeRTOS task — runs on Core 0
// ============================================================

void seqTask(void* /*arg*/) {
    seqInit();
    TickType_t xLastWake = xTaskGetTickCount();

    for (;;) {
        // Update BPM from potentiometer
        float potNorm = inputGetPotNorm(0);  // POT_BPM
        uint16_t newBpm = (uint16_t)(SEQ_BPM_MIN + potNorm * (SEQ_BPM_MAX - SEQ_BPM_MIN));
        seqSetBpm(newBpm);

        if (sPlaying) {
            // Trigger active steps for all tracks
            for (int t = 0; t < SEQ_TRACKS; t++) {
                if (sTracks[t].steps[sCurrentStep]) {
                    sendNote(t, true);
                }
            }

            // Advance step
            sCurrentStep = (sCurrentStep + 1) % SEQ_STEPS;

            // Note-off deferred to next step (length = 1 step)
            // TODO: implement per-track note length
        }

        // Sleep until next step boundary
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(stepIntervalMs()));
    }
}
