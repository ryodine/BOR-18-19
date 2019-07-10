#ifndef BoR_1819_DEBUG_DISPLAY_GUARD_H
#define BoR_1819_DEBUG_DISPLAY_GUARD_H
//#include "SevSeg.h"
#include <Wire.h>
#include "Constants.h"

class DebugDisplay {
  public:
    DebugDisplay() {};
    void begin();
    void tick();
    void write(const char* ch);
    void setChar(const char* ch);
    void setStopCode(const char* ch);
  private:
    //SevSeg sevseg;
    const unsigned long pattern_change_time = 1000; 
    const char empty[17] = "                ";
    char chars[17] = "Rover Connected.";
};

#endif
