#include "input.h"
#include "../shared/pinout.h"
#include "../shared/config.h"

#include <Arduino.h>
#include <SPI.h>
#include <freertos/semphr.h>

// SPI bus mutex shared with sample_player.cpp (Core 1 SD access)
extern SemaphoreHandle_t gSpiMutex;

// ============================================================
// Static state
// ============================================================

static uint32_t sButtonsCurrent  = 0;
static uint32_t sButtonsPrev     = 0;
static uint32_t sButtonsDebounce = 0;
static uint32_t sDebounceTime    = 0;

static uint16_t sLedBuffer = 0;

// Encoder state (accessed from ISR)
static EncoderState sEnc[2] = {};

// ADC moving-average buffers
static uint16_t sAdcBuf[3][ADC_SAMPLES] = {};
static uint8_t  sAdcIdx[3] = {};

static const uint8_t kPotPins[3] = { PIN_POT_BPM, PIN_POT_VOL, PIN_POT_PARAM };
static const uint8_t kEncAPins[2] = { PIN_ENC1_A, PIN_ENC2_A };
static const uint8_t kEncBPins[2] = { PIN_ENC1_B, PIN_ENC2_B };
static const uint8_t kEncBtnPins[2] = { PIN_ENC1_BTN, PIN_ENC2_BTN };

// ============================================================
// Encoder ISRs
// ============================================================

// Both the helper and the dispatchers must be IRAM_ATTR so the linker
// places them in IRAM and avoids cache-miss crashes during flash access.
static void IRAM_ATTR isr_enc(int idx) {
    bool a = digitalRead(kEncAPins[idx]);
    bool b = digitalRead(kEncBPins[idx]);
    if (a != b) sEnc[idx].position = sEnc[idx].position + 1;
    else        sEnc[idx].position = sEnc[idx].position - 1;
}

static void IRAM_ATTR isr_enc0() { isr_enc(0); }
static void IRAM_ATTR isr_enc1() { isr_enc(1); }

// ============================================================
// Initialisation
// ============================================================

void inputInit() {
    // HC165 load pin (active-low)
    pinMode(PIN_HC165_LOAD, OUTPUT);
    digitalWrite(PIN_HC165_LOAD, HIGH);

    // HC595 latch pin (active-high)
    pinMode(PIN_HC595_LATCH, OUTPUT);
    digitalWrite(PIN_HC595_LATCH, LOW);

    // SPI already started by main; if not, start here
    SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, PIN_SPI_MOSI);

    // Encoder GPIOs + interrupts
    for (int i = 0; i < 2; i++) {
        pinMode(kEncAPins[i], INPUT_PULLUP);
        pinMode(kEncBPins[i], INPUT_PULLUP);
        pinMode(kEncBtnPins[i], INPUT_PULLUP);
    }
    attachInterrupt(digitalPinToInterrupt(PIN_ENC1_A), isr_enc0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC2_A), isr_enc1, CHANGE);

    // Shift button
    pinMode(PIN_SHIFT_BTN, INPUT_PULLUP);

    // Pre-fill ADC buffers
    for (int p = 0; p < 3; p++) {
        uint16_t v = analogRead(kPotPins[p]);
        for (int s = 0; s < ADC_SAMPLES; s++) sAdcBuf[p][s] = v;
    }
}

// ============================================================
// HC165 — read 20 bits (16 step + 4 function) via SPI
// We chain two HC165 (16 bits) and pad 4 more bits via SHIFT_BTN
// Actual hardware: one 16-bit chain, function buttons on dedicated GPIOs
// ============================================================

