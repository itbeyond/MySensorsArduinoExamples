#include <Arduino.h>
#include <IRSender.h>

// Heavily based on Ken Shirriff's IRRemote library:
// https://github.com/shirriff/Arduino-IRremote
//
// For PWM on Arduino, see http://playground.arduino.cc/Main/TimerPWMCheatsheet

IRSender::IRSender(uint8_t pin)
{
  _pin = pin;
}

// Set the PWM frequency. The selected pin determines which timer to use
void IRSender::setFrequency(int frequency)
{
  uint8_t pwmval8 = F_CPU / 2000 / (frequency);
  uint16_t pwmval16 = F_CPU / 2000 / (frequency);

  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW); // When not sending PWM, we want it low

  switch (_pin)
  {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Arduino Mega
    case 9:
      // Fall-through to 10, timer2 controls both 9 and 10
    case 10:
      TCCR2A = _BV(WGM20);
      TCCR2B = _BV(WGM22) | _BV(CS20);
      OCR2A = pwmval8;
      OCR2B = pwmval8 / 3;
      break;
    case 11:
      // Fall-through to 12, timer1 controls both 11 and 12
    case 12:
      TCCR1A = _BV(WGM11);
      TCCR1B = _BV(WGM13) | _BV(CS10);
      ICR1 = pwmval16;
      OCR1A = pwmval16 / 3;
      OCR1B = pwmval16 / 3;
      break;
    case 44:
      // Fall-through to 46, timer 5 controls pins 44, 45 and 46 on Arduino Mega
    case 45:
    case 46:
      TCCR5A = _BV(WGM51) | _BV(WGM50);
      TCCR5B = _BV(WGM53) | _BV(CS50);
      ICR5 = pwmval16;
      OCR5A = pwmval16 / 3;
#else
// Arduino Duemilanove etc
    case 3:
      // Fall-through to 11, timer2 controls both 3 and 11
    case 11:
      TCCR2A = _BV(WGM20);
      TCCR2B = _BV(WGM22) | _BV(CS20);
      OCR2A = pwmval8;
      OCR2B = pwmval8 / 3;
      break;
    case 9:
      // Fall-through to 10, timer1 controls both 9 and 10
    case 10:
      TCCR1A = _BV(WGM11);
      TCCR1B = _BV(WGM13) | _BV(CS10);
      ICR1 = pwmval16;
      OCR1A = pwmval16 / 3;
      OCR1B = pwmval16 / 3;
      break;
#endif
  }
}

// Send a uint8_t (8 bits) over IR
void IRSender::sendIRbyte(uint8_t sendByte, int bitMarkLength, int zeroSpaceLength, int oneSpaceLength)
{
  for (int i=0; i<8 ; i++)
  {
    if (sendByte & 0x01)
    {
      mark(bitMarkLength);
      space(oneSpaceLength);
    }
    else
    {
      mark(bitMarkLength);
      space(zeroSpaceLength);
    }

    sendByte >>= 1;
  }
}

// The Carrier IR protocol has the bits in a reverse order (compared to the other heatpumps)
// See http://www.nrtm.org/index.php/2013/07/25/reverse-bits-in-a-uint8_t/
uint8_t IRSender::bitReverse(uint8_t x)
{
  //          01010101  |         10101010
  x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
  //          00110011  |         11001100
  x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
  //          00001111  |         11110000
  x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
  return x;
}

// Send an IR 'mark' symbol, i.e. transmitter ON
void IRSender::mark(int markLength)
{
  switch (_pin)
  {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Arduino Mega
    case 9:
      (TCCR2A |= _BV(COM2B1)); // Enable pin 3 PWM output
      break;
    case 11:
      (TCCR1A |= _BV(COM1A1)); // Enable pin 9 PWM output
      break;
    case 12:
      (TCCR1A |= _BV(COM1B1)); // Enable pin 10 PWM output
      break;
    case 10:
      (TCCR2A |= _BV(COM2A1)); // Enable pin 11 PWM output
      break;
    case 44:
      (TCCR5A |= _BV(COM5C1)); // Enable pin 44 PWM output on Arduino Mega
      break;
    case 45:
      (TCCR5A |= _BV(COM5B1)); // Enable pin 45 PWM output on Arduino Mega
      break;
    case 46:
      (TCCR5A |= _BV(COM5A1)); // Enable pin 46 PWM output on Arduino Mega
      break;
#else
// Arduino Duemilanove etc
    case 3:
      (TCCR2A |= _BV(COM2B1)); // Enable pin 3 PWM output
      break;
    case 9:
      (TCCR1A |= _BV(COM1A1)); // Enable pin 9 PWM output
      break;
    case 10:
      (TCCR1A |= _BV(COM1B1)); // Enable pin 10 PWM output
      break;
    case 11:
      (TCCR2A |= _BV(COM2A1)); // Enable pin 11 PWM output
      break;
#endif
    }

  delayMicroseconds(markLength);
}

// Send an IR 'space' symbol, i.e. transmitter OFF
void IRSender::space(int spaceLength)
{
  switch (_pin)
  {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// Arduino Mega
    case 9:
      (TCCR2A &= ~(_BV(COM2B1))); // Disable pin 3 PWM output
      break;
    case 11:
      (TCCR1A &= ~(_BV(COM1A1))); // Disable pin 9 PWM output
      break;
    case 12:
      (TCCR1A &= ~(_BV(COM1B1))); // Disable pin 10 PWM output
      break;
    case 10:
      (TCCR2A &= ~(_BV(COM2A1))); // Disable pin 11 PWM output
      break;
    case 44:
      (TCCR5A &= ~(_BV(COM5C1))); // Disable pin 44 PWM output on Arduino Mega
    case 45:
      (TCCR5A &= ~(_BV(COM5B1))); // Disable pin 45 PWM output on Arduino Mega
    case 46:
      (TCCR5A &= ~(_BV(COM5A1))); // Disable pin 46 PWM output on Arduino Mega
#else
// Arduino Duemilanove etc
    case 3:
      (TCCR2A &= ~(_BV(COM2B1))); // Disable pin 3 PWM output
      break;
    case 9:
      (TCCR1A &= ~(_BV(COM1A1))); // Disable pin 9 PWM output
      break;
    case 10:
      (TCCR1A &= ~(_BV(COM1B1))); // Disable pin 10 PWM output
      break;
    case 11:
      (TCCR2A &= ~(_BV(COM2A1))); // Disable pin 11 PWM output
      break;
#endif
}

  // Mitsubishi heatpump uses > 16383us spaces, and delayMicroseconds only works up to 2^14 - 1 us
  // Use the less accurate milliseconds delay for longer delays

  if (spaceLength < 16383) {
    delayMicroseconds(spaceLength);
  } else {
    delay(spaceLength/1000);
  }
}
