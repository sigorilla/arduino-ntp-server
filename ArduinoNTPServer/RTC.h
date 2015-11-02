#ifndef RTC_H_
#define RTC_H_

#include "DateTime.h"

// RTC based on the DS1307 chip connected via I2C and the Wire library
class RTC {
 public:
  uint8_t isrunning(void);
  static uint8_t begin(void);
  static void adjust(const DateTime& dt);
  static DateTime now();
};

#endif  // RTC_H_
