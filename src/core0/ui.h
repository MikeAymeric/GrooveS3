#pragma once

// ============================================================
// GrooveS3 — UI subsystem (Core 0, SH1106 128×64 via I2C)
// ============================================================

void uiInit();
void uiUpdate();  // call at ≤30 fps from sequencer/UI task
