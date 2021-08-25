#include "DigiKeyboard.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "usbdrv.h"
#include "scancode-ascii-table.h"

static uchar    idleRate;            // in 4 ms units 
static uint8_t protocol_version = 0; // see HID1_11.pdf sect 7.2.6

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

// This was copied from https://github.com/7enderhead/kbdwtchdg which is nearly
// identical to the boot keyboard specification in https://www.usb.org/document-library/device-class-definition-hid-111
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

DigiKeyboardDevice::DigiKeyboardDevice () {
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
    
void DigiKeyboardDevice::update() {
  usbPoll();
}
	
// delay while updating until we are finished delaying
void DigiKeyboardDevice::delay(long milli) {
  unsigned long last = millis();
  while (milli > 0) {
    unsigned long now = millis();
    milli -= now - last;
    last = now;
    update();
  }
}

//sendKeyStroke: sends a key press AND release
void DigiKeyboardDevice::sendKeyStroke(uint8_t keyStroke) {
  sendKeyStroke(keyStroke, 0);
}

//sendKeyStroke: sends a key press AND release with modifiers
void DigiKeyboardDevice::sendKeyStroke(uint8_t keyStroke, uint8_t modifiers) {
  sendKeyPress(keyStroke, modifiers);
  // This stops endlessly repeating keystrokes:
  sendKeyPress(0,0);
}

//sendKeyPress: sends a key press only - no release
//to release the key, send again with keyPress=0
void DigiKeyboardDevice::sendKeyPress(uint8_t keyPress) {
  sendKeyPress(keyPress, 0);
}

//sendKeyPress: sends a key press only, with modifiers - no release
//to release the key, send again with keyPress=0
void DigiKeyboardDevice::sendKeyPress(uint8_t keyPress, uint8_t modifiers) {
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

size_t DigiKeyboardDevice::write(uint8_t chr) {
  uint8_t data = pgm_read_byte_near(ascii_to_scan_code_table + (chr - 8));
  sendKeyStroke(data & 0b01111111, data >> 7 ? MOD_SHIFT_RIGHT : 0);
  return 1;
}

void DigiKeyboardDevice::LightPrint(const char* ptr, bool pgm_mem) {
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
    if (led_states & 0b10) {
      c = ToggleCase(c);
    }
    write(c);
  }
}

void DigiKeyboardDevice::RepeatKeyStroke(uint8_t stroke, size_t num) {
  for (size_t i = 0; i < num; i++) {
    sendKeyStroke(stroke);
  }
}

void DigiKeyboardDevice::RepeatKeyStroke(uint8_t stroke, uint8_t modifiers, size_t num) {
  for (size_t i = 0; i < num; i++) {
    sendKeyStroke(stroke, modifiers);
  }
}

DigiKeyboardDevice& DigiKeyboardDevice::GetInstance() {
  return instance;
}

DigiKeyboardDevice DigiKeyboardDevice::instance = DigiKeyboardDevice();

DigiKeyboardDevice* keypoard_ptr = &DigiKeyboardDevice::GetInstance();

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
        usbMsgPtr = reinterpret_cast<unsigned char *>(&(keypoard_ptr->reportBuffer)); // send the report data
        return sizeof(DigiKeyboardDevice::KeyboardReport);
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
  // Only data we expect to receive is the byte of LED states.
  keypoard_ptr->led_states = data[0];
  return 1; // 1 byte read
}

#ifdef __cplusplus
} // extern "C"
#endif
