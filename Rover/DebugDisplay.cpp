#include "DebugDisplay.h"

void DebugDisplay::begin() {
  /*byte numDigits = 4;   
  byte digitPins[] = {32, 35, 36, 38}; //Digits: 1,2,3,4 <--put one resistor (ex: 220 Ohms, or 330 Ohms, etc, on each digit pin)
  byte segmentPins[] = {33, 37, 42, 40, 39, 34, 43, 41}; //Segments: A,B,C,D,E,F,G,Period
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(100);*/
  pinMode(DEBUG_DISPLAY_ENABLE_PIN, INPUT);
}
void DebugDisplay::tick() {
  if (digitalRead(DEBUG_DISPLAY_ENABLE_PIN)) {
    Wire.beginTransmission(DEBUG_DISPLAY_I2C);
    Wire.write(chars, 16);
    byte error = Wire.endTransmission();
  }
}
void DebugDisplay::setChar(const char* ch) {
  int len = min(strlen(ch), 17);
  strncpy(chars, empty, 17);
  strncpy(chars, ch, len);
  chars[17] = '\0';
}

void DebugDisplay::write(const char* ch) {
  setChar(ch);
  tick();
}

void DebugDisplay::setStopCode(const char* ch) {
  while (1) {
    if ((millis() % (pattern_change_time*2)) < pattern_change_time) {
      setChar(ch);
    } else {
      setChar("FAIL");
    }
    tick();
  }
}
