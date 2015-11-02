#ifndef DATETIME_H_
#define DATETIME_H_

class DateTime {
 public:
  DateTime(uint32_t t = 0);
  DateTime(uint16_t year, uint8_t month, uint8_t day,
    uint8_t hour = 0, uint8_t minute = 0, uint8_t second = 0);
  DateTime(const char *date, const char *time);
  uint16_t year() const { return 2000 + year_; }
  uint8_t month() const { return month_; }
  uint8_t day() const { return day_; }
  uint8_t hour() const { return hour_; }
  uint8_t minute() const { return minute_; }
  uint8_t second() const { return second_; }
  uint8_t dayOfWeek() const;

  // 32-bit times as seconds since 1/1/2000
  long secondstime() const;
  // 32-bit times as seconds since 1/1/1900
  uint32_t unixtime(void) const;

 protected:
  uint8_t year_, month_, day_, hour_, minute_, second_;
};

#endif  // DATETIME_H_
