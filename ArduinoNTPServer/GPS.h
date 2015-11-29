#ifndef GPS_H_
#define GPS_H_

#include "Arduino.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
#include "DateTime.h"

class GPSDateTime {
 public:
  GPSDateTime() {};

  void commit(void);
  DateTime now(void);

  void time(String time) {
    newTime_ = time.toFloat() * 100;
  }

  uint16_t hour() {
    return time_ / 1000000;
  }

  uint16_t minute() {
    return (time_ / 10000) % 100;
  }

  uint16_t second() {
    return (time_ / 100) % 100;
  }

  uint32_t centisecond() {
    return time_ % 100;
  }

  void day(String day) {
    newDay_ = day.toInt();
  }
  uint16_t day(void) { return day_; };

  void month(String month) {
    newMonth_ = month.toInt();
  }
  uint16_t month(void) { return month_; };

  void year(String year) {
    newYear_ = year.toInt();
  }
  uint16_t year(void) { return year_; };

  void ltzh(String ltzh) {
    newLtzh_ = ltzh.toInt();
  }
  uint16_t ltzh(void) { return ltzh_; };

  void ltzn(String ltzn) {
    newLtzn_ = ltzn.toInt();
  }
  uint16_t ltzn(void) { return ltzn_; };

 private:
  uint32_t newTime_;
  uint16_t newYear_, newMonth_, newDay_;
  uint16_t newLtzh_, newLtzn_;

  uint32_t time_;
  uint16_t year_, month_, day_;
  uint16_t ltzh_, ltzn_;
};

class GPS {
 public:
  GPS(int rx, int tx, bool debug)
    : debug_(debug),
      GPSSerial(SoftwareSerial(rx, tx)) {
    count_ = 0;
    tmp = "\0";
    validCode = true;
    isNotChecked = true;
    validString = false;
    isUpdated_ = false;
  };

  void begin(uint32_t baund) {
    GPSSerial.begin(baund);
    while(!GPSSerial);
    setup();
  }
  void setup();

  bool encode();
  bool isUpdated(void) {
    return isUpdated_;
  }

  DateTime now(void) {
    return datetime_.now();
  }

  SoftwareSerial GPSSerial;
  const String GPS_CODE_ZDA = "GLZDA";
 private:
  uint8_t *s2ck(uint8_t *input, int length);
  String s2ckv0(String input);
  void sendMessage(uint8_t *msg, uint8_t len);
  bool getAck(uint8_t *msg);

  String tmp;

  uint8_t count_;
  uint8_t parity_;

  bool isNotChecked;
  bool validCode;
  bool validString;
  bool isUpdated_;

  bool debug_;
  String msg; // TODO: should be removed, only for debug

  GPSDateTime datetime_;
};

#endif  // GPS_H_
