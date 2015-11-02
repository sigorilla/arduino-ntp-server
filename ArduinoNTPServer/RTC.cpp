#include <Wire.h>
#include <avr/pgmspace.h>
#include "RTC.h"

#define DS1307_ADDRESS 0x68

#if (ARDUINO >= 100)
  #include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
#else
  #include <WProgram.h>
#endif

//The new wire library needs to take an int when you are sending for the zero register
int i = 0;

////////////////////////
// RTC implementation //
////////////////////////

static uint8_t bcd2bin (uint8_t val) {
  return val - 6 * (val >> 4);
}

static uint8_t bin2bcd (uint8_t val) {
  return val + 6 * (val / 10);
}

uint8_t RTC::begin(void) {
  return 1;
}

#if (ARDUINO >= 100)

uint8_t RTC::isrunning(void) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(i);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 1);
  uint8_t ss = Wire.read();
  return !(ss>>7);
}

void RTC::adjust(const DateTime& dt) {
    Wire.beginTransmission(DS1307_ADDRESS);
    Wire.write(i);
    Wire.write(bin2bcd(dt.second()));
    Wire.write(bin2bcd(dt.minute()));
    Wire.write(bin2bcd(dt.hour()));
    Wire.write(bin2bcd(0));
    Wire.write(bin2bcd(dt.day()));
    Wire.write(bin2bcd(dt.month()));
    Wire.write(bin2bcd(dt.year() - 2000));
    Wire.write(i);
    Wire.endTransmission();
}

DateTime RTC::now() {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(i);
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire.read() & 0x7F);
  uint8_t mm = bcd2bin(Wire.read());
  uint8_t hh = bcd2bin(Wire.read());
  Wire.read();
  uint8_t d = bcd2bin(Wire.read());
  uint8_t m = bcd2bin(Wire.read());
  uint16_t y = bcd2bin(Wire.read()) + 2000;

  return DateTime (y, m, d, hh, mm, ss);
}

#else

uint8_t RTC::isrunning(void) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.send(i); 
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 1);
  uint8_t ss = Wire.receive();
  return !(ss>>7);
}

void RTC::adjust(const DateTime& dt) {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.send(i);
  Wire.send(bin2bcd(dt.second()));
  Wire.send(bin2bcd(dt.minute()));
  Wire.send(bin2bcd(dt.hour()));
  Wire.send(bin2bcd(0));
  Wire.send(bin2bcd(dt.day()));
  Wire.send(bin2bcd(dt.month()));
  Wire.send(bin2bcd(dt.year() - 2000));
  Wire.send(i);
  Wire.endTransmission();
}

DateTime RTC::now() {
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.send(i); 
  Wire.endTransmission();

  Wire.requestFrom(DS1307_ADDRESS, 7);
  uint8_t ss = bcd2bin(Wire.receive() & 0x7F);
  uint8_t mm = bcd2bin(Wire.receive());
  uint8_t hh = bcd2bin(Wire.receive());
  Wire.receive();
  uint8_t d = bcd2bin(Wire.receive());
  uint8_t m = bcd2bin(Wire.receive());
  uint16_t y = bcd2bin(Wire.receive()) + 2000;

  return DateTime(y, m, d, hh, mm, ss);
}

#endif
