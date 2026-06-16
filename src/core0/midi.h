#pragma once

#include "../shared/midi_message.h"

// ============================================================
// GrooveS3 — MIDI DIN 5-pin IN/OUT (UART1, Core 0)
// ============================================================

void midiInit();
void midiSendMessage(const MidiMessage& msg);  // → MIDI OUT DIN
void midiPoll();                               // parse MIDI IN → gMidiQueue
