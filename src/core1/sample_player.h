#pragma once

#include <ESP32Synth.h>

// ============================================================
// GrooveS3 — PCM sample player (Core 1)
// Loads WAV files from MicroSD and plays them on trigger.
// Samples are expected in /samples/ on the SD root.
// ============================================================

#define MAX_VOICES   6     // one per sequencer track

void samplePlayerInit();
void samplePlayerLoad(uint8_t voice, const char* path);  // load WAV from SD
void samplePlayerTrigger(uint8_t voice, uint8_t velocity);
void samplePlayerProcess(ESP32Synth* synth);             // mix into synth buffer
