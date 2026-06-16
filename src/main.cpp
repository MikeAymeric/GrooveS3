#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

#include "shared/midi_message.h"
#include "shared/config.h"
#include "core0/sequencer.h"
#include "core0/input.h"
#include "core0/ui.h"
#include "core0/midi.h"
#include "core1/audio_engine.h"

// ============================================================
// Inter-core queue (defined here, extern in midi_message.h)
// ============================================================
QueueHandle_t gMidiQueue = nullptr;

// ============================================================
// Core 0 task: sequencer + input + UI + MIDI I/O
// ============================================================
static void taskSequencer(void* /*arg*/) {
    inputInit();
    uiInit();
    midiInit();

    // Kick off the sequencer loop
    seqTask(nullptr);  // seqTask contains its own infinite loop
}

// ============================================================
// Core 1 task: audio engine (ESP32Synth + sample player)
// ============================================================
static void taskAudio(void* /*arg*/) {
    audioEngineTask(nullptr);  // audioEngineTask contains its own infinite loop
}

// ============================================================
// Arduino setup — runs on Core 1 by default
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("[GrooveS3] Boot");

    // Create inter-core MIDI queue
    gMidiQueue = xQueueCreate(MIDI_QUEUE_LENGTH, sizeof(MidiMessage));
    if (!gMidiQueue) {
        Serial.println("[ERROR] Queue creation failed — halting");
        while (true) {}
    }

    // Pin Core 0 task: sequencer + UI
    xTaskCreatePinnedToCore(
        taskSequencer,
        "Sequencer",
        TASK_SEQUENCER_STACK,
        nullptr,
        TASK_SEQUENCER_PRIORITY,
        nullptr,
        0   // Core 0
    );

    // Pin Core 1 task: audio engine
    xTaskCreatePinnedToCore(
        taskAudio,
        "AudioEngine",
        TASK_AUDIO_STACK,
        nullptr,
        TASK_AUDIO_PRIORITY,
        nullptr,
        1   // Core 1
    );

    Serial.println("[GrooveS3] Tasks started");
}

// Arduino loop runs on Core 1 at lowest priority — not used
void loop() {
    vTaskDelay(portMAX_DELAY);
}
