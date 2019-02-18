#include "DebugDisplay.h"

void DebugDisplay::begin() {
  byte numDigits = 4;   
  byte digitPins[] = {32, 35, 36, 38}; //Digits: 1,2,3,4 <--put one resistor (ex: 220 Ohms, or 330 Ohms, etc, on each digit pin)
  byte segmentPins[] = {33, 37, 42, 40, 39, 34, 43, 41}; //Segments: A,B,C,D,E,F,G,Period
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default. Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros);
  sevseg.setBrightness(100);
}
void DebugDisplay::tick() {
  sevseg.refreshDisplay();
}
void DebugDisplay::setChar(char* ch) {
  sevseg.setChars(ch);
}
void DebugDisplay::setStopCode(char* ch) {
  while (1) {
    if ((millis() % (pattern_change_time*2)) < pattern_change_time) {
      sevseg.setChars(ch);
    } else {
      sevseg.setChars("FAIL");
    }
    tick();
  }
}
