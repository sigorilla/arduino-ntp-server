#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include "DateTime.h"
#include "RTC.h"
#include "TinyGPS++.h"

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

unsigned int NTP_PORT = 123;

const int NTP_PACKET_SIZE = 48;

byte packetBuffer[ NTP_PACKET_SIZE ];

EthernetUDP Udp;

RTC RTC;

int TZ = 3;

// TODO: delete after connect GPS module
DateTime dtt(__DATE__, __TIME__);
DateTime dt(dtt.year(), dtt.month(), dtt.day(), dtt.hour() - TZ, dtt.minute(), dtt.second());

unsigned long referenceTime = 0;
unsigned long originTime = 0;
unsigned long receiveTime = 0;
unsigned long transmitTime = 0;

// GPS
static const int RXPin = 8, TXPin = 9;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial GPSSerial(RXPin, TXPin);
// For stats that happen every 5 seconds
unsigned long last = 0UL;

void setup() {
  Serial.begin(115200);
  GPSSerial.begin(GPSBaud);

  while (!Serial || !GPSSerial);
  Ethernet.begin(mac, ip);
  Udp.begin(NTP_PORT);

  Wire.begin();
  RTC.begin();
  RTC.adjust(dt);

  Serial.println("NTP Server is running.");
}

void loop() {
  // work with GPS
  if (!GPSSerial.isListening()) {
    Serial.println("GPS is not listening.");
  }
  while (GPSSerial.available() > 0) {
    if (gps.encode(GPSSerial.read())) {
      readGPS();
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    delay(2000);
  }

  // NTP

  IPAddress remoteIP;
  int remotePort;
  int packetSize = Udp.parsePacket();

  if (packetSize) {
    Serial.println("Get UDP packet.");
    DateTime now = RTC.now();
    receiveTime = now.unixtime();
    // time of last set or update
    referenceTime = receiveTime;

    remoteIP = Udp.remoteIP();
    remotePort = Udp.remotePort();
    Serial.print("IP: ");
    Serial.println(remoteIP);
    Serial.print("port: ");
    Serial.println(remotePort);
    // We've received a packet, read the data from it
    // read the packet into the buffer
    Udp.read(packetBuffer, NTP_PACKET_SIZE);

    // the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    originTime = highWord << 16 | lowWord;

    sendNTPpacket(remoteIP, remotePort);
  }
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress remoteIP, int remotePort) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  // LI, Version: 4, Mode: 4 (server)
  packetBuffer[0] = 0b11100100;
  // Stratum, or type of clock
  packetBuffer[1] = 0b00000001;
  // Polling Interval
  packetBuffer[2] = 6;
  // Peer Clock Precision
  packetBuffer[3] = 0xEC;
  // 8 bytes of zero for Root Delay & Root Dispersion
  // G
  packetBuffer[12] = 71;
  // P
  packetBuffer[13] = 80;
  // S
  packetBuffer[14] = 83;
  packetBuffer[15] = 0;
  // Reference Time
  packetBuffer[16] = (referenceTime & 0xFF000000) >> 24;
  packetBuffer[17] = (referenceTime & 0x00FF0000) >> 16;
  packetBuffer[18] = (referenceTime & 0x0000FF00) >> 8;
  packetBuffer[19] = (referenceTime & 0x000000FF);
  // 20-23 fractions

  // Origin Time
  packetBuffer[24] = (originTime & 0xFF000000) >> 24;
  packetBuffer[25] = (originTime & 0x00FF0000) >> 16;
  packetBuffer[26] = (originTime & 0x0000FF00) >> 8;
  packetBuffer[27] = (originTime & 0x000000FF);
  // 28-31 fractions

  // Receive Time
  packetBuffer[32] = (receiveTime & 0xFF000000) >> 24;
  packetBuffer[33] = (receiveTime & 0x00FF0000) >> 16;
  packetBuffer[34] = (receiveTime & 0x0000FF00) >> 8;
  packetBuffer[35] = (receiveTime & 0x000000FF);
  // 36-39 fractions

  // Transmit Time
  DateTime now = RTC.now();
  transmitTime = now.unixtime();
  packetBuffer[40] = (transmitTime & 0xFF000000) >> 24;
  packetBuffer[41] = (transmitTime & 0x00FF0000) >> 16;
  packetBuffer[42] = (transmitTime & 0x0000FF00) >> 8;
  packetBuffer[43] = (transmitTime & 0x000000FF);
  // 44-47 fractions

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(remoteIP, remotePort);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void readGPS() {
  if (gps.location.isUpdated()) {
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);
  } else if (gps.date.isUpdated()) {
    Serial.print(F("DATE       Fix Age="));
    Serial.print(gps.date.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.date.value());
    Serial.print(F(" Year="));
    Serial.print(gps.date.year());
    Serial.print(F(" Month="));
    Serial.print(gps.date.month());
    Serial.print(F(" Day="));
    Serial.println(gps.date.day());
  } else if (gps.time.isUpdated()) {
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  } else if (gps.speed.isUpdated()) {
    Serial.print(F("SPEED      Fix Age="));
    Serial.print(gps.speed.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.speed.value());
    Serial.print(F(" Knots="));
    Serial.print(gps.speed.knots());
    Serial.print(F(" MPH="));
    Serial.print(gps.speed.mph());
    Serial.print(F(" m/s="));
    Serial.print(gps.speed.mps());
    Serial.print(F(" km/h="));
    Serial.println(gps.speed.kmph());
  } else if (gps.course.isUpdated()) {
    Serial.print(F("COURSE     Fix Age="));
    Serial.print(gps.course.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.course.value());
    Serial.print(F(" Deg="));
    Serial.println(gps.course.deg());
  } else if (gps.altitude.isUpdated()) {
    Serial.print(F("ALTITUDE   Fix Age="));
    Serial.print(gps.altitude.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.altitude.value());
    Serial.print(F(" Meters="));
    Serial.print(gps.altitude.meters());
    Serial.print(F(" Miles="));
    Serial.print(gps.altitude.miles());
    Serial.print(F(" KM="));
    Serial.print(gps.altitude.kilometers());
    Serial.print(F(" Feet="));
    Serial.println(gps.altitude.feet());
  } else if (gps.satellites.isUpdated()) {
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());
  } else if (gps.hdop.isUpdated()) {
    Serial.print(F("HDOP       Fix Age="));
    Serial.print(gps.hdop.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.hdop.value());
  } else if (millis() - last > 5000) {
    Serial.println();
    if (gps.location.isValid()) {
      static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
      double distanceToLondon =
        TinyGPSPlus::distanceBetween(
          gps.location.lat(),
          gps.location.lng(),
          LONDON_LAT,
          LONDON_LON);
      double courseToLondon =
        TinyGPSPlus::courseTo(
          gps.location.lat(),
          gps.location.lng(),
          LONDON_LAT,
          LONDON_LON);

      Serial.print(F("LONDON     Distance="));
      Serial.print(distanceToLondon/1000, 6);
      Serial.print(F(" km Course-to="));
      Serial.print(courseToLondon, 6);
      Serial.print(F(" degrees ["));
      Serial.print(TinyGPSPlus::cardinal(courseToLondon));
      Serial.println(F("]"));
    }

    Serial.print(F("DIAGS      Chars="));
    Serial.print(gps.charsProcessed());
    Serial.print(F(" Sentences-with-Fix="));
    Serial.print(gps.sentencesWithFix());
    Serial.print(F(" Failed-checksum="));
    Serial.print(gps.failedChecksum());
    Serial.print(F(" Passed-checksum="));
    Serial.println(gps.passedChecksum());

    if (gps.charsProcessed() < 10) {
      Serial.println(F("WARNING: No GPS data.  Check wiring."));
    }

    last = millis();
    Serial.println();
  }
}
