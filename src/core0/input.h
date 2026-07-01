#pragma once

#include <stdint.h>

// ============================================================
// GrooveS3 — Input subsystem (Core 0)
// Handles: 74HC165 buttons, 74HC595 LEDs, EC11 encoders, ADC pots
// ============================================================

// --- Button bitmask layout from HC165 (16 step + 8 function) + dedicated SHIFT GPIO ---
// Bits 0-15  → step buttons
// Bits 16-23 → function buttons FB1-FB8 (74HC165 #3)
// Bit  24    → SHIFT (dedicated GPIO, held modifier — never toggle)
#define BTN_FB1         (1UL << 16)   // PLAY/STOP
#define BTN_FB2         (1UL << 17)   // REC
#define BTN_FB3         (1UL << 18)   // MODE: OVERVIEW
#define BTN_FB4         (1UL << 19)   // MODE: PATTERN
#define BTN_FB5         (1UL << 20)   // MODE: SOUND
#define BTN_FB6         (1UL << 21)   // MODE: NOTE
#define BTN_FB7         (1UL << 22)   // MODE: FX
#define BTN_FB8         (1UL << 23)   // MODE: MIXER
#define BTN_SHIFT       (1UL << 24)   // held modifier (dedicated GPIO)

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
