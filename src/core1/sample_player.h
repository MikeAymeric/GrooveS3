#pragma once

#include "../shared/config.h"   // SEQ_TRACKS = 6
#include <Arduino.h>

// ============================================================
// GrooveS3 — PCM sample player (Core 1)
// Loads WAV files from MicroSD and plays them on trigger.
// Samples are expected in /samples/ on the SD root.
// ============================================================

// Use SEQ_TRACKS (6) as the voice count — do NOT redefine MAX_VOICES
// which is already set to 80 by ESP32Synth_Config.hpp.

void samplePlayerInit();
void samplePlayerLoad(uint8_t voice, const char* path);  // load WAV from SD
void samplePlayerTrigger(uint8_t voice, uint8_t velocity);
void samplePlayerProcess();             // mix into synth buffer
