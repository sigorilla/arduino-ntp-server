#include <Ethernet2.h>
#include <EthernetUdp2.h>
#include <Wire.h>
#include <SPI.h>
#include "DateTime.h"
#include "RTC.h"

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

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    continue;
  }
  Ethernet.begin(mac, ip);
  Udp.begin(NTP_PORT);

  Wire.begin();
  RTC.begin();
  RTC.adjust(dt);
  
  Serial.println("NTP Server is running.");
}

void loop() {
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
