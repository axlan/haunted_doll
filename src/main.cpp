#include <Arduino.h>

#include <DigiKeyboard.h>

#include "keyboard_ui.h" 

const unsigned long MAX_DOUBLE_CLICK_TIME_MS = 1000;

unsigned long last_activate_time = 0;
bool active = false;
bool caps_lock_only = false;

uint8_t last_led_states = 0;

EntryUI entry_ui;

void Exit() { active = false; }

void ShowScore(const char* buffer) {
  entry_ui.SetExtraText(buffer);
}

// Bits 0 1 2 = Num Caps Scroll
bool LockPressed(uint8_t offset) {
  return bitRead(last_led_states ^ DigiKeyboardDevice::GetInstance().led_states, offset);
}

bool NumLockPressed() { return LockPressed(0); }

bool CapsLockPressed() { return LockPressed(1); }

bool ScrollLockPressed() { return LockPressed(2); }

void setup() {
  // don't need to set anything up to use DigiKeyboard
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
    if (DigiKeyboardDevice::GetInstance().led_states != last_led_states) {
      if (last_activate_time == 0 ||
          (millis() - last_activate_time) > MAX_DOUBLE_CLICK_TIME_MS) {
        last_activate_time = millis();
      } else {
        active = true;
        caps_lock_only = CapsLockPressed();
        entry_ui.ChooseEntry(0);
      }
    }
  } else {
    if(caps_lock_only) {
      HandleCapsOnly();
    } else {
      HandleFullKeyboard();
    }
  }

  last_led_states = DigiKeyboardDevice::GetInstance().led_states;
  DigiKeyboardDevice::GetInstance().delay(50);
}
