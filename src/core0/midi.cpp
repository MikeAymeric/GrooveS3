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
    uint8_t status = (uint8_t)msg.type | (msg.channel & 0x0F);
    sMidiSerial.write(status);

    switch (msg.type) {
        case MidiMsgType::NOTE_ON:
        case MidiMsgType::NOTE_OFF:
        case MidiMsgType::CC:
            sMidiSerial.write(msg.note & 0x7F);
            sMidiSerial.write(msg.velocity & 0x7F);
            break;
        case MidiMsgType::CLOCK:
        case MidiMsgType::START:
        case MidiMsgType::STOP:
            // Single-byte real-time messages — status already sent
            break;
        default:
            break;
    }
}

void midiPoll() {
    while (sMidiSerial.available()) {
        uint8_t b = (uint8_t)sMidiSerial.read();

        if (b & 0x80) {
            // Status byte
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
