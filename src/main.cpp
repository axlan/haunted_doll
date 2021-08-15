#include <Arduino.h>

#include <DigiKeyboard.h>

#include "entries.h"

unsigned long max_double_click_time_ms = 1000;

unsigned long last_activate_time = 0;
bool active = false;

void Exit() {
  active = false;
}


uint8_t last_led_states = 0;

class EntryUI
{
 public:
  void ChooseEntry(uint8_t idx) {
    entry_active = idx;
    selected_choice = 0;
    num_choices = CHOICE_COUNTS[idx];
    RepeatKeyStroke(KEY_EQUALS, NUM_COLS);
    RepeatKeyStroke(KEY_ENTER, 2);
    DigiKeyboard.printf(ENTRIES[idx]);
    RepeatKeyStroke(KEY_ENTER, 2);
    uint8_t choice_len = 0;
    for (int i = 0; i < num_choices; i++) {
      choice_len += strlen(MENU_KEYS[CHOICES[idx][i]]);
    }
    padding = (NUM_COLS - choice_len) / (num_choices + 1);
    for (int i = 0; i < num_choices; i++) {
      RepeatKeyStroke(KEY_SPACE, padding);
      DigiKeyboard.printf(MENU_KEYS[CHOICES[idx][i]]);
    }
    RepeatKeyStroke(KEY_ENTER, 2);
    HighLight(selected_choice);
  }

  void IncrementChoice() {
    selected_choice = (selected_choice + 1) % num_choices;
    HighLight(selected_choice);
  }
  void DecrementChoice() {
    selected_choice = (selected_choice == 0) ? num_choices - 1 : selected_choice - 1;
    HighLight(selected_choice);
  }
  void Select() {
    RepeatKeyStroke(KEY_ARROW_DOWN, 3);
    RepeatKeyStroke(KEY_ENTER, LINES_BETWEEN_ENTRIES);
    uint8_t choice = CHOICES[entry_active][selected_choice];
    ENTRY_CALLBACKS[choice](entry_active, choice);
    ChooseEntry(choice);
  }
 private:
  static const size_t NUM_COLS = 100;
  static const size_t LINES_BETWEEN_ENTRIES = 20;
  uint8_t entry_active = 0;
  uint8_t selected_choice = 0;
  uint8_t num_choices = 0;
  uint8_t padding = 0;
  
  void RepeatKeyStroke(byte stroke, size_t num) {
    for (size_t i = 0; i < num; i++) {
      DigiKeyboard.sendKeyStroke(stroke);
    }
  }

    void RepeatKeyStroke(byte stroke, byte modifiers, size_t num) {
    for (size_t i = 0; i < num; i++) {
      DigiKeyboard.sendKeyStroke(stroke, modifiers);
    }
  }

  void HighLight(uint8_t idx) {
    if (num_choices == 0) {
      return;
    }
    DigiKeyboard.sendKeyStroke(KEY_ARROW_DOWN);
    DigiKeyboard.sendKeyStroke(KEY_ARROW_UP);
    int i;
    for(i = 0; i <= idx; i++) {
      RepeatKeyStroke(KEY_ARROW_RIGHT, padding);
      if (i == idx) {
        RepeatKeyStroke(KEY_ARROW_RIGHT, MOD_SHIFT_LEFT, strlen(MENU_KEYS[CHOICES[entry_active][i]]));
      } else {
        RepeatKeyStroke(KEY_ARROW_RIGHT, strlen(MENU_KEYS[CHOICES[entry_active][i]]));
      }
    }
  }
};

EntryUI entry_ui;

//Bits 0 1 2 = Num Caps Scroll
bool LockPressed(uint8_t offset) {
  return bitRead(last_led_states ^ DigiKeyboard.led_states, offset);
}

bool NumLockPressed() {
  return LockPressed(0);
}

bool CapsLockPressed() {
  return LockPressed(1);
}

bool ScrollLockPressed() {
  return LockPressed(2);
}

void setup() {
  // don't need to set anything up to use DigiKeyboard
}

void loop() {
  
  if (!active) {
    if (NumLockPressed()) {
      if (last_activate_time == 0 || (millis() - last_activate_time) > max_double_click_time_ms) {
        last_activate_time = millis();
      } else {
        active = true;
        entry_ui.ChooseEntry(0);
      }
    }
  } else {
    if (NumLockPressed()) {
      entry_ui.IncrementChoice();
    } else if (CapsLockPressed()) {
      entry_ui.DecrementChoice();
    } else if (ScrollLockPressed()) {
      entry_ui.Select();
    }
  }
  
  last_led_states = DigiKeyboard.led_states;
  
}
