#include "GPS.h"

// $GPZDA,174304.36,24,11,2015,00,00*66
// $0    ,1        ,2 ,3 ,4   ,5 ,6 *7  <-- pos
//  https://www.u-blox.com/sites/default/files/products/documents/u-blox6-GPS-GLONASS-QZSS-V14_ReceiverDescrProtSpec_%28GPS.G6-SW-12013%29_Public.pdf
bool GPS::encode(char c) {

  if (c == '$') {
    tmp = "\0";
    msg = "$";
    count_ = 0;
    parity_ = 0;
    validCode = true;
    isNotChecked = true;
    isUpdated_ = false;
    return false;
  }

  if (!validCode) {
    return false;
  }
  msg += c;
  if (c == ',' || c == '*') {
    // determinator between values
    if (debug_) {
      Serial.print("TMP: ");
      Serial.print(count_);
      Serial.print(" <> ");
      Serial.print(tmp);
      Serial.println();
    }
    switch (count_) {
      case 0: // ID
        if (tmp.equals(GPS_CODE_GPZDA)) {
          validCode = true;
        } else {
          validCode = false;
        }
        break;
      case 1: // time
        datetime_.time(tmp);
        break;
      case 2: // day
        datetime_.day(tmp);
        break;
      case 3: // month
        datetime_.month(tmp);
        break;
      case 4: // year
        datetime_.year(tmp);
        break;
      case 5:
        datetime_.ltzh(tmp);
        break;
      case 6:
        datetime_.ltzn(tmp);
        break;
      default:
        break;
    }
    if (c == ',') {
      parity_ ^= (uint8_t) c;
    }
    if (c == '*') {
      isNotChecked = false;
    }
    tmp = "\0";
    count_++;
  } else if (c == '\r') {
    // carriage return, so check
    String checksum = tmp;
    String checkParity = String(parity_, HEX);
    checkParity.toUpperCase();

    validString = checkParity.equals(checksum);

    if (validString) {
      if (debug_) {
        Serial.println("Checksum is VALID!");
      }
      datetime_.commit();
      isUpdated_ = true;
      // commit datetime
    } else {
      if (debug_) {
        Serial.print("INVALID: ");
        Serial.println(checkParity);
      }
    }
  } else if (c == '\n') {
    // end of string
    tmp = "\0";
    count_ = 0;
    parity_ = 0;
    if (debug_) {
      Serial.print("MSG: ");
      Serial.println(msg);
    }
    return true;
  } else {
    // ordinary char
    tmp += c;
    if (isNotChecked) {
      // XOR of all characters from $ to *
      parity_ ^= (uint8_t) c;
    }
  }

  return false;
}

void GPSDateTime::commit() {
  time_ = newTime_;
  year_ = newYear_;
  month_ = newMonth_;
  day_ = newDay_;
  ltzh_ = newLtzh_;
  ltzn_ = newLtzn_;
}

DateTime GPSDateTime::now() {
  return DateTime(year(), month(), day(),
    hour(), minute(), second(), centisecond());
}
