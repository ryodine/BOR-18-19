#ifndef BoR_1819_DEBUG_DISPLAY_GUARD_H
#define BoR_1819_DEBUG_DISPLAY_GUARD_H
#include "SevSeg.h"

class DebugDisplay {
  public:
    DebugDisplay() {};
    void begin();
    void tick();
    void setChar(char* ch);
    void setStopCode(char* ch);
  private:
    SevSeg sevseg;
    const unsigned long pattern_change_time = 1000; 
};

#endif
