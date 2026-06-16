#include "sample_player.h"
#include "../shared/config.h"
#include "../shared/pinout.h"

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>

// ============================================================
// SPI bus mutex — shared between Core 0 (HC165/HC595) and Core 1 (SD).
// Defined here (Core 1 owner); extern-declared in input.cpp.
// ============================================================
SemaphoreHandle_t gSpiMutex = nullptr;

static SdFat   sSD;
static bool    sSdReady = false;

// Renamed from Voice to avoid collision with ESP32Synth's Voice struct.
struct SampleVoice {
    int16_t* buffer;      // PCM data in PSRAM (nullptr if not loaded)
    uint32_t length;      // total samples
    uint32_t cursor;      // playback cursor
    bool     playing;
    float    gainL;
    float    gainR;
};

static SampleVoice sVoices[SEQ_TRACKS] = {};

void samplePlayerInit() {
    // Create SPI mutex before any SPI transaction
    gSpiMutex = xSemaphoreCreateMutex();

    if (xSemaphoreTake(gSpiMutex, pdMS_TO_TICKS(500)) != pdTRUE) {
        Serial.println("[SD] Could not acquire SPI mutex at init");
        return;
    }

    bool ok = sSD.begin(PIN_SD_CS, SD_SCK_MHZ(4));

    xSemaphoreGive(gSpiMutex);

    if (!ok) {
        Serial.println("[SD] Mount failed — sample player disabled");
        return;
    }
    sSdReady = true;

    // Default sample map (files must exist on SD)
    const char* defaults[SEQ_TRACKS] = {
        "/samples/kick.wav",
        "/samples/snare.wav",
        "/samples/hihat_cl.wav",
        "/samples/hihat_op.wav",
        "/samples/clap.wav",
        "/samples/clave.wav",
    };
    for (int i = 0; i < SEQ_TRACKS; i++) {
        samplePlayerLoad(i, defaults[i]);
    }
}

void samplePlayerLoad(uint8_t voice, const char* path) {
    if (!sSdReady || voice >= SEQ_TRACKS) return;

    if (xSemaphoreTake(gSpiMutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        Serial.printf("[SD] SPI busy, skipping load of %s\n", path);
        return;
    }

    FsFile f = sSD.open(path, FILE_READ);
    xSemaphoreGive(gSpiMutex);

    if (!f) {
        Serial.printf("[SD] Cannot open %s\n", path);
        return;
    }

    // Skip 44-byte WAV header (assumes 16-bit PCM, no extra chunks)
    f.seek(44);
    uint32_t dataBytes = f.size() - 44;
    uint32_t samples   = dataBytes / sizeof(int16_t);

    // Free previous buffer if present
    if (sVoices[voice].buffer) {
        heap_caps_free(sVoices[voice].buffer);
        sVoices[voice].buffer = nullptr;
    }

    int16_t* buf = (int16_t*)heap_caps_malloc(dataBytes, MALLOC_CAP_SPIRAM);
    if (!buf) {
        Serial.printf("[PSRAM] Alloc failed for %s (%lu bytes)\n", path, dataBytes);
        f.close();
        return;
    }

    if (xSemaphoreTake(gSpiMutex, pdMS_TO_TICKS(500)) != pdTRUE) {
        heap_caps_free(buf);
        f.close();
        return;
    }
    f.read(buf, dataBytes);
    xSemaphoreGive(gSpiMutex);

    f.close();

    sVoices[voice].buffer  = buf;
    sVoices[voice].length  = samples;
    sVoices[voice].cursor  = 0;
    sVoices[voice].playing = false;
    sVoices[voice].gainL   = 1.0f;
    sVoices[voice].gainR   = 1.0f;

    Serial.printf("[SD] Loaded %s (%lu samples)\n", path, samples);
}

void samplePlayerTrigger(uint8_t voice, uint8_t /*velocity*/) {
    if (voice >= SEQ_TRACKS || !sVoices[voice].buffer) return;
    sVoices[voice].cursor  = 0;
    sVoices[voice].playing = true;
}

void samplePlayerProcess(ESP32Synth* /*synth*/) {
    // TODO Phase 2: integrate with ESP32Synth mixing pipeline.
    for (int v = 0; v < SEQ_TRACKS; v++) {
        if (!sVoices[v].playing || !sVoices[v].buffer) continue;
        SampleVoice& vx = sVoices[v];
        if (vx.cursor >= vx.length) { vx.playing = false; continue; }
        vx.cursor = min(vx.cursor + (uint32_t)AUDIO_BUFFER_SIZE, vx.length);
        if (vx.cursor >= vx.length) vx.playing = false;
    }
}
