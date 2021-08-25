/*
 * Based on Obdev's AVRUSB code and under the same license.
 * Modified for Digispark by Digistump
 * Further modified by Axlan to capture indicator LEDs based on
 * https://github.com/7enderhead/kbdwtchdg
 */

#ifndef __DigiKeyboard_h__
#define __DigiKeyboard_h__

#include <Arduino.h>

/* Keyboard usage values, see usb.org's HID-usage-tables document, chapter
 * 10 Keyboard/Keypad Page for more codes.
 */
#define MOD_CONTROL_LEFT    (1<<0)
#define MOD_SHIFT_LEFT      (1<<1)
#define MOD_ALT_LEFT        (1<<2)
#define MOD_GUI_LEFT        (1<<3)
#define MOD_CONTROL_RIGHT   (1<<4)
#define MOD_SHIFT_RIGHT     (1<<5)
#define MOD_ALT_RIGHT       (1<<6)
#define MOD_GUI_RIGHT       (1<<7)

#define KEY_A       4
#define KEY_B       5
#define KEY_C       6
#define KEY_D       7
#define KEY_E       8
#define KEY_F       9
#define KEY_G       10
#define KEY_H       11
#define KEY_I       12
#define KEY_J       13
#define KEY_K       14
#define KEY_L       15
#define KEY_M       16
#define KEY_N       17
#define KEY_O       18
#define KEY_P       19
#define KEY_Q       20
#define KEY_R       21
#define KEY_S       22
#define KEY_T       23
#define KEY_U       24
#define KEY_V       25
#define KEY_W       26
#define KEY_X       27
#define KEY_Y       28
#define KEY_Z       29
#define KEY_1       30
#define KEY_2       31
#define KEY_3       32
#define KEY_4       33
#define KEY_5       34
#define KEY_6       35
#define KEY_7       36
#define KEY_8       37
#define KEY_9       38
#define KEY_0       39

#define KEY_ENTER   40
#define KEY_ESCAPE  41
#define KEY_BACKSPACE  42
#define KEY_TAB     43
#define KEY_SPACE   44
#define KEY_MINUS   45
#define KEY_EQUALS  46
#define KEY_LBRACKET 47
#define KEY_RBRACKET 48
#define KEY_BACKSLASH 49
#define KEY_NONUS_NUMBER 50
#define KEY_SEMICOLON 51
#define KEY_QUOTE   52
#define KEY_TILDE   53
#define KEY_COMMA   54
#define KEY_PERIOD  55
#define KEY_SLASH   56
#define KEY_CAPSLOCK 57

#define KEY_F1      58
#define KEY_F2      59
#define KEY_F3      60
#define KEY_F4      61
#define KEY_F5      62
#define KEY_F6      63
#define KEY_F7      64
#define KEY_F8      65
#define KEY_F9      66
#define KEY_F10     67
#define KEY_F11     68
#define KEY_F12     69

#define KEY_ARROW_RIGHT 0x4F
#define KEY_ARROW_LEFT  0x50
#define KEY_ARROW_DOWN  0x51
#define KEY_ARROW_UP    0x52

char ToggleCase(char in);

class DigiKeyboardDevice : public Print {
 public:
  DigiKeyboardDevice ();
    
  void update();
	
	// delay while updating until we are finished delaying
	void delay(long milli);
  
  //sendKeyStroke: sends a key press AND release
  void sendKeyStroke(uint8_t keyStroke);

  //sendKeyStroke: sends a key press AND release with modifiers
  void sendKeyStroke(uint8_t keyStroke, uint8_t modifiers);

  //sendKeyPress: sends a key press only - no release
  //to release the key, send again with keyPress=0
  void sendKeyPress(uint8_t keyPress);

  //sendKeyPress: sends a key press only, with modifiers - no release
  //to release the key, send again with keyPress=0
  void sendKeyPress(uint8_t keyPress, uint8_t modifiers);
  
  size_t write(uint8_t chr);

  void LightPrint(const char* ptr, bool pgm_mem = false);

  void RepeatKeyStroke(uint8_t stroke, size_t num);

  void RepeatKeyStroke(uint8_t stroke, uint8_t modifiers, size_t num);

  using Print::write;

  static DigiKeyboardDevice& GetInstance();

  struct KeyboardReport{
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
  };

  KeyboardReport    reportBuffer;
  uint8_t    led_states;
  static DigiKeyboardDevice instance;
};

#endif // __DigiKeyboard_h__
