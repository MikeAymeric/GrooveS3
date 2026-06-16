#include "midi.h"
#include "../shared/pinout.h"
#include "../shared/config.h"

#include <Arduino.h>

// UART1 for MIDI DIN
static HardwareSerial sMidiSerial(1);

// Simple single-byte MIDI parser state
static uint8_t sStatus  = 0;
static uint8_t sByte1   = 0;
static uint8_t sByteIdx = 0;

void midiInit() {
    sMidiSerial.begin(MIDI_BAUD_RATE, SERIAL_8N1, PIN_MIDI_RX, PIN_MIDI_TX);
}

void midiSendMessage(const MidiMessage& msg) {
    switch (msg.type) {
        case MidiMsgType::NOTE_ON:
        case MidiMsgType::NOTE_OFF:
        case MidiMsgType::CC:
            // Channel messages: status = type | channel (0-based)
            sMidiSerial.write((uint8_t)msg.type | (msg.channel & 0x0F));
            sMidiSerial.write(msg.note     & 0x7F);
            sMidiSerial.write(msg.velocity & 0x7F);
            break;
        case MidiMsgType::CLOCK:
        case MidiMsgType::START:
        case MidiMsgType::STOP:
            // Real-time: single byte, no channel field
            sMidiSerial.write((uint8_t)msg.type);
            break;
        default:
            break;
    }
}

void midiPoll() {
    while (sMidiSerial.available()) {
        uint8_t b = (uint8_t)sMidiSerial.read();

        // Real-time messages (0xF8-0xFF) are single-byte and can appear anywhere,
        // including in the middle of other messages. Handle them without touching
        // the running-status state machine.
        if (b >= 0xF8) {
            MidiMessage msg;
            msg.channel  = 0;
            msg.note     = 0;
            msg.velocity = 0;
            if      (b == 0xF8) msg.type = MidiMsgType::CLOCK;
            else if (b == 0xFA) msg.type = MidiMsgType::START;
            else if (b == 0xFC) msg.type = MidiMsgType::STOP;
            else continue;  // ignore other system real-time (0xFE active sensing, etc.)
            xQueueSend(gMidiQueue, &msg, 0);
            continue;
        }

        if (b & 0x80) {
            // Status byte — update running status and reset data byte counter
            sStatus  = b;
            sByteIdx = 0;
            sByte1   = 0;
            continue;
        }

        // Data bytes
        switch (sStatus & 0xF0) {
            case 0x90:  // NOTE ON
            case 0x80:  // NOTE OFF
            case 0xB0:  // CC
                if (sByteIdx == 0) {
                    sByte1 = b;
                    sByteIdx = 1;
                } else {
                    MidiMessage msg;
                    msg.type     = (sStatus & 0xF0) == 0x90 ? MidiMsgType::NOTE_ON
                                 : (sStatus & 0xF0) == 0x80 ? MidiMsgType::NOTE_OFF
                                 : MidiMsgType::CC;
                    msg.channel  = sStatus & 0x0F;
                    msg.note     = sByte1;
                    msg.velocity = b;
                    xQueueSend(gMidiQueue, &msg, 0);
                    sByteIdx = 0;
                }
                break;
            default:
                break;
        }
    }
}
