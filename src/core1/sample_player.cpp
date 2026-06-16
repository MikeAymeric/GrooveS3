#include "sample_player.h"
#include "../shared/config.h"
#include "../shared/pinout.h"

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>

static SdFat   sSD;
static bool    sSdReady = false;

struct Voice {
    int16_t* buffer;      // PCM data in PSRAM
    uint32_t length;      // total samples
    uint32_t cursor;      // playback cursor
    bool     playing;
    float    gainL;       // 0.0-1.0
    float    gainR;
};

static Voice sVoices[MAX_VOICES] = {};

void samplePlayerInit() {
    if (!sSD.begin(PIN_SD_CS, SD_SCK_MHZ(4))) {
        Serial.println("[SD] Mount failed — sample player disabled");
        return;
    }
    sSdReady = true;

    // Default sample map (files must exist on SD)
    const char* defaults[MAX_VOICES] = {
        "/samples/kick.wav",
        "/samples/snare.wav",
        "/samples/hihat_cl.wav",
        "/samples/hihat_op.wav",
        "/samples/clap.wav",
        "/samples/clave.wav",
    };
    for (int i = 0; i < MAX_VOICES; i++) {
        samplePlayerLoad(i, defaults[i]);
    }
}

void samplePlayerLoad(uint8_t voice, const char* path) {
    if (!sSdReady || voice >= MAX_VOICES) return;

    FsFile f = sSD.open(path, FILE_READ);
    if (!f) {
        Serial.printf("[SD] Cannot open %s\n", path);
        return;
    }

    // Skip 44-byte WAV header (assumes 16-bit PCM, no extra chunks)
    f.seek(44);
    uint32_t dataBytes = f.size() - 44;
    uint32_t samples   = dataBytes / sizeof(int16_t);

    // Allocate in PSRAM
    if (sVoices[voice].buffer) heap_caps_free(sVoices[voice].buffer);
    sVoices[voice].buffer  = (int16_t*)heap_caps_malloc(dataBytes, MALLOC_CAP_SPIRAM);
    sVoices[voice].length  = samples;
    sVoices[voice].cursor  = 0;
    sVoices[voice].playing = false;
    sVoices[voice].gainL   = 1.0f;
    sVoices[voice].gainR   = 1.0f;

    if (!sVoices[voice].buffer) {
        Serial.printf("[PSRAM] Alloc failed for %s (%lu bytes)\n", path, dataBytes);
        f.close();
        return;
    }

    f.read(sVoices[voice].buffer, dataBytes);
    f.close();
    Serial.printf("[SD] Loaded %s (%lu samples)\n", path, samples);
}

void samplePlayerTrigger(uint8_t voice, uint8_t /*velocity*/) {
    if (voice >= MAX_VOICES || !sVoices[voice].buffer) return;
    sVoices[voice].cursor  = 0;
    sVoices[voice].playing = true;
}

void samplePlayerProcess(ESP32Synth* /*synth*/) {
    // Mix active voices into a stereo interleaved scratch buffer
    // and write to I2S directly.  ESP32Synth's DMA callback is used
    // elsewhere; here we write via I2S driver when no synth notes are active.
    // TODO: integrate with ESP32Synth mixing pipeline in Phase 2.
    for (int v = 0; v < MAX_VOICES; v++) {
        if (!sVoices[v].playing) continue;
        Voice& vx = sVoices[v];
        if (vx.cursor >= vx.length) { vx.playing = false; continue; }
        // Advance cursor by buffer size — actual mixing TBD
        vx.cursor = min(vx.cursor + AUDIO_BUFFER_SIZE, vx.length);
        if (vx.cursor >= vx.length) vx.playing = false;
    }
}
