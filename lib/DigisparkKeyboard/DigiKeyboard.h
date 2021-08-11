/*
 * Based on Obdev's AVRUSB code and under the same license.
 *
 * TODO: Make a proper file header. :-)
 * Modified for Digispark by Digistump
 */
#ifndef __DigiKeyboard_h__
#define __DigiKeyboard_h__

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "usbdrv.h"
#include "scancode-ascii-table.h"

// TODO: Work around Arduino 12 issues better.
//#include <WConstants.h>
//#undef int()

typedef uint8_t byte;


#define BUFFER_SIZE 2 // Minimum of 2: 1 for modifiers + 1 for keystroke 


static uchar    idleRate;           // in 4 ms units 
static uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6

/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and but we do allow
 * simultaneous key presses. 
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */
const PROGMEM unsigned char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
   0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
   0x09, 0x06,                    // USAGE (Keyboard)
   0xa1, 0x01,                    // COLLECTION (Application)
   0x75, 0x01,                    //   REPORT_SIZE (1)
   0x95, 0x08,                    //   REPORT_COUNT (8)
   0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
   0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)(224)
   0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
   0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
   0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
   0x81, 0x02,                    //   INPUT (Data,Var,Abs) ; Modifier byte
   0x95, 0x01,                    //   REPORT_COUNT (1)
   0x75, 0x08,                    //   REPORT_SIZE (8)
   0x81, 0x03,                    //   INPUT (Cnst,Var,Abs) ; Reserved byte
   0x95, 0x05,                    //   REPORT_COUNT (5)
   0x75, 0x01,                    //   REPORT_SIZE (1)
   0x05, 0x08,                    //   USAGE_PAGE (LEDs)
   0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
   0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
   0x91, 0x02,                    //   OUTPUT (Data,Var,Abs) ; LED report
   0x95, 0x01,                    //   REPORT_COUNT (1)
   0x75, 0x03,                    //   REPORT_SIZE (3)
   0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs) ; LED report padding
   0x95, 0x06,                    //   REPORT_COUNT (6)
   0x75, 0x08,                    //   REPORT_SIZE (8)
   0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
   0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
   0x05, 0x07,                    //   USAGE_PAGE (Keyboard)(Key Codes)
   0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))(0)
   0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)(101)
   0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
   0xc0                           // END_COLLECTION
};



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

#define KEY_SPACE   44

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

#define KEY_ARROW_LEFT 0x50


class DigiKeyboardDevice : public Print {
 public:
  DigiKeyboardDevice () {
    cli();
    usbDeviceDisconnect();
    _delay_ms(250);
    usbDeviceConnect();


    usbInit();
      
    sei();

    // TODO: Remove the next two lines once we fix
    //       missing first keystroke bug properly.
    memset(&reportBuffer, 0, sizeof(reportBuffer));      
    usbSetInterrupt(reinterpret_cast<unsigned char *>(&reportBuffer), sizeof(reportBuffer));
  }
    
  void update() {
    usbPoll();
  }
	
	// delay while updating until we are finished delaying
	void delay(long milli) {
		unsigned long last = millis();
	  while (milli > 0) {
	    unsigned long now = millis();
	    milli -= now - last;
	    last = now;
	    update();
	  }
	}
  
  //sendKeyStroke: sends a key press AND release
  void sendKeyStroke(byte keyStroke) {
    sendKeyStroke(keyStroke, 0);
  }

  //sendKeyStroke: sends a key press AND release with modifiers
  void sendKeyStroke(byte keyStroke, byte modifiers) {
	sendKeyPress(keyStroke, modifiers);
    // This stops endlessly repeating keystrokes:
	sendKeyPress(0,0);
  }

  //sendKeyPress: sends a key press only - no release
  //to release the key, send again with keyPress=0
  void sendKeyPress(byte keyPress) {
	sendKeyPress(keyPress, 0);
  }

  //sendKeyPress: sends a key press only, with modifiers - no release
  //to release the key, send again with keyPress=0
  void sendKeyPress(byte keyPress, byte modifiers) {
   	while (!usbInterruptIsReady()) {
      // Note: We wait until we can send keyPress
      //       so we know the previous keyPress was
      //       sent.
    	usbPoll();
    	_delay_ms(5);
    }
    
    memset(&reportBuffer, 0, sizeof(reportBuffer));
		
    reportBuffer.modifier = modifiers;
    reportBuffer.keycode[0] = keyPress;
    
    usbSetInterrupt(reinterpret_cast<unsigned char *>(&reportBuffer), sizeof(reportBuffer));
  }
  
  size_t write(uint8_t chr) {
    uint8_t data = pgm_read_byte_near(ascii_to_scan_code_table + (chr - 8));
    sendKeyStroke(data & 0b01111111, data >> 7 ? MOD_SHIFT_RIGHT : 0);
    return 1;
  }
  
  struct KeyboardReport{
   uint8_t modifier;
   uint8_t reserved;
   uint8_t keycode[6];
  };

  KeyboardReport    reportBuffer;
  uint8_t    led_states;
  using Print::write;
};

DigiKeyboardDevice DigiKeyboard = DigiKeyboardDevice();

#ifdef __cplusplus
extern "C"{
#endif
// see http://vusb.wikidot.com/driver-api
// constants are found in usbdrv.h
usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
   // see HID1_11.pdf sect 7.2 and http://vusb.wikidot.com/driver-api
   usbRequest_t *rq = (usbRequest_t *)((void *)data);

   if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
   return 0; // ignore request if it's not a class specific request

   // see HID1_11.pdf sect 7.2
   switch (rq->bRequest)
   {
      case USBRQ_HID_GET_IDLE:
      usbMsgPtr = &idleRate; // send data starting from this byte
      return 1; // send 1 byte
      case USBRQ_HID_SET_IDLE:
      idleRate = rq->wValue.bytes[1]; // read in idle rate
      return 0; // send nothing
      case USBRQ_HID_GET_PROTOCOL:
      usbMsgPtr = &protocol_version; // send data starting from this byte
      return 1; // send 1 byte
      case USBRQ_HID_SET_PROTOCOL:
      protocol_version = rq->wValue.bytes[1];
      return 0; // send nothing
      case USBRQ_HID_GET_REPORT:
      usbMsgPtr = reinterpret_cast<unsigned char *>(&DigiKeyboard.reportBuffer); // send the report data
      return sizeof(DigiKeyboard.reportBuffer);
      case USBRQ_HID_SET_REPORT:
      if (rq->wLength.word == 1) // check data is available
      {
         // 1 byte, we don't check report type (it can only be output or feature)
         // we never implemented "feature" reports so it can't be feature
         // so assume "output" reports
         // this means set LED status
         // since it's the only one in the descriptor
         return USB_NO_MSG; // send nothing but call usbFunctionWrite
      }
      else // no data or do not understand data, ignore
      {
         return 0; // send nothing
      }
      default: // do not understand data, ignore
      return 0; // send nothing
   }
}

usbMsgLen_t usbFunctionWrite(uint8_t * data, uchar len)
{
   DigiKeyboard.led_states = data[0];
   return 1; // 1 byte read
}

#ifdef __cplusplus
} // extern "C"
#endif


#endif // __DigiKeyboard_h__