static uint32_t hc165Read() {
    uint8_t hi = 0, lo = 0;

    if (gSpiMutex && xSemaphoreTake(gSpiMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
        // Pulse LOAD low to latch parallel inputs
        digitalWrite(PIN_HC165_LOAD, LOW);
        delayMicroseconds(1);
        digitalWrite(PIN_HC165_LOAD, HIGH);

        SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
        hi = SPI.transfer(0xFF);
        lo = SPI.transfer(0xFF);
        SPI.endTransaction();

        xSemaphoreGive(gSpiMutex);
    }

    // HC165 outputs 1 when button is open, 0 when pressed — invert
    uint16_t steps = ~((uint16_t)(hi << 8) | lo);

    // Function buttons on dedicated GPIO (active-low)
    uint32_t func = 0;
    if (!digitalRead(PIN_SHIFT_BTN)) func |= BTN_PLAY;

    return (func << 16) | steps;
}

// ============================================================
// HC595 — shift out 16-bit LED state
// ============================================================

void inputFlushLeds() {
    if (!gSpiMutex || xSemaphoreTake(gSpiMutex, pdMS_TO_TICKS(5)) != pdTRUE) return;
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(PIN_HC595_LATCH, LOW);
    SPI.transfer16(sLedBuffer);
    digitalWrite(PIN_HC595_LATCH, HIGH);
    SPI.endTransaction();
    xSemaphoreGive(gSpiMutex);
}

void inputSetLed(uint8_t step, bool on) {
    if (step >= 16) return;
    if (on) sLedBuffer |=  (1U << step);
    else    sLedBuffer &= ~(1U << step);
}

// ============================================================
// ADC — moving average
// ============================================================

static uint16_t adcRead(uint8_t pot) {
    uint16_t raw = (uint16_t)analogRead(kPotPins[pot]);
    sAdcBuf[pot][sAdcIdx[pot]] = raw;
    sAdcIdx[pot] = (sAdcIdx[pot] + 1) % ADC_SAMPLES;
    uint32_t sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) sum += sAdcBuf[pot][i];
    return (uint16_t)(sum / ADC_SAMPLES);
}

// ============================================================
// Public poll — call once per task loop iteration
// ============================================================

void inputPoll() {
    // Buttons with 20ms debounce
    uint32_t raw = hc165Read();
    uint32_t now = millis();
    if (raw != sButtonsDebounce) {
        sButtonsDebounce = raw;
        sDebounceTime = now;
    }
    if ((now - sDebounceTime) >= DEBOUNCE_MS) {
        sButtonsPrev    = sButtonsCurrent;
        sButtonsCurrent = sButtonsDebounce;
    }

    // Encoder click debounce (simple edge detection)
    for (int i = 0; i < 2; i++) {
        bool pressed = !digitalRead(kEncBtnPins[i]);
        if (pressed && !sEnc[i].pressed) sEnc[i].clicked = true;
        sEnc[i].pressed = pressed;
    }

    // ADC refresh
    for (int p = 0; p < 3; p++) adcRead(p);
}

// ============================================================
// Public accessors
// ============================================================

uint32_t inputGetButtons() { return sButtonsCurrent; }

bool inputButtonPressed(uint8_t step) {
    if (step >= 16) return false;
    bool prev = (sButtonsPrev >> step) & 1;
    bool curr = (sButtonsCurrent >> step) & 1;
    return curr && !prev;  // rising edge only
}

int32_t inputGetEncoderDelta(uint8_t enc) {
    if (enc > 1) return 0;
    // Disable interrupts for the read-modify-write so the ISR cannot fire
    // between reading position and clearing it.
    portDISABLE_INTERRUPTS();
    int32_t delta = sEnc[enc].position;
    sEnc[enc].position = 0;
    portENABLE_INTERRUPTS();
    return delta;
}

bool inputGetEncoderClick(uint8_t enc) {
    if (enc > 1) return false;
    bool c = sEnc[enc].clicked;
    sEnc[enc].clicked = false;
    return c;
}

uint16_t inputGetPotRaw(uint8_t pot) {
    if (pot > 2) return 0;
    uint32_t sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) sum += sAdcBuf[pot][i];
    return (uint16_t)(sum / ADC_SAMPLES);
}

float inputGetPotNorm(uint8_t pot) {
    return inputGetPotRaw(pot) / 4095.0f;
}
