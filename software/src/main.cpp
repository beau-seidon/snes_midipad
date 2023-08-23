#include <Arduino.h>
#include <iostream>
#include <MIDI.h>

#include <controller_pin_definitions.h>
#include <snes_controller.h>
#include <note_names.h>
#include <diatonic_modes.h>



MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midi2);

GamePad controller;

static const int note_button[] = { 
  GamePad::DOWN, GamePad::UP, GamePad::B, GamePad::LEFT,
  GamePad::Y, GamePad::RIGHT, GamePad::X, GamePad::A 
};

static const byte controller_midi_channel = 1;
int velocity = 100;

DiatonicMode mode;

const int *default_mode = mode.ionian;
static const int default_tonic = NOTE_C_4;

bool pitch_bend = false;
bool all_notes_off = true;

bool sustain[8] = {false};
int note[8];



/* snes midi controller functions ********************************************/

void set_notes(int tonic, const int *mode) {
  for (int n=0; n<8; n++) {
    note[n] = tonic + mode[n];
  }
}


void initialize_notes() {
  set_notes(default_tonic, default_mode);
}


bool pitch_mod_pressed() {
  return (controller.down(GamePad::START));
}


bool diatonic_mod_pressed() {
  return (controller.down(GamePad::SELECT));
}


void set_transpose() {

  int interval = 0;
  
  if(controller.pressed(GamePad::RIGHT)) interval += octave;
  if(controller.pressed(GamePad::LEFT)) interval -= octave;
  if(controller.pressed(GamePad::UP)) interval += semitone;
  if(controller.pressed(GamePad::DOWN)) interval -= semitone;

  if (interval == 0) return;

  for (int n=0; n<8; n++) {
    
    if ((note[n] + interval) > 127) {
      note[n] -= 127;
    } else if ((note[n] + interval) < 0) {
      note[n] += 127;
    }

    note[n] += interval;
  }

}


void set_diatonic_mode() {

  int tonic = note[0];
  int m = -1;

  for (int b=0; b<8; b++) {
    if (controller.pressed(note_button[b])) {
      m = b;
      break;
    }
  }

  switch (m) {

    case 0:
      set_notes(tonic, mode.ionian);
      break;

    case 1:
      set_notes(tonic, mode.dorian);
      break;

    case 2:
      set_notes(tonic, mode.phrygian);
      break;

    case 3:
      set_notes(tonic, mode.lydian);
      break;

    case 4:
      set_notes(tonic, mode.mixolydian);
      break;

    case 5:
      set_notes(tonic, mode.aeolian);
      break;

    case 6:
      set_notes(tonic, mode.locrian);
      break;                 
    
    case 7:
      set_notes(tonic, mode.ionian);
      break;
    
    default:
      return; 
  }
  
}


void handle_pitch_bend(byte channel) {
  if(!pitch_bend && controller.down(GamePad::R) && !controller.down(GamePad::L)) {
    midi2.sendPitchBend(MIDI_PITCHBEND_MAX, channel);
    pitch_bend = true;
    
  } else if(!pitch_bend && !controller.down(GamePad::R) && controller.down(GamePad::L)) {
    midi2.sendPitchBend(MIDI_PITCHBEND_MIN, channel);
    pitch_bend = true;

  } else if(pitch_bend && ((!controller.down(GamePad::R) && !controller.down(GamePad::L)) || (controller.down(GamePad::R) && controller.down(GamePad::L)))) {
    midi2.sendPitchBend(0, channel);
    pitch_bend = false;
  } 
}


void handle_modifiers() {
  if(pitch_mod_pressed() && !diatonic_mod_pressed()) {
    set_transpose();
  } else if(!pitch_mod_pressed() && diatonic_mod_pressed()) {
    set_diatonic_mode();
  } else if (pitch_mod_pressed() && diatonic_mod_pressed()) {
    initialize_notes();
  }
}


void sendAllNotesOff(int channel) {
  for(int i=0; i<128; i++) midi2.sendNoteOff(i, 0, channel);
  all_notes_off = true;
}


void handle_note_commands(int velocity, byte channel) {

  if(!pitch_mod_pressed() && !diatonic_mod_pressed()) {
    for (int b=0; b<8; b++) {
      
      if (controller.pressed(note_button[b])) {
          midi2.sendNoteOn(note[b], velocity, channel);
          all_notes_off = false;
          sustain[b] = true;
      }
      
      if(sustain[b]) {
        if (!controller.down(note_button[b])) {
          midi2.sendNoteOff(note[b], 0, channel);
          sustain[b] = false;
        }
      }

    }

  } else {
    if(!all_notes_off) {
      sendAllNotesOff(controller_midi_channel);
    }
  }

}



/* microcontroller functions **************************************************/

void setup() {
  midi2.begin(controller_midi_channel);
	controller.init(LATCH_PIN, CLOCK_PIN, DATA_PIN);
  initialize_notes();
}

void loop() {
	controller.poll();

  handle_modifiers();
  handle_pitch_bend(controller_midi_channel);
  handle_note_commands(velocity, controller_midi_channel);

  delay(2);
}


