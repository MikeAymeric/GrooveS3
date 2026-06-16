#pragma once

// ============================================================
// GrooveS3 — Audio engine (Core 1)
// Wraps ESP32Synth + I2S output on PCM5102A
// ============================================================

void audioEngineInit();
void audioEngineTask(void* arg);  // FreeRTOS task entry point (Core 1)
