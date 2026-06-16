#pragma once

#include <stdint.h>

// ============================================================
// GrooveS3 — Input subsystem (Core 0)
// Handles: 74HC165 buttons, 74HC595 LEDs, EC11 encoders, ADC pots
// ============================================================

// --- Button bitmask layout from HC165 (16 step + 4 function) ---
// Bits 0-15  → step buttons
// Bits 16-19 → function buttons (PLAY, STOP, REC, TRACK_SEL)
#define BTN_PLAY        (1UL << 16)
#define BTN_STOP        (1UL << 17)
#define BTN_REC         (1UL << 18)
#define BTN_TRACK_SEL   (1UL << 19)

struct EncoderState {
    volatile int32_t position;   // accumulated clicks (+ CW, - CCW)
    volatile bool    pressed;    // true if push button currently held
    volatile bool    clicked;    // single-shot flag, cleared after read
};

void     inputInit();
void     inputPoll();            // call from Core 0 task loop

uint32_t inputGetButtons();      // current debounced button bitmask
bool     inputButtonPressed(uint8_t step);  // step 0-15

void     inputSetLed(uint8_t step, bool on);  // buffer LED state
void     inputFlushLeds();                    // shift out to HC595

int32_t  inputGetEncoderDelta(uint8_t enc);   // enc: 0 or 1; resets after read
bool     inputGetEncoderClick(uint8_t enc);   // true once per click

uint16_t inputGetPotRaw(uint8_t pot);         // pot: 0=BPM, 1=VOL, 2=PARAM (0-4095)
float    inputGetPotNorm(uint8_t pot);        // 0.0 – 1.0
