#include <Arduino.h>

#include <DigiKeyboard.h>

#include "entries.h"

void setup() {
  // don't need to set anything up to use DigiKeyboard
}


void loop() {
  // this is generally not necessary but with some older systems it seems to
  // prevent missing the first character after a delay:
  DigiKeyboard.sendKeyStroke(0);

  String test = String("LEDs: ") + String(DigiKeyboard.led_states);

  // Type out this string letter by letter on the computer (assumes US-style
  // keyboard)
  DigiKeyboard.println(test);

  // It's better to use DigiKeyboard.delay() over the regular Arduino delay()
  // if doing keyboard stuff because it keeps talking to the computer to make
  // sure the computer knows the keyboard is alive and connected
  DigiKeyboard.delay(5000);
}
