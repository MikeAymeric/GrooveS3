#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <stdint.h>

// ============================================================
// GrooveS3 — Internal MIDI message for inter-core queue
// ============================================================

enum class MidiMsgType : uint8_t {
    NOTE_ON  = 0x90,
    NOTE_OFF = 0x80,
    CC       = 0xB0,
    CLOCK    = 0xF8,
    START    = 0xFA,
    STOP     = 0xFC,
};

struct MidiMessage {
    MidiMsgType type;
    uint8_t     channel;   // 0-15 (channel 1-16)
    uint8_t     note;      // note number or CC number
    uint8_t     velocity;  // note velocity or CC value
};

// Handle to the queue shared between Core 0 (producer) and Core 1 (consumer).
// Defined in main.cpp, extern-declared here so both cores can access it.
extern QueueHandle_t gMidiQueue;
