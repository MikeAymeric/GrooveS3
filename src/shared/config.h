#pragma once

// ============================================================
// GrooveS3 — Global configuration constants
// ============================================================

// --- Sequencer ---
#define SEQ_STEPS           16
#define SEQ_TRACKS           6
#define SEQ_BPM_DEFAULT    120
#define SEQ_BPM_MIN         40
#define SEQ_BPM_MAX        200

// --- Audio ---
#define AUDIO_SAMPLE_RATE  44100
#define AUDIO_BUFFER_SIZE    512   // I2S DMA buffer in samples per channel
#define AUDIO_BITS_PER_SAMPLE 16

// --- Queue ---
#define MIDI_QUEUE_LENGTH   32    // inter-core FreeRTOS queue depth

// --- Display ---
#define DISPLAY_WIDTH      128
#define DISPLAY_HEIGHT      64
#define DISPLAY_I2C_ADDR  0x3C
#define DISPLAY_FPS_MAX     30

// --- Input ---
#define ADC_SAMPLES          8    // moving-average window for potentiometers
#define DEBOUNCE_MS         20    // software debounce for buttons

// --- SD card ---
#define SD_SPI_FREQ_HZ   4000000  // 4 MHz — conservative for reliability

// --- MIDI ---
#define MIDI_BAUD_RATE   31250
#define MIDI_CHANNEL         1    // default transmit/receive channel (1-16)

// --- Task stack sizes ---
#define TASK_SEQUENCER_STACK   8192
#define TASK_AUDIO_STACK      16384

// --- Task priorities ---
#define TASK_SEQUENCER_PRIORITY  1
#define TASK_AUDIO_PRIORITY      2

// --- Global variables ---
extern volatile float gMasterVolume;
