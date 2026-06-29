#include "audio_engine.h"
#include "sample_player.h"
#include "../shared/midi_message.h"
#include "../shared/config.h"
#include "../shared/pinout.h"

#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <AMY-Arduino.h>

// ============================================================
// I2S configuration (PCM5102A, 16-bit stereo 44.1 kHz)
// ============================================================

// Pin definiti in pinout.h: PIN_I2S_DIN=38, PIN_I2S_BCLK=39, PIN_I2S_LRCK=40
static void initI2S() {
    amy_config_t c = amy_default_config();
    c.audio    = AMY_AUDIO_IS_I2S;
    c.i2s_dout = PIN_I2S_DIN;     // DAC data in = nostro DIN out
    c.i2s_bclk = PIN_I2S_BCLK;
    c.i2s_lrc  = PIN_I2S_LRCK;
    amy_start(c);
}

// ============================================================
// MIDI dispatch
// ============================================================

static void dispatchMidi(const MidiMessage& msg) {
    amy_event e = amy_default_event();
    e.osc = msg.channel;  // voce 0-5

    switch (msg.type) {
        case MidiMsgType::NOTE_ON:
            e.midi_note = (float)msg.note;
            e.velocity  = (msg.velocity > 0) ? msg.velocity / 127.0f : 0.0f;
            amy_add_event(&e);
            break;
        case MidiMsgType::NOTE_OFF:
            e.velocity = 0.0f;
            amy_add_event(&e);
            break;
        default:
            break;
    }
}

// ============================================================
// Init & task
// ============================================================

void audioEngineInit() {
    initI2S();
    amy_event e = amy_default_event();
    e.osc = 0;
    e.wave = PCM;
    e.preset = 1;
    amy_add_event(&e);
    e = amy_default_event();
    e.osc = 1;
    e.wave = PCM;
    e.preset = 2;
    amy_add_event(&e);
    samplePlayerInit();
}

void audioEngineTask(void* /*arg*/) {
    audioEngineInit();

    MidiMessage msg;
    for (;;) {
        while (xQueueReceive(gMidiQueue, &msg, 0) == pdTRUE) {
            dispatchMidi(msg);
        }
        amy_update();
        static float sPrevVolume = 1.0f;
        float vol = gMasterVolume;
        if (vol != sPrevVolume) {
            sPrevVolume = vol;
            amy_event ve = amy_default_event();
            ve.volume[0] = vol;
            amy_add_event(&ve);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }   
}
