#include <Arduino.h>
#include <iostream>
#include <MIDI.h>

#include <controller_pin_definitions.h>
#include <snes_controller.h>
#include <note_names.h>
#include <diatonic_modes.h>



GamePad controller;
const int NOTE_BUTTON[] = {
    GamePad::DOWN, GamePad::UP, GamePad::B, GamePad::LEFT,
    GamePad::Y, GamePad::RIGHT, GamePad::X, GamePad::A
};


MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midi2);
const byte CONTROLLER_MIDI_CHANNEL = 1;
byte velocity = 100;


DiatonicMode mode;
const int *DEFAULT_DIATONIC_MODE = mode.ionian;
const int DEFAULT_TONIC = NOTE_C_4;


bool pitch_bend = false;
bool all_notes_off = true;

bool sustain[8] = {false};
int note[8];



// snes midi controller functions

void setNotes(int tonic, const int *MODE) {
    for (int n = 0; n < 8; n++) {
        note[n] = tonic + MODE[n];
    }
}


void initializeNotes() {
    setNotes(DEFAULT_TONIC, DEFAULT_DIATONIC_MODE);
}


bool pitchModPressed() {
    return (controller.down(GamePad::START));
}


bool diatonicModPressed() {
    return (controller.down(GamePad::SELECT));
}


void setTranspose() {

    int interval = 0;

    if (controller.pressed(GamePad::RIGHT)) interval += octave;
    if (controller.pressed(GamePad::LEFT)) interval -= octave;
    if (controller.pressed(GamePad::UP)) interval += semitone;
    if (controller.pressed(GamePad::DOWN)) interval -= semitone;

    if (interval == 0) return;

    for (int n = 0; n < 8; n++) {
        if ((note[n] + interval) > 127) {
            note[n] -= 127;
        } else if ((note[n] + interval) < 0) {
            note[n] += 127;
        }
        note[n] += interval;
    }
}


void setDiatonicMode() {

    int tonic = note[0];
    int m = -1;

    for (int b = 0; b < 8; b++) {
        if (controller.pressed(NOTE_BUTTON[b])) {
            m = b;
            break;
        }
    }

    switch (m) {
        case 0:
            setNotes(tonic, mode.ionian);
            break;

        case 1:
            setNotes(tonic, mode.dorian);
            break;

        case 2:
            setNotes(tonic, mode.phrygian);
            break;

        case 3:
            setNotes(tonic, mode.lydian);
            break;

        case 4:
            setNotes(tonic, mode.mixolydian);
            break;

        case 5:
            setNotes(tonic, mode.aeolian);
            break;

        case 6:
            setNotes(tonic, mode.locrian);
            break;

        case 7:
            setNotes(tonic, mode.ionian);
            break;

        default:
            return;
    }
}


void handlePitchBend(byte channel) {
    if (!pitch_bend && controller.down(GamePad::R) && !controller.down(GamePad::L)) {
        midi2.sendPitchBend(MIDI_PITCHBEND_MAX, channel);
        pitch_bend = true;

    } else if (!pitch_bend && !controller.down(GamePad::R) && controller.down(GamePad::L)) {
        midi2.sendPitchBend(MIDI_PITCHBEND_MIN, channel);
        pitch_bend = true;

    } else if (pitch_bend && ((!controller.down(GamePad::R) && !controller.down(GamePad::L)) || (controller.down(GamePad::R) && controller.down(GamePad::L)))) {
        midi2.sendPitchBend(0, channel);
        pitch_bend = false;
    }
}


void handleModifiers() {
    if (pitchModPressed() && !diatonicModPressed()) {
        setTranspose();

    } else if (!pitchModPressed() && diatonicModPressed()) {
        setDiatonicMode();

    } else if (pitchModPressed() && diatonicModPressed()) {
        initializeNotes();
    }
}


void sendAllNotesOff(int channel) {
    for (int i = 0; i < 128; i++) midi2.sendNoteOff(i, 0, channel);
    all_notes_off = true;
}


void handleNoteCommands(int velocity, byte channel) {

    if (!pitchModPressed() && !diatonicModPressed()) {
        for (int b=0; b<8; b++) {
            if (controller.pressed(NOTE_BUTTON[b])) {
                midi2.sendNoteOn(note[b], velocity, channel);
                all_notes_off = false;
                sustain[b] = true;
            }
            if (sustain[b]) {
                if (!controller.down(NOTE_BUTTON[b])) {
                    midi2.sendNoteOff(note[b], 0, channel);
                    sustain[b] = false;
                }
            }
        }

    } else {
        if (!all_notes_off) {
            sendAllNotesOff(CONTROLLER_MIDI_CHANNEL);
        }
    }
}



// mcu functions

void setup() {
    midi2.begin(CONTROLLER_MIDI_CHANNEL);
    controller.init(LATCH_PIN, CLOCK_PIN, DATA_PIN);
    initializeNotes();
}

void loop() {
    controller.poll();
    handleModifiers();
    handlePitchBend(CONTROLLER_MIDI_CHANNEL);
    handleNoteCommands(velocity, CONTROLLER_MIDI_CHANNEL);
}
