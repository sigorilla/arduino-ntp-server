#ifndef DATETIME_H_
#define DATETIME_H_

#include "Arduino.h"

class DateTime {
 public:
  DateTime(uint32_t t = 0, uint32_t centisecond = 0);
  DateTime(uint16_t year, uint16_t month, uint16_t day,
    uint16_t hour = 0, uint16_t minute = 0, uint16_t second = 0,
    uint32_t centisecond = 0);
  DateTime(const char *date, const char *time, uint32_t centisecond = 0);

  void time(uint32_t t);
  void centisecond(uint32_t centisecond) {
    centisecond_ = centisecond;
  };

  uint16_t year() const { return 2000 + year_; }
  uint16_t month() const { return month_; }
  uint16_t day() const { return day_; }
  uint16_t hour() const { return hour_; }
  uint16_t minute() const { return minute_; }
  uint16_t second() const { return second_; }
  uint32_t centisecond() const { return centisecond_; }
  uint16_t dayOfWeek() const;

  // 32-bit times as seconds since 1/1/2000
  long secondstime() const;
  // 32-bit times as seconds since 1/1/1900
  uint32_t ntptime(void) const;
  // 32-bit times as seconds since 1/1/1970
  uint32_t unixtime(void) const;

  void print(void) {
    Serial.print(F("UNIX: "));
    Serial.print(unixtime());
    Serial.print(F("."));
    Serial.println(centisecond());

    Serial.print(F("NTP: "));
    Serial.print(ntptime());
    Serial.print(F("."));
    Serial.println(centisecond());

    Serial.print(F("DATE: "));
    Serial.print(day());
    Serial.print(F("."));
    Serial.print(month());
    Serial.print(F("."));
    Serial.println(year());

    Serial.print(F("TIME: "));
    Serial.print(hour());
    Serial.print(F(":"));
    Serial.print(minute());
    Serial.print(F(":"));
    Serial.print(second());
    Serial.print(F("."));
    Serial.println(centisecond());

    Serial.println();
  };

 protected:
  uint16_t year_, month_, day_, hour_, minute_, second_;
  uint32_t centisecond_;
};

#endif  // DATETIME_H_
