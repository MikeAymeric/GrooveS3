#include "ui.h"
#include "sequencer.h"
#include "input.h"
#include "../shared/pinout.h"
#include "../shared/config.h"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ============================================================
// Display instance
// ============================================================

static Adafruit_SH1106G sDisplay(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);

static uint32_t sLastFrameMs = 0;
static const uint32_t kFrameMinMs = 1000 / DISPLAY_FPS_MAX;

// ============================================================
// Layout constants
// ============================================================

#define ROW_BPM_Y     0
#define ROW_TRACK_Y   10
#define ROW_STEPS_Y   24    // 16 step cells, 7px wide each
#define STEP_W        7
#define STEP_H        9
#define STEP_GAP      1

// ============================================================
// Init
// ============================================================

void uiInit() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    sDisplay.begin(DISPLAY_I2C_ADDR, true);
    sDisplay.setTextSize(1);
    sDisplay.setTextColor(SH110X_WHITE);
    sDisplay.clearDisplay();
    sDisplay.display();
}

// ============================================================
// Draw helpers
// ============================================================

static void drawBpm() {
    sDisplay.setCursor(0, ROW_BPM_Y);
    sDisplay.print("BPM:");
    sDisplay.print(seqGetBpm());
    sDisplay.print(seqIsPlaying() ? "  PLAY" : "  STOP");
}

static void drawTrack() {
    sDisplay.setCursor(0, ROW_TRACK_Y);
    sDisplay.print("TRK:");
    sDisplay.print(seqGetActiveTrack() + 1);
}

static void drawSteps() {
    uint8_t activeTrack = seqGetActiveTrack();
    uint8_t currentStep = seqGetCurrentStep();

    for (int s = 0; s < SEQ_STEPS; s++) {
        int x = s * (STEP_W + STEP_GAP);
        int y = ROW_STEPS_Y;
        bool active  = seqGetStep(activeTrack, s);
        bool current = (s == currentStep) && seqIsPlaying();

        if (active) {
            sDisplay.fillRect(x, y, STEP_W, STEP_H, SH110X_WHITE);
            if (current) {
                // Invert the current step box when playing
                sDisplay.fillRect(x + 1, y + 1, STEP_W - 2, STEP_H - 2, SH110X_BLACK);
            }
        } else {
            sDisplay.drawRect(x, y, STEP_W, STEP_H, SH110X_WHITE);
            if (current) {
                sDisplay.fillRect(x + 2, y + 2, STEP_W - 4, STEP_H - 4, SH110X_WHITE);
            }
        }
    }
}

// ============================================================
// Handle encoder input for UI navigation
// ============================================================

static void processEncoders() {
    int32_t delta1 = inputGetEncoderDelta(0);  // ENC1 → track selection
    if (delta1 != 0) {
        int8_t t = (int8_t)seqGetActiveTrack() + (delta1 > 0 ? 1 : -1);
        t = constrain(t, 0, SEQ_TRACKS - 1);
        seqSetActiveTrack((uint8_t)t);
    }

    // ENC1 click → play/stop toggle
    if (inputGetEncoderClick(0)) {
        if (seqIsPlaying()) seqStop();
        else                seqPlay();
    }

    // ENC2 → future: edit parameter of selected step
}

// ============================================================
// Public update — throttled to DISPLAY_FPS_MAX
// ============================================================

void uiUpdate() {
    uint32_t now = millis();
    if ((now - sLastFrameMs) < kFrameMinMs) return;
    sLastFrameMs = now;

    processEncoders();

    sDisplay.clearDisplay();
    drawBpm();
    drawTrack();
    drawSteps();
    sDisplay.display();
}
