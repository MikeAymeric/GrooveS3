#pragma once

#include <stdint.h>
#include "../shared/config.h"

// ============================================================
// GrooveS3 — Step sequencer (Core 0)
// 16 steps × 6 tracks; MIDI note dispatch via FreeRTOS queue
// ============================================================

struct Track {
    bool    steps[SEQ_STEPS];   // true = step active
    bool    is_pcm;             // true = track is playing PCM samples
    uint8_t note;               // MIDI note number for this track
    uint8_t velocity;           // default note velocity
    uint8_t channel;            // MIDI channel (0-based)
    uint8_t midiLen;            // note length in steps (1 = 1 step)
};

void     seqInit();
void     seqTask(void* arg);    // FreeRTOS task entry point (Core 0)

void     seqToggleStep(uint8_t track, uint8_t step);
bool     seqGetStep(uint8_t track, uint8_t step);
void     seqSetBpm(uint16_t bpm);
uint16_t seqGetBpm();
void     seqSetActiveTrack(uint8_t track);
uint8_t  seqGetActiveTrack();
uint8_t  seqGetCurrentStep();
bool     seqIsPlaying();
void     seqPlay();
void     seqStop();
