#include <Arduino.h>
#include <DigiKeyboard.h>

#include "entries.h"

const unsigned long MAX_DOUBLE_CLICK_TIME_MS = 1000;

unsigned long last_activate_time = 0;
bool active = false;

void Exit() { active = false; }

uint8_t last_led_states = 0;

char ToggleCase(char in) {
  const char CASE_OFFSET = 'a' - 'A';
  if (in >= 'a' && in <= 'z') {
    return in - CASE_OFFSET;
  }
  if (in >= 'A' && in <= 'Z') {
    return in + CASE_OFFSET;
  }
  return in;
}

void Print(const char* ptr, bool pgm_mem = false) {
  while(true) {
    char c;
    if (pgm_mem) {
      c = pgm_read_byte_near(ptr++);
    } else {
      c = *(ptr++);
    }
    if (c == 0) {
      break;
    }
    if (DigiKeyboard.led_states & 0b10) {
      c = ToggleCase(c);
    }
    DigiKeyboard.write(c);
  }
}

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
class EntryUI {
 public:
  void ChooseEntry(uint8_t idx) {
    entry_active = idx;
    selected_choice = 0;
    num_choices = CHOICE_COUNTS[idx];
    RepeatKeyStroke(KEY_EQUALS, NUM_COLS);
    RepeatKeyStroke(KEY_ENTER, 2);
    Print(ENTRIES[idx], true);
    RepeatKeyStroke(KEY_ENTER, 2);
    uint8_t choice_len = 0;
    for (int i = 0; i < num_choices; i++) {
      choice_len += strlen(MENU_KEYS[CHOICES[idx][i]]);
    }
    padding = (NUM_COLS - choice_len) / (num_choices + 1);
    for (int i = 0; i < num_choices; i++) {
      RepeatKeyStroke(KEY_SPACE, padding);
      Print(MENU_KEYS[CHOICES[idx][i]]);
    }
    DigiKeyboard.sendKeyStroke(KEY_ENTER);
    DigiKeyboard.sendKeyStroke(KEY_ARROW_UP);
    HighLight(selected_choice);
  }

  void IncrementChoice() {
    selected_choice = (selected_choice + 1) % num_choices;
    HighLight(selected_choice);
  }
  void DecrementChoice() {
    selected_choice =
        (selected_choice == 0) ? num_choices - 1 : selected_choice - 1;
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
  static const size_t NUM_COLS = 70;
  static const size_t LINES_BETWEEN_ENTRIES = 30;
  uint8_t entry_active = 0;
  uint8_t selected_choice = 0;
  uint8_t num_choices = 0;
  uint8_t padding = 0;

  void HighLight(uint8_t idx) {
    if (num_choices == 0) {
      return;
    }
    DigiKeyboard.sendKeyStroke(KEY_ARROW_UP);
    DigiKeyboard.sendKeyStroke(KEY_ARROW_RIGHT);
    int i;
    for (i = 0; i <= idx; i++) {
      RepeatKeyStroke(KEY_ARROW_RIGHT, padding);
      if (i == idx) {
        RepeatKeyStroke(KEY_ARROW_RIGHT, MOD_SHIFT_LEFT,
                        strlen(MENU_KEYS[CHOICES[entry_active][i]]));
      } else {
        RepeatKeyStroke(KEY_ARROW_RIGHT,
                        strlen(MENU_KEYS[CHOICES[entry_active][i]]));
      }
    }
  }
};

EntryUI entry_ui;

// Bits 0 1 2 = Num Caps Scroll
bool LockPressed(uint8_t offset) {
  return bitRead(last_led_states ^ DigiKeyboard.led_states, offset);
}

bool NumLockPressed() { return LockPressed(0); }

bool CapsLockPressed() { return LockPressed(1); }

bool ScrollLockPressed() { return LockPressed(2); }

void setup() {
  // don't need to set anything up to use DigiKeyboard
  pinMode(LED_BUILTIN, OUTPUT);
}

void HandleFullKeyboard() {
  if (NumLockPressed()) {
    entry_ui.IncrementChoice();
  } else if (CapsLockPressed()) {
    entry_ui.DecrementChoice();
  } else if (ScrollLockPressed()) {
    entry_ui.Select();
  }
}

unsigned long MAX_DOUBLE_CLICK_TIME_SELECT_MS = 350;
unsigned long last_select_time = 0;
void HandleCapsOnly() {
  if (CapsLockPressed()) {
    if (last_select_time == 0) {
      last_select_time = millis();
    } else {
      entry_ui.Select();
      last_select_time = 0;
    }
  } else if (last_select_time != 0 && (millis() - last_select_time) > MAX_DOUBLE_CLICK_TIME_SELECT_MS) {
    entry_ui.IncrementChoice();
    last_select_time = 0;
  }
}

void loop() {
  if (!active) {
    if (CapsLockPressed()) {
      if (last_activate_time == 0 ||
          (millis() - last_activate_time) > MAX_DOUBLE_CLICK_TIME_MS) {
        last_activate_time = millis();
      } else {
        active = true;
        entry_ui.ChooseEntry(0);
      }
    }
  } else {
    // HandleFullKeyboard();
    HandleCapsOnly();
  }
  digitalWrite(LED_BUILTIN, active);

  last_led_states = DigiKeyboard.led_states;
  DigiKeyboard.delay(50);
}
