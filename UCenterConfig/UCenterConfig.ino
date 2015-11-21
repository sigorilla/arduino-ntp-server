/**
 * Sketch for U Center via GPS module
 */

#include <SoftwareSerial.h>

const int gRx = 8;
const int gTx = 9;

SoftwareSerial gSerial(gRx, gTx);

void setup() {
  Serial.begin(9600);
  gSerial.begin(9600);
}

void loop() {
  if (gSerial.available()) {
    int inByte = gSerial.read();
    Serial.write(inByte);
  }

  if (Serial.available()) {
    int inByte = Serial.read();
    gSerial.write(inByte);
  }
}
